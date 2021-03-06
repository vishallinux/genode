/**
 * \brief  Shared object startup code
 * \author Sebastian Sumpf
 * \date   2009-08-14
 */

/*
 * Copyright (C) 2009-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#define BEG { (ld_hook) ~1U }
#define END { (ld_hook) 0   }
#define SECTION(x) __attribute__((used,section( x )))

typedef void (*ld_hook)(void);
static ld_hook _lctors_start[1] SECTION("_mark_ctors_start") = BEG;
static ld_hook _lctors_end[1]   SECTION("_mark_ctors_end")   = END;

/* called by dynamic linker on library startup (ld-genode.so) */
extern "C" {
	void _init(void) __attribute__((used,section(".init")));

	void _init(void)
	{
		/* call static constructors */
		for(ld_hook *func = _lctors_end; func > _lctors_start + 1; (*--func)());
	}
}
