// SPDX-License-Identifier: GPL-2.0
/*
 * ARCH_INLINE_* inlines the lock wrappers and leaves the out-of-line
 * versions out of the image. Restore them next to the inlined fast path:
 * anything that resolves lock entry points by name through kallsyms,
 * such as KernelPatch/APatch and the KPMs built on top of it, otherwise
 * gets NULL and calls address 0.
 */

#include <linux/spinlock.h>
#include <linux/rwlock.h>
#include <linux/export.h>

#undef _raw_spin_trylock
#undef _raw_spin_trylock_bh
#undef _raw_spin_lock
#undef _raw_spin_lock_irqsave
#undef _raw_spin_lock_irq
#undef _raw_spin_lock_bh
#undef _raw_spin_unlock
#undef _raw_spin_unlock_irqrestore
#undef _raw_spin_unlock_irq
#undef _raw_spin_unlock_bh
#undef _raw_read_trylock
#undef _raw_read_lock
#undef _raw_read_lock_irqsave
#undef _raw_read_lock_irq
#undef _raw_read_lock_bh
#undef _raw_read_unlock
#undef _raw_read_unlock_irqrestore
#undef _raw_read_unlock_irq
#undef _raw_read_unlock_bh
#undef _raw_write_trylock
#undef _raw_write_lock
#undef _raw_write_lock_irqsave
#undef _raw_write_lock_irq
#undef _raw_write_lock_bh
#undef _raw_write_unlock
#undef _raw_write_unlock_irqrestore
#undef _raw_write_unlock_irq
#undef _raw_write_unlock_bh

#ifdef CONFIG_INLINE_SPIN_TRYLOCK
int __lockfunc _raw_spin_trylock(raw_spinlock_t *lock)
{
	return __raw_spin_trylock(lock);
}
EXPORT_SYMBOL(_raw_spin_trylock);
#endif

#ifdef CONFIG_INLINE_SPIN_TRYLOCK_BH
int __lockfunc _raw_spin_trylock_bh(raw_spinlock_t *lock)
{
	return __raw_spin_trylock_bh(lock);
}
EXPORT_SYMBOL(_raw_spin_trylock_bh);
#endif

#ifdef CONFIG_INLINE_SPIN_LOCK
void __lockfunc _raw_spin_lock(raw_spinlock_t *lock)
{
	__raw_spin_lock(lock);
}
EXPORT_SYMBOL(_raw_spin_lock);
#endif

#ifdef CONFIG_INLINE_SPIN_LOCK_IRQSAVE
unsigned long __lockfunc _raw_spin_lock_irqsave(raw_spinlock_t *lock)
{
	return __raw_spin_lock_irqsave(lock);
}
EXPORT_SYMBOL(_raw_spin_lock_irqsave);
#endif

#ifdef CONFIG_INLINE_SPIN_LOCK_IRQ
void __lockfunc _raw_spin_lock_irq(raw_spinlock_t *lock)
{
	__raw_spin_lock_irq(lock);
}
EXPORT_SYMBOL(_raw_spin_lock_irq);
#endif

#ifdef CONFIG_INLINE_SPIN_LOCK_BH
void __lockfunc _raw_spin_lock_bh(raw_spinlock_t *lock)
{
	__raw_spin_lock_bh(lock);
}
EXPORT_SYMBOL(_raw_spin_lock_bh);
#endif

#ifndef CONFIG_UNINLINE_SPIN_UNLOCK
void __lockfunc _raw_spin_unlock(raw_spinlock_t *lock)
{
	__raw_spin_unlock(lock);
}
EXPORT_SYMBOL(_raw_spin_unlock);
#endif

#ifdef CONFIG_INLINE_SPIN_UNLOCK_IRQRESTORE
void __lockfunc _raw_spin_unlock_irqrestore(raw_spinlock_t *lock,
					    unsigned long flags)
{
	__raw_spin_unlock_irqrestore(lock, flags);
}
EXPORT_SYMBOL(_raw_spin_unlock_irqrestore);
#endif

#ifdef CONFIG_INLINE_SPIN_UNLOCK_IRQ
void __lockfunc _raw_spin_unlock_irq(raw_spinlock_t *lock)
{
	__raw_spin_unlock_irq(lock);
}
EXPORT_SYMBOL(_raw_spin_unlock_irq);
#endif

#ifdef CONFIG_INLINE_SPIN_UNLOCK_BH
void __lockfunc _raw_spin_unlock_bh(raw_spinlock_t *lock)
{
	__raw_spin_unlock_bh(lock);
}
EXPORT_SYMBOL(_raw_spin_unlock_bh);
#endif

#ifdef CONFIG_INLINE_READ_TRYLOCK
int __lockfunc _raw_read_trylock(rwlock_t *lock)
{
	return __raw_read_trylock(lock);
}
EXPORT_SYMBOL(_raw_read_trylock);
#endif

