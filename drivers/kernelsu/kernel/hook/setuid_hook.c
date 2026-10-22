#include <linux/compiler.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
#include <linux/sched/signal.h>
#endif
#include <linux/slab.h>
#include <linux/task_work.h>
#include <linux/thread_info.h>
#include <linux/seccomp.h>
#include <linux/printk.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/uidgid.h>

#include "policy/app_profile.h"
#include "policy/allowlist.h"
#include "policy/app_profile.h"
#include "hook/setuid_hook.h"
#include "klog.h" // IWYU pragma: keep
#include "manager/manager_identity.h"
#include "infra/seccomp_cache.h"
#include "supercall/supercall.h"
#include "hook/hook_manager.h"
#include "feature/kernel_umount.h"
#include "compat/kernel_compat.h"

#ifdef CONFIG_KSU_SUSFS
#include <linux/susfs_def.h>
#include <linux/workqueue.h>
#include "selinux/selinux.h"
#endif // #ifdef CONFIG_KSU_SUSFS

#ifdef CONFIG_KSU_SUSFS
extern u32 susfs_zygote_sid;
extern u32 susfs_zygote_next_sid;
extern struct work_struct susfs_extra_works;

// - Defer extra susfs works (e.g. re-flagging sus_path_loop) to a workqueue so we
//   do not block the zygote child here and reduce the risk of time side channels.
static inline void ksu_handle_extra_susfs_work(void)
{
    if (!work_pending(&susfs_extra_works))
        schedule_work(&susfs_extra_works);
}

// Same conditions ksu_handle_umount() uses to decide whether a zygote child gets umounted
static inline bool ksu_susfs_should_umount_uid(uid_t new_uid)
{
    if (is_isolated_process(new_uid))
        return true;

    return (is_appuid(new_uid) || new_uid == WEBVIEW_ZYGOTE_UID) &&
           ksu_uid_should_umount(new_uid);
}

// - Processes spawned by zygote_next already live in the init mount namespace,
//   so only flag them for susfs and do not umount anything here.
static int ksu_handle_susfs_zygote_next_setresuid(uid_t new_uid)
{
    if (unlikely(is_uid_manager(new_uid)))
        return 0;

    if (ksu_susfs_should_umount_uid(new_uid)) {
        susfs_set_current_proc_no_su();
        susfs_set_current_proc_umounted();
        susfs_set_current_proc_umounted_for_zygote_next();
        ksu_handle_extra_susfs_work();
        return 0;
    }

    if (!ksu_is_allow_uid_for_current(new_uid))
        susfs_set_current_proc_no_su();

    return 0;
}

// - Flag zygote spawned processes for susfs. The setresuid hook may be reached more than
//   once for the same process (syscall + LSM hook), so bail out if it is already flagged.
static void ksu_handle_susfs_zygote_setresuid(uid_t new_uid)
{
    if (!susfs_is_sid_equal(current_cred(), susfs_zygote_sid))
        return;

    if (susfs_is_current_proc_umounted())
        return;

    if (ksu_susfs_should_umount_uid(new_uid)) {
        susfs_set_current_proc_no_su();
        susfs_set_current_proc_umounted();
        ksu_handle_extra_susfs_work();
        return;
    }

    if (!ksu_is_allow_uid_for_current(new_uid))
        susfs_set_current_proc_no_su();
}
#endif // #ifdef CONFIG_KSU_SUSFS


int ksu_handle_setresuid(uid_t old_uid, uid_t new_uid)
{
    // we rely on the fact that zygote always call setresuid(3) with same uids

#ifdef CONFIG_KSU_SUSFS
    // We only care about processes spawned by zygote_next as root
    if (unlikely(current_uid().val == 0 &&
                 susfs_is_sid_equal(current_cred(), susfs_zygote_next_sid)))
        return ksu_handle_susfs_zygote_next_setresuid(new_uid);
#endif // #ifdef CONFIG_KSU_SUSFS

    pr_info("handle_setresuid from %d to %d\n", old_uid, new_uid);

    if (unlikely(is_uid_manager(new_uid))) {

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
        if (current->seccomp.mode == SECCOMP_MODE_FILTER && current->seccomp.filter) {
            ksu_seccomp_allow_cache(current->seccomp.filter, __NR_reboot);
        }
#else
		disable_seccomp();
#endif

#ifdef KSU_KPROBES_HOOK
        ksu_set_task_tracepoint_flag(current);
#endif

        pr_info("install fd for manager: %d\n", new_uid);
        ksu_install_fd();
        return 0;
    }

    if (ksu_is_allow_uid_for_current(new_uid)) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
        if (current->seccomp.mode == SECCOMP_MODE_FILTER && current->seccomp.filter) {
            ksu_seccomp_allow_cache(current->seccomp.filter, __NR_reboot);
        }
#else
		disable_seccomp();
#endif

#ifdef KSU_KPROBES_HOOK
		ksu_set_task_tracepoint_flag(current);
#endif
	} else {
#ifdef KSU_KPROBES_HOOK
		ksu_clear_task_tracepoint_flag_if_needed(current);
#endif
    }

#ifdef CONFIG_KSU_SUSFS
    if (current_uid().val == 0)
        ksu_handle_susfs_zygote_setresuid(new_uid);
#endif // #ifdef CONFIG_KSU_SUSFS

    // Handle kernel umount
    ksu_handle_umount(old_uid, new_uid);

    return 0;
}

void __init ksu_setuid_hook_init(void)
{
	ksu_kernel_umount_init();
}

void __exit ksu_setuid_hook_exit(void)
{
	pr_info("ksu_core_exit\n");
	ksu_kernel_umount_exit();
}