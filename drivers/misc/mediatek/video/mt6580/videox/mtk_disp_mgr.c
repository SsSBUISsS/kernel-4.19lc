// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019 MediaTek Inc.
 *
 */

#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/module.h>
#include <generated/autoconf.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/param.h>
#include <linux/uaccess.h>
#include <linux/sched.h>

#include <linux/workqueue.h>
#include <linux/semaphore.h>
#include <linux/slab.h>

#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/module.h>
#include <linux/wait.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include "mtk_sync.h"
#include "disp_debug.h"
#include "disp_log.h"
#include "disp_lcm.h"
#include "disp_utils.h"
#include "ddp_hal.h"
#include "ddp_dump.h"
#include "ddp_path.h"
#include "ddp_drv.h"
#include "ddp_info.h"
#include "ddp_ovl.h"
#include "m4u.h"
#include "primary_display.h"
#include "cmdq_def.h"
#include "cmdq_record.h"
#include "cmdq_reg.h"
#include "cmdq_core.h"
#include "ddp_manager.h"
#include "disp_drv_platform.h"
#include "disp_recorder.h"
#include "mtk_disp_mgr.h"
#include "disp_session.h"
#include "mtk_ovl.h"
#include "ddp_mmp.h"
#include "mtkfb_fence.h"
#include "mtkfb_debug.h"

#ifdef CONFIG_MTK_HDMI_SUPPORT
#include "extd_hdmi_drv.h"
#endif

enum ePREPARE_FENCE_TYPE{
	PREPARE_INPUT_FENCE,
	PREPARE_OUTPUT_FENCE,
	PREPARE_PRESENT_FENCE
};

static dev_t mtk_disp_mgr_devno;
static struct cdev *mtk_disp_mgr_cdev;
static struct class *mtk_disp_mgr_class;
unsigned int is_hwc_enabled = 0;

