#ifdef CONFIG_KSU_SUSFS
#include <linux/namei.h>
#include <linux/susfs.h>
#include "objsec.h"
#endif // #ifdef CONFIG_KSU_SUSFS

#define KSU_DRIVER_PERMISSION_SU_SESSION (1UL << 0)

struct ksu_driver_context {
	unsigned long permissions;
};

static int anon_ksu_release(struct inode *inode, struct file *filp)
{
	kfree(filp->private_data);
	pr_info("ksu fd released\n");
	return 0;
}

static long anon_ksu_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	return ksu_supercall_handle_ioctl(filp, cmd, (void __user *)arg);
}

// File operations structure
static const struct file_operations anon_ksu_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = anon_ksu_ioctl,
	.compat_ioctl = anon_ksu_ioctl,
	.release = anon_ksu_release,
};

// Install KSU fd to current process
static int ksu_install_fd_with_permissions(unsigned int fd_flags, unsigned long permissions)
{
	struct ksu_driver_context *context;
	struct file *filp;
	const char *name;
	int fd;

	context = kzalloc(sizeof(*context), GFP_KERNEL);
	if (!context)
		return -ENOMEM;

	context->permissions = permissions;
	name = permissions & KSU_DRIVER_PERMISSION_SU_SESSION ? "[ksu_driver_su]" : "[ksu_driver]";

	fd = get_unused_fd_flags(fd_flags);
	if (fd < 0) {
		pr_err("ksu_install_fd: failed to get unused fd\n");
		kfree(context);
		return fd;
	}

	filp = anon_inode_getfile(name, &anon_ksu_fops, context, O_RDWR);
	if (IS_ERR(filp)) {
		pr_err("ksu_install_fd: failed to create anon inode file\n");
		put_unused_fd(fd);
		kfree(context);
		return PTR_ERR(filp);
	}

	fd_install(fd, filp);
	pr_info("ksu fd installed: %d for pid %d\n", fd, current->pid);
	return fd;
}

int ksu_install_fd(void)
{
	return ksu_install_fd_with_permissions(O_CLOEXEC, 0);
}

int ksu_install_su_fd(void)
{
	// This descriptor must be installed after the exec into ksud.
	return ksu_install_fd_with_permissions(0, KSU_DRIVER_PERMISSION_SU_SESSION);
}

bool ksu_is_su_session_fd(const struct file *filp)
{
	const struct ksu_driver_context *context = filp->private_data;

	return context && (context->permissions & KSU_DRIVER_PERMISSION_SU_SESSION);
}

