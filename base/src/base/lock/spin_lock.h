/*
 * \brief  Spin lock implementation
 * \author Norman Feske
 * \author Stefan Kalkowski
 * \date   2009-03-25
 */

/*
 * Copyright (C) 2009-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__BASE__LOCK__SPIN_H_
#define _INCLUDE__BASE__LOCK__SPIN_H_

/* Genode includes */
#include <cpu/atomic.h>

/* local includes */
#include <lock_helper.h>

/*
 * Spinlock functions used for protecting the critical sections within the
 * 'lock' and 'unlock' functions. Contention in these short-running code
 * portions is rare but is must be considered.
 */

enum State { SPINLOCK_LOCKED, SPINLOCK_UNLOCKED };

static inline void spinlock_lock(volatile int *lock_variable)
{
	while (!Genode::cmpxchg(lock_variable, SPINLOCK_UNLOCKED, SPINLOCK_LOCKED)) {
		/*
		 * Yield our remaining time slice to help the spinlock holder to pass
		 * the critical section.
		 */
		thread_yield();
	}
}


static inline void spinlock_unlock(volatile int *lock_variable)
{
	*lock_variable = SPINLOCK_UNLOCKED;
}

#endif /* _INCLUDE__BASE__LOCK__SPIN_H_ */
