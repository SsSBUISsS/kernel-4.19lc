/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 MediaTek Inc.
 */

/* -------------------------------------------- */
#ifndef CAMERA_PIPE_MGR_IMP_H
#define CAMERA_PIPE_MGR_IMP_H
/* -------------------------------------------- */
#define MTRUE               true
#define MFALSE              false
/* -------------------------------------------- */
#define LOG_TAG "CamPipeMgr"
#define LOG_MSG(fmt, arg...)    pr_debug(LOG_TAG "[%s]" \
fmt "\r\n", __func__,           ##arg)
#define LOG_WRN(fmt, arg...)    pr_warn(LOG_TAG "[%s]WRN(%5d):" \
fmt "\r\n", __func__, __LINE__, ##arg)
#define LOG_ERR(fmt, arg...)    pr_err(LOG_TAG "[%s]ERR(%5d):" \
fmt "\r\n", __func__, __LINE__, ##arg)
#define LOG_DMP(fmt, arg...)    pr_err(LOG_TAG "" fmt, ##arg)
/* -------------------------------------------- */
#define CAM_PIPE_MGR_DEV_NUM        (1)
#define CAM_PIPE_MGR_DEV_MINOR_NUM  (1)
#define CAM_PIPE_MGR_DEV_NO_MINOR   (0)
#define CAM_PIPE_MGR_JIFFIES_MAX    (0xFFFFFFFF)
#define CAM_PIPE_MGR_PROC_NAME      "Default"
#define CAM_PIPE_MGR_SCEN_HW_AMOUNT (7)
#define CAM_PIPE_MGR_TIMEOUT_MAX    (10*1000)
/* -------------------------------------------- */
#define CAM_PIPE_MGR_PIPE_NAME_LEN          (10)
#define CAM_PIPE_MGR_PIPE_NAME_CAM_IO       "CamIO"
#define CAM_PIPE_MGR_PIPE_NAME_POST_PROC    "PostProc"
#define CAM_PIPE_MGR_PIPE_NAME_XDP_CAM      "CamXDP"
/* -------------------------------------------- */
enum CAM_PIPE_MGR_PIPE_ENUM {
	CAM_PIPE_MGR_PIPE_CAM_IO,
	CAM_PIPE_MGR_PIPE_POST_PROC,
	CAM_PIPE_MGR_PIPE_XDP_CAM,
	CAM_PIPE_MGR_PIPE_AMOUNT
};
/*  */
enum CAM_PIPE_MGR_STATUS_ENUM {
	CAM_PIPE_MGR_STATUS_OK,
	CAM_PIPE_MGR_STATUS_FAIL,
	CAM_PIPE_MGR_STATUS_TIMEOUT,
	CAM_PIPE_MGR_STATUS_UNKNOW
};
/*  */
#define CAM_PIPE_MGR_LOCK_TABLE_NONE    (0)
#define CAM_PIPE_MGR_LOCK_TABLE_IC      ((1<<CAM_PIPE_MGR_PIPE_CAM_IO)| \
					    (1<<CAM_PIPE_MGR_PIPE_POST_PROC))
#define CAM_PIPE_MGR_LOCK_TABLE_VR      ((1<<CAM_PIPE_MGR_PIPE_CAM_IO)| \
					    (1<<CAM_PIPE_MGR_PIPE_XDP_CAM))
#define CAM_PIPE_MGR_LOCK_TABLE_ZSD     ((1<<CAM_PIPE_MGR_PIPE_CAM_IO)| \
					    (1<<CAM_PIPE_MGR_PIPE_XDP_CAM))
#define CAM_PIPE_MGR_LOCK_TABLE_IP      ((1<<CAM_PIPE_MGR_PIPE_POST_PROC))
#define CAM_PIPE_MGR_LOCK_TABLE_N3D     ((1<<CAM_PIPE_MGR_PIPE_CAM_IO)| \
					    (1<<CAM_PIPE_MGR_PIPE_POST_PROC))
#define CAM_PIPE_MGR_LOCK_TABLE_VSS     ((1<<CAM_PIPE_MGR_PIPE_CAM_IO)| \
					    (1<<CAM_PIPE_MGR_PIPE_POST_PROC))
/* -------------------------------------------- */
struct CAM_PIPE_MGR_PROC_STRUCT {
	pid_t Pid;
	pid_t Tgid;
	char ProcName[TASK_COMM_LEN];
	unsigned int PipeMask;
	unsigned int TimeS;
	unsigned int TimeUS;
};
/*  */
struct CAM_PIPE_MGR_PIPE_STRUCT {
	pid_t Pid;
	pid_t Tgid;
	char ProcName[TASK_COMM_LEN];
	unsigned int TimeS;
	unsigned int TimeUS;
};
/*  */
struct CAM_PIPE_MGR_STRUCT {
	unsigned long PipeMask;
	spinlock_t SpinLock;
	dev_t DevNo;
	struct cdev *pCharDrv;
	struct class *pClass;
	wait_queue_head_t WaitQueueHead;
	struct CAM_PIPE_MGR_MODE_STRUCT Mode;
	struct CAM_PIPE_MGR_PIPE_STRUCT PipeInfo[CAM_PIPE_MGR_PIPE_AMOUNT];
	char PipeName[CAM_PIPE_MGR_PIPE_AMOUNT][CAM_PIPE_MGR_PIPE_NAME_LEN];
	unsigned long PipeLockTable[CAM_PIPE_MGR_SCEN_HW_AMOUNT];
	unsigned long LogMask;
};
/* -------------------------------------------- */
#endif
/* -------------------------------------------- */