static int mtk_disp_mgr_open(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t mtk_disp_mgr_read(struct file *file, char __user *data,
				 size_t len, loff_t *ppos)
{
	return 0;
}

static int mtk_disp_mgr_release(struct inode *inode, struct file *file)
{
	return 0;
}

static int mtk_disp_mgr_flush(struct file *a_pstFile, fl_owner_t a_id)
{
	return 0;
}

static int mtk_disp_mgr_mmap(struct file *file, struct vm_area_struct *vma)
{
	static const unsigned long addr_min = 0x14000000;
	static const unsigned long addr_max = 0x14025000;
	static const unsigned long size = addr_max - addr_min;
	const unsigned long require_size = vma->vm_end - vma->vm_start;
	unsigned long pa_start = vma->vm_pgoff << PAGE_SHIFT;
	unsigned long pa_end = pa_start + require_size;

	DISPDBG("mmap size %ld, vmpg0ff 0x%lx, pastart 0x%lx, paend 0x%lx\n",
		require_size, vma->vm_pgoff, pa_start, pa_end);

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	if (require_size > size || (pa_start < addr_min || pa_end > addr_max)) {
		DISPERR("mmap size range over flow!!\n");
		return -EAGAIN;
	}
	if (remap_pfn_range(vma,
			    vma->vm_start,
			    vma->vm_pgoff,
			    (vma->vm_end - vma->vm_start), vma->vm_page_prot)) {
		DISPERR("display mmap failed!!\n");
		return -EAGAIN;
	}

	return 0;
}

#define DDP_OUTPUT_LAYID 4

static unsigned int session_config[MAX_SESSION_COUNT];
static DEFINE_MUTEX(disp_session_lock);

struct OVL_CONFIG_STRUCT ovl2mem_in_cached_config[DDP_OVL_LAYER_MUN] = {
	{.layer = 0, .isDirty = 1},
	{.layer = 1, .isDirty = 1},
	{.layer = 2, .isDirty = 1},
	{.layer = 3, .isDirty = 1}
};

int _session_inited(struct disp_session_config config)
{
	return 0;
}

int disp_create_session(struct disp_session_config *config)
{
	int ret = 0;
	int is_session_inited = 0;
	unsigned int session =
	    MAKE_DISP_SESSION(config->type, config->device_id);
	int i, idx = -1;
	/* 1.To check if this session exists already */
	mutex_lock(&disp_session_lock);
	for (i = 0; i < MAX_SESSION_COUNT; i++) {
		if (session_config[i] == session) {
			is_session_inited = 1;
			idx = i;
			break;
		}
	}

	if (is_session_inited == 1) {
		config->session_id = session;
		goto done;
	}

	for (i = 0; i < MAX_SESSION_COUNT; i++) {
		if (session_config[i] == 0 && idx == -1) {
			idx = i;
			break;
		}
	}
	/* 1.To check if support this session (mode,type,dev) */
	/* TODO: */

	/* 2. Create this session */
	if (idx != -1) {
		config->session_id = session;
		session_config[idx] = session;
		DISPDBG("New session(0x%x)\n", session);
	} else {
		DISPERR("Invalid session creation request\n");
		ret = -1;
	}
done:
	mutex_unlock(&disp_session_lock);

	DISPPR_FENCE("new session:0x%x\n", config->session_id);
	if (DISP_SESSION_TYPE(session) == DISP_SESSION_MEMORY) {
		/* ovl2mem_init(session); */
		/* _ovl2mem_out_cached_config.mode = 3; */
	} else if (DISP_SESSION_TYPE(session) == DISP_SESSION_EXTERNAL) {
#ifdef CONFIG_MTK_HDMI_SUPPORT
		extd_hdmi_path_init(config->mode, session);
#endif
	}
	DISPMSG("new session done\n");

	return ret;
}

bool release_session_buffer(enum DISP_SESSION_TYPE type, unsigned int layerid,
			    unsigned long layer_phy_addr);
int disp_destroy_session(struct disp_session_config *config)
{
	int ret = -1;
	unsigned int session = config->session_id;
	int i;

	DISPMSG("disp_destroy_session, 0x%x", config->session_id);

	if (DISP_SESSION_TYPE(session) == DISP_SESSION_MEMORY) {
		/* ovl2mem_deinit(); */
		/* _ovl2mem_out_cached_config.mode = 0; */
	} else if (DISP_SESSION_TYPE(session) == DISP_SESSION_EXTERNAL) {
#ifdef CONFIG_MTK_HDMI_SUPPORT
		extd_hdmi_path_deinit();
#endif
	}

	release_session_buffer(DISP_SESSION_TYPE(config->session_id), 0xFF,
			       0);

	/* 1.To check if this session exists already, and remove it */
	mutex_lock(&disp_session_lock);
	for (i = 0; i < MAX_SESSION_COUNT; i++) {
		if (session_config[i] == session) {
			session_config[i] = 0;
			ret = 0;
			break;
		}
	}

	mutex_unlock(&disp_session_lock);
	DISPPR_FENCE("destroy_session done\n");

	/* 2. Destroy this session */
	if (ret == 0)
		DISPDBG("Destroy session(0x%x)\n", session);
	else
		DISPMSG("session(0x%x) does not exists\n", session);

	return ret;
}

bool release_session_buffer(enum DISP_SESSION_TYPE type, unsigned int layerid,
			    unsigned long layer_phy_addr)
{
	unsigned int session = 0;
	int i = 0;
	int buff_idx = 0;
	int release_layer_num = HW_OVERLAY_COUNT;

	if (type < DISP_SESSION_EXTERNAL)
		return false;

	for (i = 0; i < MAX_SESSION_COUNT; i++) {
		if (DISP_SESSION_TYPE(session_config[i]) == type) {
			session = session_config[i];
			break;
		}
	}

	if (session == 0)
		/* /||((layerid < HW_OVERLAY_COUNT) &&(layer_phy_addr == NULL))) */
	{
		DISPERR("sess(%u) (type %d)(addr 0x%lx) failed!\n", session,
			type, layer_phy_addr);
		return false;
	}

	mutex_lock(&disp_session_lock);

	if (layerid < release_layer_num) {
		buff_idx =
		    mtkfb_query_release_idx(session, layerid, layer_phy_addr);

		if (buff_idx > 0)
			DISPMSG
			    ("release_session_buffer layerid %x addr 0x%lx idx %d\n",
			     layerid, layer_phy_addr, buff_idx);

		if (buff_idx > 0)
			mtkfb_release_fence(session, layerid, buff_idx);
	} else {
		if (type == DISP_SESSION_MEMORY)
			release_layer_num = HW_OVERLAY_COUNT + 1;

		for (i = 0; i < release_layer_num; i++)
			mtkfb_release_layer_fence(session, i);

		DISPMSG("release_session_buffer(type %d) release layer %d\n",
			type, release_layer_num);
	}

	mutex_unlock(&disp_session_lock);
	return true;
}

int _ioctl_create_session(unsigned long arg)
{
	int ret = 0;
	void __user *argp = (void __user *)arg;
	struct disp_session_config config;

	if (copy_from_user(&config, argp, sizeof(config))) {
		DISPMSG("[FB]: copy_from_user failed! line:%d\n", __LINE__);
		return -EFAULT;
	}

	if (disp_create_session(&config) != 0)
		ret = -EFAULT;

	if (copy_to_user(argp, &config, sizeof(config))) {
		DISPMSG("[FB]: copy_to_user failed! line:%d\n", __LINE__);
		ret = -EFAULT;
	}

	return ret;
}

int _ioctl_destroy_session(unsigned long arg)
{
	int ret = 0;
	void __user *argp = (void __user *)arg;
	struct disp_session_config config;

	if (copy_from_user(&config, argp, sizeof(config))) {
		DISPMSG("[FB]: copy_from_user failed! line:%d\n", __LINE__);
		return -EFAULT;
	}

	if (disp_destroy_session(&config) != 0)
		ret = -EFAULT;

	return ret;
}

char *disp_session_mode_spy(unsigned int session_id)
{
	switch (DISP_SESSION_TYPE(session_id)) {
	case DISP_SESSION_PRIMARY:
		return "P";
	case DISP_SESSION_EXTERNAL:
		return "E";
	case DISP_SESSION_MEMORY:
		return "M";
	default:
		return "Unknown";
	}
}
/*
static unsigned int get_current_ticket(void)
{
	return dprec_get_vsync_count();
}
*/
int _ioctl_trigger_session(unsigned long arg)
{
	int ret = 0;
	void __user *argp = (void __user *)arg;
	struct disp_session_config config;
	unsigned int session_id = 0;
	unsigned long ticket = 0;
	struct disp_session_sync_info *session_info;

	if (copy_from_user(&config, argp, sizeof(config))) {
		DISPMSG("[FB]: copy_from_user failed! line:%d\n", __LINE__);
		return -EFAULT;
	}

	session_id = config.session_id;

	ticket = primary_display_get_ticket();
	session_info = disp_get_session_sync_info_for_debug(session_id);

	if (session_info) {
		unsigned int proc_name = (current->comm[0] << 24) |
		    (current->comm[1] << 16) |
		    (current->comm[2] << 8) | (current->comm[3] << 0);
		dprec_start(&session_info->event_trigger, proc_name, 0);
	}

	if (DISP_SESSION_TYPE(session_id) == DISP_SESSION_PRIMARY) {
		DISPPR_FENCE("T+/%s%d/%d\n", disp_session_mode_spy(session_id),
			     DISP_SESSION_DEV(session_id),
			     config.present_fence_idx);
	} else {
		DISPPR_FENCE("T+/%s%d\n", disp_session_mode_spy(session_id),
			     DISP_SESSION_DEV(session_id));
	}

	if (DISP_SESSION_TYPE(session_id) == DISP_SESSION_PRIMARY) {
		/* only primary display update present fence, external display has no present fence mechanism */
		if (config.present_fence_idx != (unsigned int)-1) {
			primary_display_update_present_fence
			    (config.present_fence_idx);
			mmprofile_log_ex(ddp_mmp_get_events()->present_fence_set,
				       MMPROFILE_FLAG_PULSE,
				       config.present_fence_idx, 0);
		}
		/* primary_display_trigger(0, NULL, config.need_merge); */
		primary_display_trigger(0, NULL, 0);
	} else if (DISP_SESSION_TYPE(session_id) == DISP_SESSION_EXTERNAL) {
#ifdef CONFIG_MTK_HDMI_SUPPORT
		mutex_lock(&disp_session_lock);
		ret = extd_hdmi_trigger(0, NULL, session_id);

		mutex_unlock(&disp_session_lock);
#endif
	} else if (DISP_SESSION_TYPE(session_id) == DISP_SESSION_MEMORY) {
		ovl2mem_trigger(1, NULL, 0);
	} else {
		DISPERR("session type is wrong:0x%08x\n", session_id);
		ret = -1;
	}

	if (session_info)
		dprec_done(&session_info->event_trigger, 0, 0);

	/* allow Idlemgr to switch DL to Decouple. */
	is_hwc_enabled = 1;
	return ret;
}


extern struct disp_sync_info * _get_sync_info(unsigned int session_id,
				      unsigned int timeline_id);
int _ioctl_prepare_present_fence(unsigned long arg)
{
	int ret = 0;

	void __user *argp = (void __user *)arg;
	struct mtk_sync_create_fence_data data;
	struct disp_present_fence preset_fence_struct;
	static unsigned int fence_idx;
	struct disp_sync_info *layer_info = NULL;
	int timeline_id = disp_sync_get_present_timeline_id();

	if (copy_from_user
	    (&preset_fence_struct, (void __user *)arg,
	     sizeof(struct disp_present_fence))) {
		DISPERR("[FB Driver]: copy_from_user failed! line:%d\n",
		       __LINE__);
		return (-EFAULT);
	}

	if (DISP_SESSION_TYPE(preset_fence_struct.session_id) !=
	    DISP_SESSION_PRIMARY) {
		DISPERR("non-primary ask for present fence! session=0x%x\n",
			preset_fence_struct.session_id);
		data.fence = MTK_FB_INVALID_FENCE_FD;
		data.value = 0;
	} else {
		layer_info =
		    _get_sync_info(preset_fence_struct.session_id, timeline_id);
		if (layer_info == NULL) {
			DISPERR("layer_info is null\n");
			ret = -EFAULT;
			return ret;
		}
		/* create fence */
		data.fence = MTK_FB_INVALID_FENCE_FD;
		data.value = ++fence_idx;
		ret = fence_create(layer_info->timeline, &data);
		if (ret != 0) {
			DISPPR_ERROR
			    ("%s%d,layer%d create Fence Object failed!\n",
			     disp_session_mode_spy
			     (preset_fence_struct.session_id),
			     DISP_SESSION_DEV(preset_fence_struct.session_id),
			     timeline_id);
			ret = -EFAULT;
		}
	}

	preset_fence_struct.present_fence_fd = data.fence;
	preset_fence_struct.present_fence_index = data.value;
	if (copy_to_user
	    (argp, &preset_fence_struct, sizeof(preset_fence_struct))) {
		DISPERR("[FB Driver]: copy_to_user failed! line:%d\n",
		       __LINE__);
		ret = -EFAULT;
	}
	mmprofile_log_ex(ddp_mmp_get_events()->present_fence_get,
		       MMPROFILE_FLAG_PULSE, preset_fence_struct.present_fence_fd,
		       preset_fence_struct.present_fence_index);
	/* DISPPR_FENCE("P+/%s%d/L%d/id%d/fd%d\n",
		     disp_session_mode_spy(preset_fence_struct.session_id),
		     DISP_SESSION_DEV(preset_fence_struct.session_id),
		     timeline_id, preset_fence_struct.present_fence_index,
		     preset_fence_struct.present_fence_fd); */

	return ret;
}

int _ioctl_prepare_buffer(unsigned long arg, enum ePREPARE_FENCE_TYPE type)
{
	int ret = 0;
	void __user *argp = (void __user *)arg;
	struct disp_buffer_info info;
	struct mtkfb_fence_buf_info *buf, *buf2;

	if (copy_from_user(&info, (void __user *)arg, sizeof(info))) {
		DISPERR("[FB Driver]: copy_from_user failed! line:%d\n", __LINE__);
		return -EFAULT;
	}

	if (type == PREPARE_INPUT_FENCE)
		;
	else if (type == PREPARE_PRESENT_FENCE)
		info.layer_id = disp_sync_get_present_timeline_id();
	else if (type == PREPARE_OUTPUT_FENCE)
		info.layer_id = disp_sync_get_output_timeline_id();
	else
		DISPPR_ERROR("type is wrong: %d\n", type);

	if (info.layer_en && info.ion_fd >= 0) {
		buf = disp_sync_prepare_buf(&info);
		if (buf != NULL) {
			info.fence_fd = buf->fence;
			info.index = buf->idx;
		} else {
			DISPPR_ERROR("prepare fence failed, 0x%08x/l%d/e%d/ion%d/cache%d\n",
				     info.session_id, info.layer_id, info.layer_en, info.ion_fd,
				     info.cache_sync);
			DISPPR_FENCE
			    ("P+ FAIL /%s%d/l%d/e%d/ion%d/c%d/id%d/ffd%d\n",
			     disp_session_mode_spy(info.session_id),
			     DISP_SESSION_DEV(info.session_id), info.layer_id,
			     info.layer_en, info.ion_fd, info.cache_sync,
			     info.index, info.fence_fd);
			info.fence_fd = MTK_FB_INVALID_FENCE_FD;	/* invalid fd */
			info.index = 0;
		}

		if (type == PREPARE_OUTPUT_FENCE) {
			if (primary_display_is_decouple_mode()
			    && primary_display_is_mirror_mode()) {
				/*create second fence for wdma when decouple mirror mode */
				info.layer_id =
				    disp_sync_get_output_interface_timeline_id
				    ();
				buf2 = disp_sync_prepare_buf(&info);
				if (buf2 != NULL) {
					info.interface_fence_fd = buf2->fence;
					info.interface_index = buf2->idx;
				} else {
					DISPPR_ERROR("prepare fence failed, 0x%08x/l%d/e%d/ion%d/cache%d\n",
						     info.session_id, info.layer_id, info.layer_en,
						     info.ion_fd, info.cache_sync);
					DISPPR_FENCE
					    ("P+ FAIL /%s%d/l%d/e%d/ion%d/c%d/id%d/ffd%d\n",
					     disp_session_mode_spy
					     (info.session_id),
					     DISP_SESSION_DEV(info.session_id),
					     info.layer_id, info.layer_en,
					     info.ion_fd, info.cache_sync,
					     info.index, info.fence_fd);
					info.interface_fence_fd = MTK_FB_INVALID_FENCE_FD;	/* invalid fd */
					info.interface_index = 0;
				}
			} else {
				info.interface_fence_fd = MTK_FB_INVALID_FENCE_FD;	/* invalid fd */
				info.interface_index = 0;
			}
		}
	} else {
		DISPPR_ERROR("wrong prepare param, 0x%08x/l%d/e%d/ion%d/cache%d\n", info.session_id,
			     info.layer_id, info.layer_en, info.ion_fd, info.cache_sync);
		DISPPR_FENCE("P+ FAIL /%s%d/l%d/e%d/ion%d/c%d/id%d/ffd%d\n",
			     disp_session_mode_spy(info.session_id),
			     DISP_SESSION_DEV(info.session_id), info.layer_id,
			     info.layer_en, info.ion_fd, info.cache_sync,
			     info.index, info.fence_fd);
		info.fence_fd = MTK_FB_INVALID_FENCE_FD;	/* invalid fd */
		info.index = 0;
	}
	if (copy_to_user(argp, &info, sizeof(info))) {
		DISPERR("[FB Driver]: copy_to_user failed! line:%d\n", __LINE__);
		ret = -EFAULT;
	}
	return ret;
}

const char *_disp_format_spy(enum DISP_FORMAT format)
{
	switch (format) {
	case DISP_FORMAT_RGB565:
		return "RGB565";
	case DISP_FORMAT_RGB888:
		return "RGB888";
	case DISP_FORMAT_BGR888:
		return "BGR888";
	case DISP_FORMAT_ARGB8888:
		return "ARGB8888";
	case DISP_FORMAT_ABGR8888:
		return "ABGR8888";
	case DISP_FORMAT_RGBA8888:
		return "RGBA8888";
	case DISP_FORMAT_BGRA8888:
		return "BGRA8888";
	case DISP_FORMAT_YUV422:
		return "YUV422";
	case DISP_FORMAT_XRGB8888:
		return "XRGB8888";
	case DISP_FORMAT_XBGR8888:
		return "XBGR8888";
	case DISP_FORMAT_RGBX8888:
		return "RGBX8888";
	case DISP_FORMAT_BGRX8888:
		return "BGRX8888";
	case DISP_FORMAT_UYVY:
		return "UYVY";
	case DISP_FORMAT_YUV420_P:
		return "YUV420_P";
	case DISP_FORMAT_YV12:
		return "YV12";
	default:
		return "unknown";
	}
}

static int _sync_convert_fb_layer_to_ovl_struct(unsigned int session_id,
						struct disp_input_config *src,
						struct OVL_CONFIG_STRUCT *dst,
						unsigned int dst_mva)
{
	unsigned int layerpitch = 0;
	unsigned int layerbpp = 0;

	dst->layer = src->layer_id;

	if (!src->layer_enable) {
		dst->layer_en = 0;
		dst->isDirty = true;
		return 0;
	}

	switch (src->src_fmt) {
	case DISP_FORMAT_YUV422:
		dst->fmt = eYUY2;
		layerpitch = 2;
		layerbpp = 16;
		break;

	case DISP_FORMAT_RGB565:
		dst->fmt = eRGB565;
		layerpitch = 2;
		layerbpp = 16;
		break;

	case DISP_FORMAT_RGB888:
		dst->fmt = eRGB888;
		layerpitch = 3;
		layerbpp = 24;
		break;

	case DISP_FORMAT_BGR888:
		dst->fmt = eBGR888;
		layerpitch = 3;
		layerbpp = 24;
		break;
		/* xuecheng, ?????? */
	case DISP_FORMAT_ARGB8888:
		/* dst->fmt = eRGBA8888; */
		dst->fmt = eARGB8888;
		layerpitch = 4;
		layerbpp = 32;
		break;

	case DISP_FORMAT_ABGR8888:
		dst->fmt = eABGR8888;
		layerpitch = 4;
		layerbpp = 32;
		break;
	case DISP_FORMAT_RGBA8888:
		dst->fmt = eRGBA8888;
		layerpitch = 4;
		layerbpp = 32;
		break;

	case DISP_FORMAT_BGRA8888:
		/* dst->fmt = eABGR8888; */
		dst->fmt = eBGRA8888;
		layerpitch = 4;
		layerbpp = 32;
		break;
	case DISP_FORMAT_XRGB8888:
		dst->fmt = eARGB8888;
		layerpitch = 4;
		layerbpp = 32;
		break;

	case DISP_FORMAT_XBGR8888:
		dst->fmt = eABGR8888;
		layerpitch = 4;
		layerbpp = 32;
		break;
	case DISP_FORMAT_RGBX8888:
		dst->fmt = eRGBA8888;
		layerpitch = 4;
		layerbpp = 32;
		break;
	case DISP_FORMAT_BGRX8888:
		dst->fmt = eBGRA8888;
		layerpitch = 4;
		layerbpp = 32;
		break;

	case DISP_FORMAT_UYVY:
		dst->fmt = eUYVY;
		layerpitch = 2;
		layerbpp = 16;
		break;

	default:
		DISPERR("Invalid color format: 0x%x\n", src->src_fmt);
		return -1;
	}
	dst->source = (enum OVL_LAYER_SOURCE)src->buffer_source;

	dst->vaddr = (unsigned long)src->src_base_addr;
	dst->security = src->security;
	if (src->src_phy_addr != NULL)
		dst->addr = (unsigned long)src->src_phy_addr;
	else
		dst->addr = dst_mva;

	dst->isTdshp = src->isTdshp;
	dst->buff_idx = src->next_buff_idx;
	dst->identity = src->identity;
	dst->connected_type = src->connected_type;

	/* set Alpha blending */
	dst->aen = src->alpha_enable;
	dst->alpha = src->alpha;
	dst->sur_aen = src->sur_aen;
	dst->src_alpha = src->src_alpha;
	dst->dst_alpha = src->dst_alpha;

	if (DISP_FORMAT_XRGB8888 == src->src_fmt ||
	    DISP_FORMAT_XBGR8888 == src->src_fmt ||
	    DISP_FORMAT_RGBX8888 == src->src_fmt ||
	    DISP_FORMAT_BGRX8888 == src->src_fmt) {
		dst->aen = false;
	}

	/* set src width, src height */
	dst->src_x = src->src_offset_x;
	dst->src_y = src->src_offset_y;
	dst->src_w = src->src_width;
	dst->src_h = src->src_height;
	dst->dst_x = src->tgt_offset_x;
	dst->dst_y = src->tgt_offset_y;
	dst->dst_w = src->tgt_width;
	dst->dst_h = src->tgt_height;
	if (dst->dst_w > dst->src_w)
		dst->dst_w = dst->src_w;
	if (dst->dst_h > dst->src_h)
		dst->dst_h = dst->src_h;

	dst->src_pitch = src->src_pitch * layerpitch;
	/* set color key */
	dst->key = src->src_color_key;
	dst->keyEn = src->src_use_color_key;

	/* data transferring is triggerred in MTKFB_TRIG_OVERLAY_OUT */
	dst->layer_en = src->layer_enable;
	dst->yuv_range = src->yuv_range;
	dst->isDirty = true;
	if (src->buffer_source == DISP_BUFFER_ALPHA) {
		dst->source = OVL_LAYER_SOURCE_RESERVED;	/* dim layer, constant alpha */
	} else if (src->buffer_source == DISP_BUFFER_ION
		   || src->buffer_source == DISP_BUFFER_MVA) {
		dst->source = OVL_LAYER_SOURCE_MEM;	/* from memory */
	} else {
		DISPERR("unknown source = %d", src->buffer_source);
		dst->source = OVL_LAYER_SOURCE_MEM;
	}

	return 0;
}

static int _sync_convert_fb_layer_to_disp_input(unsigned int session_id,
						struct disp_input_config *src,
						struct primary_disp_input_config *dst,
						unsigned int dst_mva)
{
	unsigned int layerpitch = 0;
	unsigned int layerbpp = 0;

	dst->layer = src->layer_id;

	if (!src->layer_enable) {
		dst->layer_en = 0;
		dst->isDirty = true;
		return 0;
	}

	switch (src->src_fmt) {
	case DISP_FORMAT_YUV422:
		dst->fmt = eYUY2;
		layerpitch = 2;
		layerbpp = 16;
		break;

	case DISP_FORMAT_RGB565:
		dst->fmt = eRGB565;
		layerpitch = 2;
		layerbpp = 16;
		break;

	case DISP_FORMAT_RGB888:
		dst->fmt = eRGB888;
		layerpitch = 3;
		layerbpp = 24;
		break;

	case DISP_FORMAT_BGR888:
		dst->fmt = eBGR888;
		layerpitch = 3;
		layerbpp = 24;
		break;
		/* xuecheng, ?????? */
	case DISP_FORMAT_ARGB8888:
		dst->fmt = eARGB8888;
		layerpitch = 4;
		layerbpp = 32;
		break;

	case DISP_FORMAT_ABGR8888:
		dst->fmt = eABGR8888;
		layerpitch = 4;
		layerbpp = 32;
		break;
	case DISP_FORMAT_RGBA8888:
		dst->fmt = eRGBA8888;
		layerpitch = 4;
		layerbpp = 32;
		break;

	case DISP_FORMAT_BGRA8888:
		/* dst->fmt = eABGR8888; */
		dst->fmt = eBGRA8888;
		layerpitch = 4;
		layerbpp = 32;
		break;
	case DISP_FORMAT_XRGB8888:
		dst->fmt = eARGB8888;
		layerpitch = 4;
		layerbpp = 32;
		break;

	case DISP_FORMAT_XBGR8888:
		dst->fmt = eABGR8888;
		layerpitch = 4;
		layerbpp = 32;
		break;

	case DISP_FORMAT_RGBX8888:
		dst->fmt = eRGBA8888;
		layerpitch = 4;
		layerbpp = 32;
		break;

	case DISP_FORMAT_BGRX8888:
		dst->fmt = eBGRA8888;
		layerpitch = 4;
		layerbpp = 32;
		break;

	case DISP_FORMAT_UYVY:
		dst->fmt = eUYVY;
		layerpitch = 2;
		layerbpp = 16;
		break;

	case DISP_FORMAT_YV12:
		dst->fmt = eYV12;
		layerpitch = 1;
		layerbpp = 8;
		break;
	default:
		DISPERR("Invalid color format: 0x%x\n", src->src_fmt);
		return -1;
	}
	dst->buffer_source = src->buffer_source;

	dst->vaddr = (unsigned long)src->src_base_addr;
	dst->security = src->security;
	if (src->src_phy_addr != NULL)
		dst->addr = (unsigned long)src->src_phy_addr;
	else
		dst->addr = dst_mva;

	dst->isTdshp = src->isTdshp;
	dst->buff_idx = src->next_buff_idx;
	dst->identity = src->identity;
	dst->connected_type = src->connected_type;

	/* set Alpha blending */
	dst->aen = src->alpha_enable;
	dst->alpha = src->alpha;
	dst->sur_aen = src->sur_aen;
	dst->src_alpha = src->src_alpha;
	dst->dst_alpha = src->dst_alpha;

	if (DISP_FORMAT_ARGB8888 == src->src_fmt
	    || DISP_FORMAT_ABGR8888 == src->src_fmt
	    || DISP_FORMAT_RGBA8888 == src->src_fmt
	    || DISP_FORMAT_BGRA8888 == src->src_fmt) {
		dst->aen = true;
	}

	/* set src width, src height */
	dst->src_x = src->src_offset_x;
	dst->src_y = src->src_offset_y;
	dst->src_w = src->src_width;
	dst->src_h = src->src_height;
	dst->dst_x = src->tgt_offset_x;
	dst->dst_y = src->tgt_offset_y;
	dst->dst_w = src->tgt_width;
	dst->dst_h = src->tgt_height;
	if (dst->dst_w > dst->src_w)
		dst->dst_w = dst->src_w;
	if (dst->dst_h > dst->src_h)
		dst->dst_h = dst->src_h;

	dst->src_pitch = src->src_pitch * layerpitch;

	/* set color key */
	dst->key = src->src_color_key;
	dst->keyEn = src->src_use_color_key;

	/* data transferring is triggerred in MTKFB_TRIG_OVERLAY_OUT */
	dst->layer_en = src->layer_enable;
	dst->isDirty = true;
	dst->yuv_range = src->yuv_range;
	dst->buffer_source = src->buffer_source;

	return 0;
}

static int set_memory_buffer(struct disp_session_input_config *input)
{
	int i = 0;
	unsigned int layer_id = 0;
	unsigned int dst_size = 0;
	unsigned long dst_mva = 0;
	unsigned int session_id = input->session_id;
	struct disp_session_sync_info *session_info = disp_get_session_sync_info_for_debug(session_id);

	struct ovl2mem_in_config input_params[HW_OVERLAY_COUNT];

	memset((void *)&input_params, 0, sizeof(input_params));
	for (i = 0; i < input->config_layer_num; i++) {
		dst_mva = 0;
		layer_id = input->config[i].layer_id;
		if (input->config[i].layer_enable) {
			if (input->config[i].buffer_source == DISP_BUFFER_ALPHA) {
				DISPPR_FENCE("ML %d is dim layer,fence %d\n",
					     input->config[i].layer_id,
					     input->config[i].next_buff_idx);
				input->config[i].src_offset_x = 0;
				input->config[i].src_offset_y = 0;
				input->config[i].sur_aen = 0;
				input->config[i].src_fmt = DISP_FORMAT_RGB888;
				input->config[i].src_pitch = input->config[i].src_width;
				input->config[i].src_phy_addr = (void *)get_dim_layer_mva_addr();
				input->config[i].next_buff_idx = 0;
			}
			if (input->config[i].src_phy_addr) {
				dst_mva = (unsigned long)input->config[i].src_phy_addr;
			} else {
				disp_sync_query_buf_info(session_id, (unsigned int)layer_id,
							 (unsigned int)
							 input->config
							 [i].next_buff_idx,
							 &dst_mva, &dst_size);
			}

			if (dst_mva == 0)
				input->config[i].layer_enable = 0;

			DISPPR_FENCE
			    ("S+/ML%d/e%d/id%d/%dx%d(%d,%d)(%d,%d)/%s/%d/0x%p/mva0x%08lx/t%d\n",
			     input->config[i].layer_id,
			     input->config[i].layer_enable,
			     input->config[i].next_buff_idx,
			     input->config[i].src_width,
			     input->config[i].src_height,
			     input->config[i].src_offset_x,
			     input->config[i].src_offset_y,
			     input->config[i].tgt_offset_x,
			     input->config[i].tgt_offset_y,
			     _disp_format_spy(input->config[i].src_fmt),
			     input->config[i].src_pitch,
			     input->config[i].src_phy_addr, dst_mva,
			     get_ovl2mem_ticket());
		} else {
			DISPPR_FENCE("S+/ML%d/e%d/id%d\n",
				     input->config[i].layer_id,
				     input->config[i].layer_enable,
				     input->config[i].next_buff_idx);
		}
		if (layer_id < ARRAY_SIZE(ovl2mem_in_cached_config))
			_sync_convert_fb_layer_to_ovl_struct(input->session_id,
						     &(input->config[i]),
						     &ovl2mem_in_cached_config
						     [layer_id], dst_mva);
		else {
			DISPERR("layer_id:%u, out of the bounds\n", layer_id);
			BUG_ON(1);
		}
		mtkfb_update_buf_ticket(session_id, layer_id,
					input->config[i].next_buff_idx,
					get_ovl2mem_ticket());
		_sync_convert_fb_layer_to_disp_input(input->session_id,
						     &(input->config[i]),
						     (struct primary_disp_input_config *)&input_params[layer_id],
						     (unsigned int)dst_mva);
		input_params[layer_id].dirty = 1;

		if (input->config[i].layer_enable) {
			mtkfb_update_buf_info(input->session_id,
					      input->config[i].layer_id,
					      input->config[i].next_buff_idx, 0,
					      input->config[i].frm_sequence);
		}
		if (session_info) {
			dprec_submit(&session_info->event_setinput,
				     input->config[i].next_buff_idx,
				     (input->
				      config_layer_num << 28) |
				     (input->config[i].layer_id << 24) |
				     (input->config[i].
				      src_fmt << 12) | input->config[i].
				     layer_enable);
		}
	}

	ovl2mem_input_config((struct ovl2mem_in_config *)&input_params);

	return 0;
}

static int set_primary_buffer(struct disp_session_input_config *input)
{
	int i = 0;
	int layer_id = 0;
	unsigned int dst_size = 0;
	unsigned long dst_mva = 0;
	unsigned int session_id = 0;
	unsigned int mva_offset = 0;
	char fence_msg_buf[512];
	unsigned int fence_msg_len = 0;
	struct disp_session_sync_info *session_info;

	session_id = input->session_id;
	session_info = disp_get_session_sync_info_for_debug(session_id);

	if (input->config_layer_num == 0
	    || input->config_layer_num > OVL_LAYER_NUM) {
		DISPERR("set_primary_buffer, config_layer_num invalid = %d!\n",
			input->config_layer_num);
		return 0;
	}

	for (i = 0; i < input->config_layer_num; i++) {
		dst_mva = 0;

		layer_id = input->config[i].layer_id;
		if (layer_id >= OVL_LAYER_NUM) {
			DISPERR("set_primary_buffer, invalid layer_id = %d!\n", layer_id);
			continue;
		}
		if (layer_id == primary_display_get_option("ASSERT_LAYER") && is_DAL_Enabled()) {
			DISPPR_FENCE("Skip L3/AEE_Layer, Due to AEE is enable now.\n");
			continue;
		}

		if (input->config[i].layer_enable) {
			unsigned int hw_fmt, Bpp, bpp, x, y, pitch;

			if (input->config[i].buffer_source == DISP_BUFFER_ALPHA) {
				fence_msg_len += sprintf(fence_msg_buf + fence_msg_len,
							 "PL %d is dim layer,fence %d\n",
							 input->config[i].layer_id,
							 input->config[i].next_buff_idx);
				input->config[i].src_offset_x = 0;
				input->config[i].src_offset_y = 0;
				input->config[i].sur_aen = 0;
				input->config[i].src_fmt = DISP_FORMAT_RGB888;
				input->config[i].src_pitch =
				    input->config[i].src_width;
				input->config[i].src_phy_addr =
				    (void *)get_dim_layer_mva_addr();
				input->config[i].next_buff_idx = 0;
			}
			if (input->config[i].src_phy_addr) {
				dst_mva = (unsigned long)input->config[i].src_phy_addr;
			} else {
				disp_sync_query_buf_info(session_id, (unsigned int)layer_id,
							 (unsigned int)
							 input->config
							 [i].next_buff_idx,
							 &dst_mva, &dst_size);
			}

			input->config[i].src_phy_addr = (void *)dst_mva;

			if (dst_mva == 0) {
				DISPPR_ERROR
				    ("disable layer %d because of no valid mva\n",
				     input->config[i].layer_id);
				input->config[i].layer_enable = 0;
			}
			/* OVL addr is not the start address of buffer, which is calculated by pitch and ROI. */
			x = input->config[i].src_offset_x;
			y = input->config[i].src_offset_y;
			pitch = input->config[i].src_pitch;
			disp_fmt_to_hw_fmt(input->config[i].src_fmt, &hw_fmt,
					   &Bpp, &bpp);
			mva_offset = (x + y * pitch) * Bpp;
			mtkfb_update_buf_info(input->session_id,
					      input->config[i].layer_id,
					      input->config[i].next_buff_idx,
					      mva_offset,
					      input->config[i].frm_sequence);
			fence_msg_len += sprintf(fence_msg_buf + fence_msg_len,
						 "S+/PL%d/e%d/id%d/%dx%d(%d,%d)(%d,%d)/%s/%d/0x%08lx/mva0x%08lx/sec%d",
						 input->config[i].layer_id, input->config[i].layer_enable,
						 input->config[i].next_buff_idx, input->config[i].src_width,
						 input->config[i].src_height, input->config[i].src_offset_x,
						 input->config[i].src_offset_y, input->config[i].tgt_offset_x,
						 input->config[i].tgt_offset_y,
						 _disp_format_spy(input->config[i].src_fmt),
						 input->config[i].src_pitch,
						 (unsigned int long)input->config[i].src_phy_addr, dst_mva,
						 input->config[i].security);
		} else {
			fence_msg_len += sprintf(fence_msg_buf + fence_msg_len, "S+/PL%d/e%d/id%d",
						 input->config[i].layer_id, input->config[i].layer_enable,
						 input->config[i].next_buff_idx);
		}

		disp_sync_put_cached_layer_info(session_id, layer_id,
						&input->config[i], dst_mva);

		if (session_info) {
			dprec_submit(&session_info->event_setinput,
				     input->config[i].next_buff_idx, dst_mva);
		}
	}
	DISPPR_FENCE("%s\n", fence_msg_buf);
	primary_display_config_input_multiple(input);

	return 0;

}

int _ioctl_set_input_buffer(unsigned long arg)
{
	int ret = 0;
	void __user *argp = (void __user *)arg;
	unsigned int session_id = 0;
	struct disp_session_input_config session_input;
	struct disp_session_sync_info *session_info;

	if (copy_from_user(&session_input, argp, sizeof(session_input))) {
		DISPERR("[FB]: copy_from_user failed! line:%d\n", __LINE__);
		return -EFAULT;
	}
	session_input.setter = SESSION_USER_HWC;
	session_id = session_input.session_id;
	session_info = disp_get_session_sync_info_for_debug(session_id);

	if (session_info)
		dprec_start(&session_info->event_setinput, 0, session_input.config_layer_num);

	DISPPR_FENCE("S+/%s%d/count%d\n", disp_session_mode_spy(session_id),
		     DISP_SESSION_DEV(session_id),
		     session_input.config_layer_num);

	if (DISP_SESSION_TYPE(session_id) == DISP_SESSION_PRIMARY) {
		ret = set_primary_buffer(&session_input);
	} else if (DISP_SESSION_TYPE(session_id) == DISP_SESSION_EXTERNAL) {
#ifdef CONFIG_MTK_HDMI_SUPPORT
		ret = set_external_buffer(&session_input);
#endif
	} else if (DISP_SESSION_TYPE(session_id) == DISP_SESSION_MEMORY) {
		ret = set_memory_buffer(&session_input);
	} else {
		DISPERR("session type is wrong:0x%08x\n", session_id);
		return -1;
	}
	if (session_info) {
		dprec_done(&session_info->event_setinput, 0,
			   session_input.config_layer_num);
	}

	return ret;
}

static int _sync_convert_fb_layer_to_disp_output(unsigned int session_id,
						 struct disp_output_config *src,
						 struct disp_mem_output_config *dst,
						 unsigned int dst_mva)
{
	unsigned int layerpitch = 0;
	unsigned int layerbpp = 0;

	disp_fmt_to_hw_fmt(src->fmt, &dst->fmt, &layerpitch, &layerbpp);
	dst->vaddr = (unsigned long)src->va;
	dst->security = src->security;
	dst->w = src->width;
	dst->h = src->height;

/* set_overlay will not use fence+ion handle */
#if defined(MTK_FB_ION_SUPPORT)
	if (src->pa != NULL)
		dst->addr = (unsigned long)src->pa;
	else
		dst->addr = dst_mva;
#else
	dst->addr = (unsigned long)src->pa;
#endif

	dst->buff_idx = src->buff_idx;
	dst->interface_idx = src->interface_idx;

	dst->x = src->x;
	dst->y = src->y;
	dst->pitch = src->pitch * layerpitch;

	return 0;
}

int _ioctl_set_output_buffer(unsigned long arg)
{
	int ret = 0;
	void __user *argp = (void __user *)arg;
	struct disp_session_output_config session_output;
	unsigned int session_id = 0;
	unsigned long dst_mva = 0;
	struct disp_session_sync_info *session_info;

	if (copy_from_user(&session_output, argp, sizeof(session_output))) {
		DISPERR("[FB]: copy_from_user failed! line:%d\n", __LINE__);
		return -EFAULT;
	}

	session_id = session_output.session_id;
	session_info = disp_get_session_sync_info_for_debug(session_id);

	if (session_info)
		dprec_start(&session_info->event_setoutput, session_output.config.buff_idx, 0);
	else
		DISPERR("can't get session_info for session_id:0x%08x\n", session_id);


	DISPMSG(" _ioctl_set_output_buffer idx %x\n",
		session_output.config.buff_idx);
	if (DISP_SESSION_TYPE(session_id) == DISP_SESSION_PRIMARY) {
		struct disp_mem_output_config primary_output;

		memset((void *)&primary_output, 0, sizeof(primary_output));
		if (session_output.config.pa) {
			dst_mva = (unsigned long)session_output.config.pa;
		} else {
			dst_mva =
			    mtkfb_query_buf_mva(session_id,
						disp_sync_get_output_timeline_id
						(),
						(unsigned int)
						session_output.config.buff_idx);
		}

		_sync_convert_fb_layer_to_disp_output(session_output.session_id,
						      &(session_output.config),
						      &primary_output, dst_mva);
		primary_output.dirty = 1;
		/* must be mirror mode */
		if (primary_display_is_decouple_mode()) {
			struct disp_input_config src;

			memset((void *)&src, 0, sizeof(struct disp_input_config));
			src.layer_id =
			    disp_sync_get_output_interface_timeline_id();
			src.layer_enable = 1;
			src.next_buff_idx = session_output.config.interface_idx;

			disp_sync_put_cached_layer_info(session_id,
							disp_sync_get_output_interface_timeline_id
							(), &src, dst_mva);

			memset((void *)&src, 0, sizeof(struct disp_input_config));
			src.layer_id = disp_sync_get_output_timeline_id();
			src.layer_enable = 1;
			src.next_buff_idx = session_output.config.buff_idx;

			disp_sync_put_cached_layer_info(session_id,
							disp_sync_get_output_timeline_id
							(), &src, dst_mva);
		}

		DISPPR_FENCE
		    ("S+O/%s%d/L%d(id%d)/L%d(id%d)/%dx%d(%d,%d)/%s/%d/0x%08x/mva0x%08lx\n",
		     disp_session_mode_spy(session_id),
		     DISP_SESSION_DEV(session_id),
		     disp_sync_get_output_timeline_id(),
		     session_output.config.buff_idx,
		     disp_sync_get_output_interface_timeline_id(),
		     session_output.config.interface_idx,
		     session_output.config.width, session_output.config.height,
		     session_output.config.x, session_output.config.y,
		     _disp_format_spy(session_output.config.fmt),
		     session_output.config.pitch, session_output.config.pitchUV,
		     dst_mva);

		primary_display_config_output(&primary_output);

		mtkfb_update_buf_info(session_output.session_id,
				      disp_sync_get_output_interface_timeline_id(),
				      session_output.config.buff_idx, 0,
				      session_output.config.frm_sequence);
		if (session_info) {
			dprec_submit(&session_info->event_setoutput,
				     session_output.config.buff_idx, dst_mva);
		}
		DISPMSG
		    ("_ioctl_set_output_buffer done idx 0x%x, mva %lx, fmt %x, w %x, h %x (%x %x), p %x\n",
		     session_output.config.buff_idx, dst_mva,
		     session_output.config.fmt, session_output.config.width,
		     session_output.config.height, session_output.config.x,
		     session_output.config.y, session_output.config.pitch);

	} else if (DISP_SESSION_TYPE(session_id) == DISP_SESSION_MEMORY) {
		struct disp_mem_output_config primary_output;

		memset((void *)&primary_output, 0, sizeof(primary_output));
		if (session_output.config.pa) {
			dst_mva = (unsigned long)session_output.config.pa;
		} else {
			dst_mva =
			    mtkfb_query_buf_mva(session_id,
						disp_sync_get_output_timeline_id
						(),
						(unsigned int)
						session_output.config.buff_idx);
		}

		mtkfb_update_buf_ticket(session_id,
					disp_sync_get_output_timeline_id(),
					session_output.config.buff_idx,
					get_ovl2mem_ticket());
		_sync_convert_fb_layer_to_disp_output(session_output.session_id,
						      &(session_output.config),
						      &primary_output, dst_mva);
		primary_output.dirty = 1;

		DISPPR_FENCE
		    ("S+/%s%d/L%d/id%d/%dx%d(%d,%d)/%s/%d/%d/0x%p/mva0x%08lx/t%d\n",
		     disp_session_mode_spy(session_id),
		     DISP_SESSION_DEV(session_id), 4,
		     session_output.config.buff_idx,
		     session_output.config.width, session_output.config.height,
		     session_output.config.x, session_output.config.y,
		     _disp_format_spy(session_output.config.fmt),
		     session_output.config.pitch, session_output.config.pitchUV,
		     session_output.config.pa, dst_mva, get_ovl2mem_ticket());

		if (dst_mva)
			ovl2mem_output_config(&primary_output);
		else
			DISPERR("error buffer idx 0x%x\n",
				session_output.config.buff_idx);

		mtkfb_update_buf_info(session_output.session_id,
				      disp_sync_get_output_timeline_id(),
				      session_output.config.buff_idx, 0,
				      session_output.config.frm_sequence);

		if (session_info) {
			dprec_submit(&session_info->event_setoutput,
				     session_output.config.buff_idx, dst_mva);
		}
		DISPMSG
		    ("_ioctl_set_output_buffer done idx 0x%x, mva %lx, fmt %x, w %x, h %x, p %x\n",
		     session_output.config.buff_idx, dst_mva,
		     session_output.config.fmt, session_output.config.width,
		     session_output.config.height, session_output.config.pitch);
	}

	if (session_info) {
		dprec_done(&session_info->event_setoutput,
			   session_output.config.buff_idx, 0);
	}

	return ret;
}

int _ioctl_get_lcm_index(unsigned long arg)
{
	int ret = 0;
	void __user *argp = (void __user *)arg;
	int index = primary_display_get_lcm_index();

	if (copy_to_user(argp, &index, sizeof(int))) {
		DISPERR("display drvier: get lcm index failed\n");
		ret = -EFAULT;
	}
	return ret;
}

int _ioctl_get_info(unsigned long arg)
{
	int ret = 0;
	void __user *argp = (void __user *)arg;
	struct disp_session_info info;
	unsigned int session_id = 0;

	if (copy_from_user(&info, argp, sizeof(info))) {
		DISPERR("[FB]: copy_from_user failed! line:%d\n", __LINE__);
		return -EFAULT;
	}

	session_id = info.session_id;

	if (DISP_SESSION_TYPE(session_id) == DISP_SESSION_PRIMARY) {
		primary_display_get_info(&info);
	} else if (DISP_SESSION_TYPE(session_id) == DISP_SESSION_EXTERNAL) {
#ifdef CONFIG_MTK_HDMI_SUPPORT
		/* this is for session test */
		_get_ext_disp_info(&info);
#endif
	} else if (DISP_SESSION_TYPE(session_id) == DISP_SESSION_MEMORY) {
		ovl2mem_get_info(&info);
	} else {
		DISPERR("session type is wrong:0x%08x\n", session_id);
		return -1;
	}

	if (copy_to_user(argp, &info, sizeof(info))) {
		DISPERR("[FB]: copy_to_user failed! line:%d\n", __LINE__);
		ret = -EFAULT;
	}

	return ret;
}

int _ioctl_get_is_driver_suspend(unsigned long arg)
{
	int ret = 0;
	void __user *argp = (void __user *)arg;
	unsigned int is_suspend = 0;

	is_suspend = primary_display_is_sleepd();
	DISPMSG("ioctl_get_is_driver_suspend, is_suspend=%d\n", is_suspend);
	if (copy_to_user(argp, &is_suspend, sizeof(int))) {
		DISPERR("[FB]: copy_to_user failed! line:%d\n", __LINE__);
		ret = -EFAULT;
	}

	return ret;
}

int _ioctl_get_display_caps(unsigned long arg)
{
	int ret = 0;
	struct disp_caps_info caps_info;
	void __user *argp = (void __user *)arg;

	if (copy_from_user(&caps_info, argp, sizeof(caps_info))) {
		DISPERR("[FB]: copy_to_user failed! line:%d\n", __LINE__);
		ret = -EFAULT;
	}
#ifdef DISP_HW_MODE_CAP
	caps_info.output_mode = DISP_HW_MODE_CAP;
#else
	caps_info.output_mode = DISP_OUTPUT_CAP_DIRECT_LINK;
#endif

#ifdef DISP_HW_PASS_MODE
	caps_info.output_pass = DISP_HW_PASS_MODE;
#else
	caps_info.output_pass = DISP_OUTPUT_CAP_SINGLE_PASS;
#endif

#ifdef DISP_HW_MAX_LAYER
	caps_info.max_layer_num = DISP_HW_MAX_LAYER;
#else
	caps_info.max_layer_num = 4;
#endif

#ifdef CONFIG_MTK_LCM_PHYSICAL_ROTATION_HW
	caps_info.is_output_rotated = 1;
	caps_info.lcm_degree = 180;
#endif

	caps_info.disp_feature |= DISP_FEATURE_NO_PARGB;

	DISPMSG("%s mode:%d, pass:%d, max_layer_num:%d\n",
		__func__, caps_info.output_mode, caps_info.output_pass,
		caps_info.max_layer_num);

	if (copy_to_user(argp, &caps_info, sizeof(caps_info))) {
		DISPERR("[FB]: copy_to_user failed! line:%d\n", __LINE__);
		ret = -EFAULT;
	}

	return ret;
}

int _ioctl_wait_vsync(unsigned long arg)
{
	int ret = 0;
	void __user *argp = (void __user *)arg;
	struct disp_session_vsync_config vsync_config;
	struct disp_session_sync_info *session_info;

	if (copy_from_user(&vsync_config, argp, sizeof(vsync_config))) {
		DISPERR("[FB]: copy_from_user failed! line:%d\n", __LINE__);
		return -EFAULT;
	}

	session_info = disp_get_session_sync_info_for_debug(vsync_config.session_id);
	if (session_info)
		dprec_start(&session_info->event_waitvsync, 0, 0);

	ret = primary_display_wait_for_vsync(&vsync_config);
	if (ret != 0)
		DISPERR("primary_display_wait_for_vsync fail, ret=%d.\n", ret);

	if (session_info)
		dprec_done(&session_info->event_waitvsync, 0, 0);

	if (copy_to_user(argp, &vsync_config, sizeof(vsync_config)))
		DISPERR("[FB]: copy_to_user failed!line:%d\n", __LINE__);
	return ret;
}

int _ioctl_set_vsync(unsigned long arg)
{
	int ret = 0;
	void __user *argp = (void __user *)arg;
	unsigned int fps = 0;

	if (copy_from_user(&fps, argp, sizeof(unsigned int))) {
		DISPERR("[FB]: copy_from_user failed! line:%d\n", __LINE__);
		return -EFAULT;
	}

	ret = primary_display_force_set_vsync_fps(fps);

	return ret;
}


int _ioctl_screen_freeze(unsigned long arg)
{
	int ret = 0;
	void __user *argp = (void __user *)arg;
	unsigned int enable;

	if (copy_from_user(&enable, argp, sizeof(unsigned int))) {
		DISPMSG("[FB]: copy_from_user failed! line:%d\n", __LINE__);
		return -EFAULT;
	}
	ret = display_freeze_mode(enable, 1);

	return ret;
}

int set_session_mode(struct disp_session_config *config_info, int force)
{
	int ret = 0;

	if (DISP_SESSION_TYPE(config_info->session_id) == DISP_SESSION_PRIMARY) {
		primary_display_switch_mode(config_info->mode,
					    config_info->session_id, 0);
	} else {
		DISPERR("[FB]: session(0x%x) swith mode(%d) fail\n",
			config_info->session_id, config_info->mode);
	}

	return ret;
}

int _ioctl_set_session_mode(unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	struct disp_session_config config_info;

	if (copy_from_user(&config_info, argp, sizeof(struct disp_session_config))) {
		DISPERR("[FB]: copy_from_user failed! line:%d\n", __LINE__);
		return -EFAULT;
	}
	return set_session_mode(&config_info, 0);
}

const char *_session_ioctl_spy(unsigned int cmd)
{
	switch (cmd) {
	case DISP_IOCTL_CREATE_SESSION:
		return "DISP_IOCTL_CREATE_SESSION";
	case DISP_IOCTL_DESTROY_SESSION:
		return "DISP_IOCTL_DESTROY_SESSION";
	case DISP_IOCTL_TRIGGER_SESSION:
		return "DISP_IOCTL_TRIGGER_SESSION";
	case DISP_IOCTL_SET_INPUT_BUFFER:
		return "DISP_IOCTL_SET_INPUT_BUFFER";
	case DISP_IOCTL_PREPARE_INPUT_BUFFER:
		return "DISP_IOCTL_PREPARE_INPUT_BUFFER";
	case DISP_IOCTL_WAIT_FOR_VSYNC:
		return "DISP_IOCL_WAIT_FOR_VSYNC";
	case DISP_IOCTL_GET_SESSION_INFO:
		return "DISP_IOCTL_GET_SESSION_INFO";
	case DISP_IOCTL_AAL_EVENTCTL:
		return "DISP_IOCTL_AAL_EVENTCTL";
	case DISP_IOCTL_AAL_GET_HIST:
		return "DISP_IOCTL_AAL_GET_HIST";
	case DISP_IOCTL_AAL_INIT_REG:
		return "DISP_IOCTL_AAL_INIT_REG";
	case DISP_IOCTL_AAL_SET_PARAM:
		return "DISP_IOCTL_AAL_SET_PARAM";
	case DISP_IOCTL_SET_GAMMALUT:
		return "DISP_IOCTL_SET_GAMMALUT";
	case DISP_IOCTL_SET_CCORR:
		return "DISP_IOCTL_SET_CCORR";
	case DISP_IOCTL_SET_PQPARAM:
		return "DISP_IOCTL_SET_PQPARAM";
	case DISP_IOCTL_GET_PQPARAM:
		return "DISP_IOCTL_GET_PQPARAM";
	case DISP_IOCTL_GET_PQINDEX:
		return "DISP_IOCTL_GET_PQINDEX";
	case DISP_IOCTL_SET_PQINDEX:
		return "DISP_IOCTL_SET_PQINDEX";
	case DISP_IOCTL_SET_TDSHPINDEX:
		return "DISP_IOCTL_SET_TDSHPINDEX";
	case DISP_IOCTL_GET_TDSHPINDEX:
		return "DISP_IOCTL_GET_TDSHPINDEX";
	case DISP_IOCTL_SET_PQ_CAM_PARAM:
		return "DISP_IOCTL_SET_PQ_CAM_PARAM";
	case DISP_IOCTL_GET_PQ_CAM_PARAM:
		return "DISP_IOCTL_GET_PQ_CAM_PARAM";
	case DISP_IOCTL_SET_PQ_GAL_PARAM:
		return "DISP_IOCTL_SET_PQ_GAL_PARAM";
	case DISP_IOCTL_GET_PQ_GAL_PARAM:
		return "DISP_IOCTL_GET_PQ_GAL_PARAM";
	case DISP_IOCTL_OD_CTL:
		return "DISP_IOCTL_OD_CTL";
	case DISP_IOCTL_GET_DISPLAY_CAPS:
		return "DISP_IOCTL_GET_DISPLAY_CAPS";
	default:
		return "unknown";
	}
}

long mtk_disp_mgr_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = -1;

	DISPDBG("mtk_disp_mgr_ioctl, cmd=%s, arg=0x%lx\n", _session_ioctl_spy(cmd), arg);

	switch (cmd) {
	case DISP_IOCTL_CREATE_SESSION:
		{
			return _ioctl_create_session(arg);
		}
	case DISP_IOCTL_DESTROY_SESSION:
		{
			return _ioctl_destroy_session(arg);
		}
	case DISP_IOCTL_TRIGGER_SESSION:
		{
			return _ioctl_trigger_session(arg);
		}
	case DISP_IOCTL_GET_PRESENT_FENCE:
		{
			return _ioctl_prepare_present_fence(arg);
		}
	case DISP_IOCTL_PREPARE_INPUT_BUFFER:
		{
			return _ioctl_prepare_buffer(arg, PREPARE_INPUT_FENCE);
		}
	case DISP_IOCTL_SET_INPUT_BUFFER:
		{
			return _ioctl_set_input_buffer(arg);
		}
	case DISP_IOCTL_WAIT_FOR_VSYNC:
		{
			return _ioctl_wait_vsync(arg);
		}
	case DISP_IOCTL_GET_SESSION_INFO:
		{
			return _ioctl_get_info(arg);
		}
	case DISP_IOCTL_GET_DISPLAY_CAPS:
		{
			return _ioctl_get_display_caps(arg);
		}
	case DISP_IOCTL_SET_VSYNC_FPS:
		{
			return _ioctl_set_vsync(arg);
		}
	case DISP_IOCTL_SET_SESSION_MODE:
		{
			return _ioctl_set_session_mode(arg);
		}
	case DISP_IOCTL_PREPARE_OUTPUT_BUFFER:
		{
			return _ioctl_prepare_buffer(arg, PREPARE_OUTPUT_FENCE);
		}
	case DISP_IOCTL_SET_OUTPUT_BUFFER:
		{
			return _ioctl_set_output_buffer(arg);
		}
	case DISP_IOCTL_GET_LCMINDEX:
		{
			return _ioctl_get_lcm_index(arg);
		}
	case DISP_IOCTL_SCREEN_FREEZE:
		{
			return _ioctl_screen_freeze(arg);
		}
	case DISP_IOCTL_AAL_EVENTCTL:
	case DISP_IOCTL_AAL_GET_HIST:
	case DISP_IOCTL_AAL_INIT_REG:
	case DISP_IOCTL_AAL_SET_PARAM:
	case DISP_IOCTL_SET_GAMMALUT:
	case DISP_IOCTL_SET_CCORR:
	case DISP_IOCTL_SET_PQPARAM:
	case DISP_IOCTL_GET_PQPARAM:
	case DISP_IOCTL_SET_PQINDEX:
	case DISP_IOCTL_GET_PQINDEX:
	case DISP_IOCTL_SET_TDSHPINDEX:
	case DISP_IOCTL_GET_TDSHPINDEX:
	case DISP_IOCTL_SET_PQ_CAM_PARAM:
	case DISP_IOCTL_GET_PQ_CAM_PARAM:
	case DISP_IOCTL_SET_PQ_GAL_PARAM:
	case DISP_IOCTL_GET_PQ_GAL_PARAM:
	case DISP_IOCTL_PQ_SET_BYPASS_COLOR:
	case DISP_IOCTL_PQ_SET_WINDOW:
	case DISP_IOCTL_OD_CTL:
	case DISP_IOCTL_WRITE_REG:
	case DISP_IOCTL_READ_REG:
	case DISP_IOCTL_MUTEX_CONTROL:
	case DISP_IOCTL_PQ_GET_TDSHP_FLAG:
	case DISP_IOCTL_PQ_SET_TDSHP_FLAG:
	case DISP_IOCTL_PQ_GET_DC_PARAM:
	case DISP_IOCTL_PQ_SET_DC_PARAM:
	case DISP_IOCTL_PQ_GET_DS_PARAM:
	case DISP_IOCTL_PQ_GET_MDP_COLOR_CAP:
	case DISP_IOCTL_PQ_GET_MDP_TDSHP_REG:
	case DISP_IOCTL_WRITE_SW_REG:
	case DISP_IOCTL_READ_SW_REG:
		{
			ret = primary_display_user_cmd(cmd, arg);
			break;
		}
	default:
		{
			DISPERR("[session]ioctl not supported, cmd nr=0x%08x, cmd size=0x%08x\n",
				(unsigned int)_IOC_NR(cmd),
				(unsigned int)_IOC_SIZE(cmd));
		}
	}

	return ret;
}

