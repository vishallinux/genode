/*
 * \brief  LAN 9118 NIC controller definitions for the RealView platform
 * \author Norman Feske
 * \date   2010-03-23
 */

/*
 * Copyright (C) 2010-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__PLATFORM__PBXA9__LAN9118_DEFS_H_
#define _INCLUDE__PLATFORM__PBXA9__LAN9118_DEFS_H_

enum {

	/**
	 * Base address of MMIO resource
	 */
	LAN9118_PHYS = 0x4e000000,

	/**
	 * Size of MMIO resource
	 *
	 * On the RealView platform, the device spans actually a much larger
	 * resource. However, only the first page is used.
	 */
	LAN9118_SIZE = 0x1000,

	/**
	 * Interrupt line
	 */
	LAN9118_IRQ = 28 + 32,
};

#endif /* _INCLUDE__PLATFORM__PBXA9__LAN9118_DEFS_H_ */