// downstream: make sure to pass arg as reference, this can allow us to extend things.
int ksu_handle_sys_reboot(int magic1, int magic2, unsigned int cmd, void __user **arg)
{
	if (magic1 != KSU_INSTALL_MAGIC1)
		return 0;
#ifdef CONFIG_KSU_SUSFS
    // If magic2 is susfs and current process is root
    if (magic2 == SUSFS_MAGIC && current_uid().val == 0) {
#ifdef CONFIG_KSU_SUSFS_SUS_PATH
        if (cmd == CMD_SUSFS_ADD_SUS_PATH) {
            susfs_add_sus_path(arg);
            return 0;
        }
        if (cmd == CMD_SUSFS_ADD_SUS_PATH_LOOP) {
            susfs_add_sus_path_loop(arg);
            return 0;
        }
#endif //#ifdef CONFIG_KSU_SUSFS_SUS_PATH
#ifdef CONFIG_KSU_SUSFS_SUS_MOUNT
        if (cmd == CMD_SUSFS_HIDE_SUS_MNTS_FOR_NON_SU_PROCS) {
            susfs_set_hide_sus_mnts_for_non_su_procs(arg);
            return 0;
        }
#endif //#ifdef CONFIG_KSU_SUSFS_SUS_MOUNT
#ifdef CONFIG_KSU_SUSFS_SUS_KSTAT
        if (cmd == CMD_SUSFS_ADD_SUS_KSTAT) {
            susfs_add_sus_kstat(arg);
            return 0;
        }
        if (cmd == CMD_SUSFS_UPDATE_SUS_KSTAT) {
            susfs_update_sus_kstat(arg);
            return 0;
        }
        if (cmd == CMD_SUSFS_ADD_SUS_KSTAT_STATICALLY) {
            susfs_add_sus_kstat(arg);
            return 0;
        }
#endif //#ifdef CONFIG_KSU_SUSFS_SUS_KSTAT
#ifdef CONFIG_KSU_SUSFS_TRY_UMOUNT
        if (cmd == CMD_SUSFS_ADD_TRY_UMOUNT) {
            susfs_add_try_umount(arg);
            return 0;
        }
#endif //#ifdef CONFIG_KSU_SUSFS_TRY_UMOUNT
#ifdef CONFIG_KSU_SUSFS_SPOOF_UNAME
        if (cmd == CMD_SUSFS_SET_UNAME) {
            susfs_set_uname(arg);
            return 0;
        }
#endif //#ifdef CONFIG_KSU_SUSFS_SPOOF_UNAME
#ifdef CONFIG_KSU_SUSFS_ENABLE_LOG
        if (cmd == CMD_SUSFS_ENABLE_LOG) {
            susfs_enable_log(arg);
            return 0;
        }
#endif //#ifdef CONFIG_KSU_SUSFS_ENABLE_LOG
#ifdef CONFIG_KSU_SUSFS_SPOOF_CMDLINE_OR_BOOTCONFIG
        if (cmd == CMD_SUSFS_SET_CMDLINE_OR_BOOTCONFIG) {
            susfs_set_cmdline_or_bootconfig(arg);
            return 0;
        }
#endif //#ifdef CONFIG_KSU_SUSFS_SPOOF_CMDLINE_OR_BOOTCONFIG
#ifdef CONFIG_KSU_SUSFS_OPEN_REDIRECT
        if (cmd == CMD_SUSFS_ADD_OPEN_REDIRECT) {
            susfs_add_open_redirect(arg);
            return 0;
        }
#endif //#ifdef CONFIG_KSU_SUSFS_OPEN_REDIRECT
#ifdef CONFIG_KSU_SUSFS_SUS_MAP
        if (cmd == CMD_SUSFS_ADD_SUS_MAP) {
            susfs_add_sus_map(arg);
            return 0;
        }
#endif // #ifdef CONFIG_KSU_SUSFS_SUS_MAP
        if (cmd == CMD_SUSFS_ENABLE_AVC_LOG_SPOOFING) {
            susfs_set_avc_log_spoofing(arg);
            return 0;
        }
        if (cmd == CMD_SUSFS_SHOW_ENABLED_FEATURES) {
            susfs_get_enabled_features(arg);
            return 0;
        }
        if (cmd == CMD_SUSFS_SHOW_VARIANT) {
            susfs_show_variant(arg);
            return 0;
        }
        if (cmd == CMD_SUSFS_SHOW_VERSION) {
            susfs_show_version(arg);
            return 0;
        }
        return 0;
    }
#endif // #ifdef CONFIG_KSU_SUSFS
#ifdef CONFIG_KSU_DEBUG
	pr_info("sys_reboot: intercepted call! magic: 0x%x id: %d\n", magic1,
		magic2);
#endif

	// when ternary on fmt?
	// cold syscall, we can splurge xD
	if (magic2 == KSU_INSTALL_MAGIC2)
		pr_info("sys_reboot: magic: 0x%x id: 0x%x pid: %d comm: %s \n", magic1, magic2, current->pid, current->comm);
	else
		pr_info("sys_reboot: magic: 0x%x id: %d pid: %d pid: %s \n", magic1, magic2, current->pid, current->comm);

	// arg4 = (unsigned long)PT_REGS_SYSCALL_PARM4(real_regs);
	// downstream: dereference arg as arg4 so we can be inline to upstream
	void __user *arg4 = (void __user *)*arg;

	if (magic2 == KSU_INSTALL_MAGIC2) {
		int fd = ksu_install_fd();

		pr_info("[%d] install ksu fd: %d\n", current->pid, fd);
		if (copy_to_user((void __user *)arg4, &fd, sizeof(fd))) {
			pr_err("install ksu fd reply err\n");
			ksu_close_fd(fd);
		}

		return 0;
	}

	toolkit_handle_sys_reboot(magic1, magic2, cmd, arg);
	return 0;
}

void __init ksu_supercalls_init(void)
{
	ksu_supercall_dump_commands();
}

void __exit ksu_supercalls_exit(void) { }