#ifdef CONFIG_COMPAT
static long mtk_disp_mgr_compat_ioctl(struct file *file, unsigned int cmd,
				      unsigned long arg)
{
	long ret = -ENOIOCTLCMD;

	switch (cmd) {

		/* add cases here for 32bit/64bit conversion */
		/* ... */

	default:
		return mtk_disp_mgr_ioctl(file, cmd, arg);
	}

	return ret;
}
#endif


static const struct file_operations mtk_disp_mgr_fops = {
	.owner = THIS_MODULE,
	.mmap = mtk_disp_mgr_mmap,
	.unlocked_ioctl = mtk_disp_mgr_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = mtk_disp_mgr_compat_ioctl,
#endif
	.open = mtk_disp_mgr_open,
	.release = mtk_disp_mgr_release,
	.flush = mtk_disp_mgr_flush,
	.read = mtk_disp_mgr_read,
};

static int mtk_disp_mgr_probe(struct platform_device *pdev)
{
	struct class_device;
	struct class_device *class_dev = NULL;
	int ret = 0;

	DISPMSG("mtk_disp_mgr_probe called!\n");

	if (alloc_chrdev_region(&mtk_disp_mgr_devno, 0, 1, DISP_SESSION_DEVICE))
		return -EFAULT;

	mtk_disp_mgr_cdev = cdev_alloc();
	mtk_disp_mgr_cdev->owner = THIS_MODULE;
	mtk_disp_mgr_cdev->ops = &mtk_disp_mgr_fops;

	ret = cdev_add(mtk_disp_mgr_cdev, mtk_disp_mgr_devno, 1);
	if (ret) {
		DISPMSG("mtk_disp_mgr_probe was failed, ruturn %d\n", ret);
		return ret;
	}
	mtk_disp_mgr_class = class_create(THIS_MODULE, DISP_SESSION_DEVICE);
	class_dev =
	    (struct class_device *)device_create(mtk_disp_mgr_class, NULL,
						 mtk_disp_mgr_devno, NULL,
						 DISP_SESSION_DEVICE);
	disp_sync_init();

	return 0;
}

