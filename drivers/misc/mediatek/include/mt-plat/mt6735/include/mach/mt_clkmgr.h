// SPDX-License-Identifier: GPL-2.0
/*
* Copyright (C) 2019  MediaTek Inc.
*
*/

#ifndef _MT_CLKMGR_H
#define _MT_CLKMGR_H

#ifdef CONFIG_MTK_CLKMGR
#ifdef CONFIG_MACH_MT6735M
#include "mach/mt_clkmgr2.h"
#elif defined(CONFIG_MACH_MT6753)
#include "mach/mt_clkmgr3.h"
#else /* CONFIG_MACH_MT6735 */
#include "mach/mt_clkmgr1_legacy.h"
#endif
#else /* !CONFIG_MTK_CLKMGR */
#ifdef CONFIG_MACH_MT6735M
#error "Does not support common clock framework"
#elif defined(CONFIG_MACH_MT6753)
#error "Does not support common clock framework"
#else /* CONFIG_MACH_MT6735 */
#include "mach/mt_clkmgr1.h"
#endif
#endif

#endif
