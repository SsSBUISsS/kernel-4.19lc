/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 MediaTek Inc.
 */


#include <linux/linkage.h>
#include <linux/init.h>

/*	__CPUINIT*/

ENTRY(mt_secondary_startup)
	mrc	p15, 0, r0, c0, c0, 5           @ MPIDR
	ubfx	r1, r0, #8, #4                  @ Cluster ID
	cmp	r1, #0                          @ Cluster 1
	beq	end
	ubfx	r1, r0, #0, #2                  @ CPU ID
	cmp	r1, #0                          @ CPU 0 (aka CPU4)
	bne	end

#ifdef CONFIG_ARM_ERRATA_826319
	mrc	p15, 1, r5, c15, c0, 0          @ Read L2ACTLR into Rt
	bic	r5, r5, #0x4000                 @ Set Bit[14]=0
	orr	r5, r5, #0x8                    @ Set Bit[3]=1
	mcr	p15, 1, r5, c15, c0, 0          @ Write Rt to L2ACTLR
	isb
#endif
end:
	b	secondary_startup
END(mt_secondary_startup)