static int mtk_disp_mgr_remove(struct platform_device *pdev)
{
	return 0;
}

static void mtk_disp_mgr_shutdown(struct platform_device *pdev)
{
}

static int mtk_disp_mgr_suspend(struct platform_device *pdev, pm_message_t mesg)
{
	return 0;
}

static int mtk_disp_mgr_resume(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver mtk_disp_mgr_driver = {
	.probe = mtk_disp_mgr_probe,
	.remove = mtk_disp_mgr_remove,
	.shutdown = mtk_disp_mgr_shutdown,
	.suspend = mtk_disp_mgr_suspend,
	.resume = mtk_disp_mgr_resume,
	.driver = {
		   .name = DISP_SESSION_DEVICE,
		   },
};

static void mtk_disp_mgr_device_release(struct device *dev)
{

}

static u64 mtk_disp_mgr_dmamask = ~(u32) 0;

static struct platform_device mtk_disp_mgr_device = {
	.name = DISP_SESSION_DEVICE,
	.id = 0,
	.dev = {
		.release = mtk_disp_mgr_device_release,
		.dma_mask = &mtk_disp_mgr_dmamask,
		.coherent_dma_mask = 0xffffffff,
		},
	.num_resources = 0,
};

static int __init mtk_disp_mgr_init(void)
{
	if (platform_device_register(&mtk_disp_mgr_device))
		return -ENODEV;

	if (platform_driver_register(&mtk_disp_mgr_driver)) {
		platform_device_unregister(&mtk_disp_mgr_device);
		return -ENODEV;
	}

	return 0;
}

static void __exit mtk_disp_mgr_exit(void)
{
	cdev_del(mtk_disp_mgr_cdev);
	unregister_chrdev_region(mtk_disp_mgr_devno, 1);

	platform_driver_unregister(&mtk_disp_mgr_driver);
	platform_device_unregister(&mtk_disp_mgr_device);

	device_destroy(mtk_disp_mgr_class, mtk_disp_mgr_devno);
	class_destroy(mtk_disp_mgr_class);
}

module_init(mtk_disp_mgr_init);
module_exit(mtk_disp_mgr_exit);

MODULE_DESCRIPTION("MediaTek Display Manager");