#ifdef CONFIG_INLINE_READ_LOCK
void __lockfunc _raw_read_lock(rwlock_t *lock)
{
	__raw_read_lock(lock);
}
EXPORT_SYMBOL(_raw_read_lock);
#endif

#ifdef CONFIG_INLINE_READ_LOCK_IRQSAVE
unsigned long __lockfunc _raw_read_lock_irqsave(rwlock_t *lock)
{
	return __raw_read_lock_irqsave(lock);
}
EXPORT_SYMBOL(_raw_read_lock_irqsave);
#endif

#ifdef CONFIG_INLINE_READ_LOCK_IRQ
void __lockfunc _raw_read_lock_irq(rwlock_t *lock)
{
	__raw_read_lock_irq(lock);
}
EXPORT_SYMBOL(_raw_read_lock_irq);
#endif

#ifdef CONFIG_INLINE_READ_LOCK_BH
void __lockfunc _raw_read_lock_bh(rwlock_t *lock)
{
	__raw_read_lock_bh(lock);
}
EXPORT_SYMBOL(_raw_read_lock_bh);
#endif

#ifdef CONFIG_INLINE_READ_UNLOCK
void __lockfunc _raw_read_unlock(rwlock_t *lock)
{
	__raw_read_unlock(lock);
}
EXPORT_SYMBOL(_raw_read_unlock);
#endif

#ifdef CONFIG_INLINE_READ_UNLOCK_IRQRESTORE
void __lockfunc _raw_read_unlock_irqrestore(rwlock_t *lock,
					    unsigned long flags)
{
	__raw_read_unlock_irqrestore(lock, flags);
}
EXPORT_SYMBOL(_raw_read_unlock_irqrestore);
#endif

#ifdef CONFIG_INLINE_READ_UNLOCK_IRQ
void __lockfunc _raw_read_unlock_irq(rwlock_t *lock)
{
	__raw_read_unlock_irq(lock);
}
EXPORT_SYMBOL(_raw_read_unlock_irq);
#endif

#ifdef CONFIG_INLINE_READ_UNLOCK_BH
void __lockfunc _raw_read_unlock_bh(rwlock_t *lock)
{
	__raw_read_unlock_bh(lock);
}
EXPORT_SYMBOL(_raw_read_unlock_bh);
#endif

#ifdef CONFIG_INLINE_WRITE_TRYLOCK
int __lockfunc _raw_write_trylock(rwlock_t *lock)
{
	return __raw_write_trylock(lock);
}
EXPORT_SYMBOL(_raw_write_trylock);
#endif

#ifdef CONFIG_INLINE_WRITE_LOCK
void __lockfunc _raw_write_lock(rwlock_t *lock)
{
	__raw_write_lock(lock);
}
EXPORT_SYMBOL(_raw_write_lock);
#endif

#ifdef CONFIG_INLINE_WRITE_LOCK_IRQSAVE
unsigned long __lockfunc _raw_write_lock_irqsave(rwlock_t *lock)
{
	return __raw_write_lock_irqsave(lock);
}
EXPORT_SYMBOL(_raw_write_lock_irqsave);
#endif

#ifdef CONFIG_INLINE_WRITE_LOCK_IRQ
void __lockfunc _raw_write_lock_irq(rwlock_t *lock)
{
	__raw_write_lock_irq(lock);
}
EXPORT_SYMBOL(_raw_write_lock_irq);
#endif

#ifdef CONFIG_INLINE_WRITE_LOCK_BH
void __lockfunc _raw_write_lock_bh(rwlock_t *lock)
{
	__raw_write_lock_bh(lock);
}
EXPORT_SYMBOL(_raw_write_lock_bh);
#endif

#ifdef CONFIG_INLINE_WRITE_UNLOCK
void __lockfunc _raw_write_unlock(rwlock_t *lock)
{
	__raw_write_unlock(lock);
}
EXPORT_SYMBOL(_raw_write_unlock);
#endif

#ifdef CONFIG_INLINE_WRITE_UNLOCK_IRQRESTORE
void __lockfunc _raw_write_unlock_irqrestore(rwlock_t *lock,
					     unsigned long flags)
{
	__raw_write_unlock_irqrestore(lock, flags);
}
EXPORT_SYMBOL(_raw_write_unlock_irqrestore);
#endif

#ifdef CONFIG_INLINE_WRITE_UNLOCK_IRQ
void __lockfunc _raw_write_unlock_irq(rwlock_t *lock)
{
	__raw_write_unlock_irq(lock);
}
EXPORT_SYMBOL(_raw_write_unlock_irq);
#endif

#ifdef CONFIG_INLINE_WRITE_UNLOCK_BH
void __lockfunc _raw_write_unlock_bh(rwlock_t *lock)
{
	__raw_write_unlock_bh(lock);
}
EXPORT_SYMBOL(_raw_write_unlock_bh);
#endif
