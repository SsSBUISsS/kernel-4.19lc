////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2014 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (??MStar Confidential Information??) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

/**
 *
 * @file    mstar_drv_platform_interface.c
 *
 * @brief   This file defines the interface of touch screen
 *
 *
 */

/*=============================================================*/
// INCLUDE FILE
/*=============================================================*/

#include "mstar_drv_platform_interface.h"
#include "mstar_drv_main.h"
#include "mstar_drv_ic_fw_porting_layer.h"
#include "mstar_drv_platform_porting_layer.h"
#include "mstar_drv_utility_adaption.h"

#ifdef CONFIG_ENABLE_HOTKNOT
#include "mstar_drv_hotknot.h"
#endif //CONFIG_ENABLE_HOTKNOT

/*=============================================================*/
// EXTERN VARIABLE DECLARATION
/*=============================================================*/

#ifdef CONFIG_ENABLE_GESTURE_WAKEUP
extern U32 g_GestureWakeupMode[2];
extern u8 g_GestureWakeupFlag;

#ifdef CONFIG_ENABLE_GESTURE_DEBUG_MODE
extern u8 g_GestureDebugFlag;
extern u8 g_GestureDebugMode;
#endif //CONFIG_ENABLE_GESTURE_DEBUG_MODE

#endif //CONFIG_ENABLE_GESTURE_WAKEUP

#ifdef CONFIG_ENABLE_PROXIMITY_DETECTION
extern u8 g_EnableTpProximity;
#endif //CONFIG_ENABLE_PROXIMITY_DETECTION

#ifdef CONFIG_ENABLE_GLOVE_MODE
extern u8 g_IsEnableGloveMode;
#endif //CONFIG_ENABLE_GLOVE_MODE

extern u8 g_IsUpdateFirmware;

extern struct input_dev *g_InputDevice;
extern struct i2c_client *g_I2cClient;

#ifdef CONFIG_ENABLE_HOTKNOT
extern u8 g_HotKnotState;
extern U32 SLAVE_I2C_ID_DWI2C;
#endif //CONFIG_ENABLE_HOTKNOT

#ifdef CONFIG_ENABLE_CHARGER_DETECTION
extern u8 g_ForceUpdate;
#endif //CONFIG_ENABLE_CHARGER_DETECTION

extern u8 IS_FIRMWARE_DATA_LOG_ENABLED;


/*=============================================================*/
// GLOBAL VARIABLE DEFINITION
/*=============================================================*/


/*=============================================================*/
// LOCAL VARIABLE DEFINITION
/*=============================================================*/

#ifdef CONFIG_ENABLE_HOTKNOT
static u8 _gAMStartCmd[4] = {HOTKNOT_SEND, ADAPTIVEMOD_BEGIN, 0, 0};
#endif //CONFIG_ENABLE_HOTKNOT

/*=============================================================*/
// GLOBAL FUNCTION DEFINITION
/*=============================================================*/

#ifdef CONFIG_ENABLE_NOTIFIER_FB
int MsDrvInterfaceTouchDeviceFbNotifierCallback(struct notifier_block *pSelf, unsigned long nEvent, void *pData)
{
    struct fb_event *pEventData = pData;
    int *pBlank;

    if (pEventData && pEventData->data && nEvent == FB_EVENT_BLANK)
    {
        pBlank = pEventData->data;

        if (*pBlank == FB_BLANK_UNBLANK)
        {
            DBG(&g_I2cClient->dev, "*** %s() TP Resume ***\n", __func__);

            if (g_IsUpdateFirmware != 0) // Check whether update frimware is finished
            {
                DBG(&g_I2cClient->dev, "Not allow to power on/off touch ic while update firmware.\n");
                return 0;
            }

#ifdef CONFIG_ENABLE_PROXIMITY_DETECTION
            if (g_EnableTpProximity == 1)
            {
                DBG(&g_I2cClient->dev, "g_EnableTpProximity = %d\n", g_EnableTpProximity);
                return 0;
            }
#endif //CONFIG_ENABLE_PROXIMITY_DETECTION
            
#ifdef CONFIG_ENABLE_GESTURE_WAKEUP
#ifdef CONFIG_ENABLE_HOTKNOT
            if (g_HotKnotState != HOTKNOT_BEFORE_TRANS_STATE && g_HotKnotState != HOTKNOT_TRANS_STATE && g_HotKnotState != HOTKNOT_AFTER_TRANS_STATE)
#endif //CONFIG_ENABLE_HOTKNOT
            {
#ifdef CONFIG_ENABLE_GESTURE_DEBUG_MODE
                if (g_GestureDebugMode == 1)
                {
                    DrvIcFwLyrCloseGestureDebugMode();
                }
#endif //CONFIG_ENABLE_GESTURE_DEBUG_MODE

                if (g_GestureWakeupFlag == 1)
                {
                    DrvIcFwLyrCloseGestureWakeup();
                }
                else
                {
                    DrvPlatformLyrEnableFingerTouchReport(); 
                }
            }
#ifdef CONFIG_ENABLE_HOTKNOT
            else    // Enable touch in hotknot transfer mode
            {
                DrvPlatformLyrEnableFingerTouchReport();     
            }
#endif //CONFIG_ENABLE_HOTKNOT
#endif //CONFIG_ENABLE_GESTURE_WAKEUP
    
#ifdef CONFIG_ENABLE_HOTKNOT
            if (g_HotKnotState != HOTKNOT_BEFORE_TRANS_STATE && g_HotKnotState != HOTKNOT_TRANS_STATE && g_HotKnotState != HOTKNOT_AFTER_TRANS_STATE)
#endif //CONFIG_ENABLE_HOTKNOT        
            {
#ifdef CONFIG_ENABLE_REGULATOR_POWER_ON
            	DrvPlatformLyrTouchDeviceRegulatorPowerOn(true);
#endif //CONFIG_ENABLE_REGULATOR_POWER_ON               
                DrvPlatformLyrTouchDevicePowerOn(); 
            }   
    
#ifdef CONFIG_ENABLE_CHARGER_DETECTION 
            {
                u8 szChargerStatus[20] = {0};
     
                DrvCommonReadFile("/sys/class/power_supply/battery/status", szChargerStatus, 20);
            
                DBG(&g_I2cClient->dev, "*** Battery Status : %s ***\n", szChargerStatus);
            
                g_ForceUpdate = 1; // Set flag to force update charger status
                
                if (strstr(szChargerStatus, "Charging") != NULL || strstr(szChargerStatus, "Full") != NULL || strstr(szChargerStatus, "Fully charged") != NULL) // Charging
                {
                    DrvFwCtrlChargerDetection(1); // charger plug-in
                }
                else // Not charging
                {
                    DrvFwCtrlChargerDetection(0); // charger plug-out
                }

                g_ForceUpdate = 0; // Clear flag after force update charger status
            }           
#endif //CONFIG_ENABLE_CHARGER_DETECTION

#ifdef CONFIG_ENABLE_GLOVE_MODE
            if (g_IsEnableGloveMode == 1)
            {
                DrvIcFwLyrOpenGloveMode();
            }
#endif //CONFIG_ENABLE_GLOVE_MODE

            if (IS_FIRMWARE_DATA_LOG_ENABLED)    
            {
                DrvIcFwLyrRestoreFirmwareModeToLogDataMode(); // Mark this function call for avoiding device driver may spend longer time to resume from suspend state.
            } //IS_FIRMWARE_DATA_LOG_ENABLED

#ifndef CONFIG_ENABLE_GESTURE_WAKEUP
            DrvPlatformLyrEnableFingerTouchReport(); 
#endif //CONFIG_ENABLE_GESTURE_WAKEUP
        }
        else if (*pBlank == FB_BLANK_POWERDOWN)
        {
            DBG(&g_I2cClient->dev, "*** %s() TP Suspend ***\n", __func__);
            
            if (g_IsUpdateFirmware != 0) // Check whether update frimware is finished
            {
                DBG(&g_I2cClient->dev, "Not allow to power on/off touch ic while update firmware.\n");
                return 0;
            }

#ifdef CONFIG_ENABLE_PROXIMITY_DETECTION
            if (g_EnableTpProximity == 1)
            {
                DBG(&g_I2cClient->dev, "g_EnableTpProximity = %d\n", g_EnableTpProximity);
                return 0;
            }
#endif //CONFIG_ENABLE_PROXIMITY_DETECTION

#ifdef CONFIG_ENABLE_GESTURE_WAKEUP
#ifdef CONFIG_ENABLE_HOTKNOT
            if (g_HotKnotState != HOTKNOT_BEFORE_TRANS_STATE && g_HotKnotState != HOTKNOT_TRANS_STATE && g_HotKnotState != HOTKNOT_AFTER_TRANS_STATE)
#endif //CONFIG_ENABLE_HOTKNOT
            {
                if (g_GestureWakeupMode[0] != 0x00000000 || g_GestureWakeupMode[1] != 0x00000000)
                {
                    DrvIcFwLyrOpenGestureWakeup(&g_GestureWakeupMode[0]);
                    return 0;
                }
            }
#endif //CONFIG_ENABLE_GESTURE_WAKEUP

#ifdef CONFIG_ENABLE_HOTKNOT
            if (g_HotKnotState == HOTKNOT_BEFORE_TRANS_STATE || g_HotKnotState == HOTKNOT_TRANS_STATE || g_HotKnotState == HOTKNOT_AFTER_TRANS_STATE)
            {
                IicWriteData(SLAVE_I2C_ID_DWI2C, &_gAMStartCmd[0], 4); 
            }
#endif //CONFIG_ENABLE_HOTKNOT 

            DrvPlatformLyrFingerTouchReleased(0, 0, 0); // Send touch end for clearing point touch
            input_sync(g_InputDevice);

            DrvPlatformLyrDisableFingerTouchReport();

#ifdef CONFIG_ENABLE_HOTKNOT
            if (g_HotKnotState != HOTKNOT_BEFORE_TRANS_STATE && g_HotKnotState != HOTKNOT_TRANS_STATE && g_HotKnotState != HOTKNOT_AFTER_TRANS_STATE)
#endif //CONFIG_ENABLE_HOTKNOT        
            {
                DrvPlatformLyrTouchDevicePowerOff(); 
#ifdef CONFIG_ENABLE_REGULATOR_POWER_ON
				DrvPlatformLyrTouchDeviceRegulatorPowerOn(false);
#endif //CONFIG_ENABLE_REGULATOR_POWER_ON
            }    
        }
    }

    return 0;
}

#else

#ifdef CONFIG_PLATFORM_USE_ANDROID_SDK_6_UPWARD
void MsDrvInterfaceTouchDeviceSuspend(struct device *pDevice)
#else
void MsDrvInterfaceTouchDeviceSuspend(struct early_suspend *pSuspend)
#endif //CONFIG_PLATFORM_USE_ANDROID_SDK_6_UPWARD
{
    DBG(&g_I2cClient->dev, "*** %s() ***\n", __func__);

    if (g_IsUpdateFirmware != 0) // Check whether update frimware is finished
    {
        DBG(&g_I2cClient->dev, "Not allow to power on/off touch ic while update firmware.\n");
        return;
    }

#ifdef CONFIG_ENABLE_PROXIMITY_DETECTION
    if (g_EnableTpProximity == 1)
    {
        DBG(&g_I2cClient->dev, "g_EnableTpProximity = %d\n", g_EnableTpProximity);
        return;
    }
#endif //CONFIG_ENABLE_PROXIMITY_DETECTION

#ifdef CONFIG_ENABLE_GESTURE_WAKEUP
#ifdef CONFIG_ENABLE_HOTKNOT
    if (g_HotKnotState != HOTKNOT_BEFORE_TRANS_STATE && g_HotKnotState != HOTKNOT_TRANS_STATE && g_HotKnotState != HOTKNOT_AFTER_TRANS_STATE)
#endif //CONFIG_ENABLE_HOTKNOT
    {
        if (g_GestureWakeupMode[0] != 0x00000000 || g_GestureWakeupMode[1] != 0x00000000)
        {
            DrvIcFwLyrOpenGestureWakeup(&g_GestureWakeupMode[0]);
            return;
        }
    }
#endif //CONFIG_ENABLE_GESTURE_WAKEUP

#ifdef CONFIG_ENABLE_HOTKNOT
    if (g_HotKnotState == HOTKNOT_BEFORE_TRANS_STATE || g_HotKnotState == HOTKNOT_TRANS_STATE || g_HotKnotState == HOTKNOT_AFTER_TRANS_STATE)
    {
        IicWriteData(SLAVE_I2C_ID_DWI2C, &_gAMStartCmd[0], 4); 
    }
#endif //CONFIG_ENABLE_HOTKNOT  

    DrvPlatformLyrFingerTouchReleased(0, 0, 0); // Send touch end for clearing point touch
    input_sync(g_InputDevice);

    DrvPlatformLyrDisableFingerTouchReport();

#ifdef CONFIG_ENABLE_HOTKNOT
    if (g_HotKnotState != HOTKNOT_BEFORE_TRANS_STATE && g_HotKnotState != HOTKNOT_TRANS_STATE && g_HotKnotState != HOTKNOT_AFTER_TRANS_STATE)
#endif //CONFIG_ENABLE_HOTKNOT        
    {
        DrvPlatformLyrTouchDevicePowerOff(); 
#ifdef CONFIG_PLATFORM_USE_ANDROID_SDK_6_UPWARD
#ifdef CONFIG_ENABLE_REGULATOR_POWER_ON
        DrvPlatformLyrTouchDeviceRegulatorPowerOn(false);
#endif //CONFIG_ENABLE_REGULATOR_POWER_ON               
#endif //CONFIG_PLATFORM_USE_ANDROID_SDK_6_UPWARD
    }    
}

#ifdef CONFIG_PLATFORM_USE_ANDROID_SDK_6_UPWARD
void MsDrvInterfaceTouchDeviceResume(struct device *pDevice)
#else
void MsDrvInterfaceTouchDeviceResume(struct early_suspend *pSuspend)
#endif //CONFIG_PLATFORM_USE_ANDROID_SDK_6_UPWARD
{
    DBG(&g_I2cClient->dev, "*** %s() ***\n", __func__);

    if (g_IsUpdateFirmware != 0) // Check whether update frimware is finished
    {
        DBG(&g_I2cClient->dev, "Not allow to power on/off touch ic while update firmware.\n");
        return;
    }

#ifdef CONFIG_ENABLE_PROXIMITY_DETECTION
    if (g_EnableTpProximity == 1)
    {
        DBG(&g_I2cClient->dev, "g_EnableTpProximity = %d\n", g_EnableTpProximity);
        return;
    }
#endif //CONFIG_ENABLE_PROXIMITY_DETECTION

#ifdef CONFIG_ENABLE_GESTURE_WAKEUP
#ifdef CONFIG_ENABLE_HOTKNOT
    if (g_HotKnotState != HOTKNOT_BEFORE_TRANS_STATE && g_HotKnotState != HOTKNOT_TRANS_STATE && g_HotKnotState != HOTKNOT_AFTER_TRANS_STATE)
#endif //CONFIG_ENABLE_HOTKNOT
    {
#ifdef CONFIG_ENABLE_GESTURE_DEBUG_MODE
        if (g_GestureDebugMode == 1)
        {
            DrvIcFwLyrCloseGestureDebugMode();
        }
#endif //CONFIG_ENABLE_GESTURE_DEBUG_MODE

        if (g_GestureWakeupFlag == 1)
        {
            DrvIcFwLyrCloseGestureWakeup();
        }
        else
        {
            DrvPlatformLyrEnableFingerTouchReport(); 
        }
    }
#ifdef CONFIG_ENABLE_HOTKNOT
    else    // Enable touch in hotknot transfer mode
    {
        DrvPlatformLyrEnableFingerTouchReport();     
    }
#endif //CONFIG_ENABLE_HOTKNOT
#endif //CONFIG_ENABLE_GESTURE_WAKEUP
    
#ifdef CONFIG_ENABLE_HOTKNOT
    if (g_HotKnotState != HOTKNOT_BEFORE_TRANS_STATE && g_HotKnotState != HOTKNOT_TRANS_STATE && g_HotKnotState != HOTKNOT_AFTER_TRANS_STATE)
#endif //CONFIG_ENABLE_HOTKNOT        
    {
#ifdef CONFIG_PLATFORM_USE_ANDROID_SDK_6_UPWARD
#ifdef CONFIG_ENABLE_REGULATOR_POWER_ON
        DrvPlatformLyrTouchDeviceRegulatorPowerOn(true);
#endif //CONFIG_ENABLE_REGULATOR_POWER_ON               
#endif //CONFIG_PLATFORM_USE_ANDROID_SDK_6_UPWARD
        DrvPlatformLyrTouchDevicePowerOn(); 
    }   
    
#ifdef CONFIG_ENABLE_CHARGER_DETECTION 
    {
        u8 szChargerStatus[20] = {0};
 
        DrvCommonReadFile("/sys/class/power_supply/battery/status", szChargerStatus, 20);
        
        DBG(&g_I2cClient->dev, "*** Battery Status : %s ***\n", szChargerStatus);
        
        g_ForceUpdate = 1; // Set flag to force update charger status

        if (strstr(szChargerStatus, "Charging") != NULL || strstr(szChargerStatus, "Full") != NULL || strstr(szChargerStatus, "Fully charged") != NULL) // Charging
        {
            DrvFwCtrlChargerDetection(1); // charger plug-in
        }
        else // Not charging
        {
            DrvFwCtrlChargerDetection(0); // charger plug-out
        }

        g_ForceUpdate = 0; // Clear flag after force update charger status
    }           
#endif //CONFIG_ENABLE_CHARGER_DETECTION

#ifdef CONFIG_ENABLE_GLOVE_MODE
    if (g_IsEnableGloveMode == 1)
    {
        DrvIcFwLyrOpenGloveMode();
    }
#endif //CONFIG_ENABLE_GLOVE_MODE

    if (IS_FIRMWARE_DATA_LOG_ENABLED)    
    {
        DrvIcFwLyrRestoreFirmwareModeToLogDataMode(); // Mark this function call for avoiding device driver may spend longer time to resume from suspend state.
    } //IS_FIRMWARE_DATA_LOG_ENABLED

#ifndef CONFIG_ENABLE_GESTURE_WAKEUP
    DrvPlatformLyrEnableFingerTouchReport(); 
#endif //CONFIG_ENABLE_GESTURE_WAKEUP
}

#endif /* CONFIG_ENABLE_NOTIFIER_FB */

#if 0/* def TOUCH_VIRTUAL_KEYS //my */
static ssize_t virtual_keys_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	struct msg2xxx_ts_platform_data *pdata = g_I2cClient->dev.platform_data;
	return sprintf(buf, "%s:%s:%d:%d:%d:%d:%s:%s:%d:%d:%d:%d:%s:%s:%d:%d:%d:%d\n"
		, __stringify(EV_KEY), __stringify(KEY_MENU), pdata->virtualkeys[0], pdata->virtualkeys[1], pdata->virtualkeys[2], pdata->virtualkeys[3]
		, __stringify(EV_KEY), __stringify(KEY_HOMEPAGE), pdata->virtualkeys[4], pdata->virtualkeys[5], pdata->virtualkeys[6], pdata->virtualkeys[7]
		, __stringify(EV_KEY), __stringify(KEY_BACK), pdata->virtualkeys[8], pdata->virtualkeys[9], pdata->virtualkeys[10], pdata->virtualkeys[11]);
}

static struct kobj_attribute virtual_keys_attr = {
	.attr = {
		.name = "virtualkeys.msg2138_ts", /* msg2xxx */
		.mode = S_IRUGO,
	},
	.show = &virtual_keys_show,
};

static struct attribute *properties_attrs[] = {
	&virtual_keys_attr.attr,
	NULL
};

static struct attribute_group properties_attr_group = {
	.attrs = properties_attrs,
};

static void pixcir_ts_virtual_keys_init(void)
{
	int ret;
	struct kobject *properties_kobj;

	properties_kobj = kobject_create_and_add("board_properties", NULL);
	if (properties_kobj)
		ret = sysfs_create_group(properties_kobj,
					 &properties_attr_group);
	if (!properties_kobj || ret)
		pr_err("failed to create board_properties\n");
}
#endif

#ifdef CONFIG_SPRD_PH_INFO
extern char SPRD_TPInfo[];
#endif
/* probe function is used for matching and initializing input device */
s32 /*__devinit*/ MsDrvInterfaceTouchDeviceProbe(struct i2c_client *pClient, const struct i2c_device_id *pDeviceId)
{
    s32 nRetVal = 0;

    DBG(&g_I2cClient->dev, "*** %s() ***\n", __func__);
  
    DrvPlatformLyrInputDeviceInitialize(pClient);
  
    DrvPlatformLyrTouchDeviceRequestGPIO(pClient);

#ifdef CONFIG_ENABLE_REGULATOR_POWER_ON
    DrvPlatformLyrTouchDeviceRegulatorPowerOn(true);
#endif //CONFIG_ENABLE_REGULATOR_POWER_ON

    DrvPlatformLyrTouchDevicePowerOn();

#ifdef CONFIG_TOUCH_DRIVER_RUN_ON_MTK_PLATFORM
#ifdef CONFIG_ENABLE_DMA_IIC
    DmaAlloc();
#endif //CONFIG_ENABLE_DMA_IIC
#endif //CONFIG_TOUCH_DRIVER_RUN_ON_MTK_PLATFORM

    nRetVal = DrvMainTouchDeviceInitialize();
    if (nRetVal == -ENODEV)
    {
        DrvPlatformLyrTouchDeviceRemove(pClient);
        return nRetVal;
    }

    DrvPlatformLyrTouchDeviceRegisterFingerTouchInterruptHandler();

    DrvPlatformLyrTouchDeviceRegisterEarlySuspend();

#ifdef CONFIG_UPDATE_FIRMWARE_BY_SW_ID
    DrvIcFwLyrCheckFirmwareUpdateBySwId();
#endif //CONFIG_UPDATE_FIRMWARE_BY_SW_ID

    DBG(&g_I2cClient->dev, "*** MStar touch driver registered ***\n");
    
    return nRetVal;
}

/* remove function is triggered when the input device is removed from input sub-system */
s32 /*__devexit*/ MsDrvInterfaceTouchDeviceRemove(struct i2c_client *pClient)
{
    DBG(&g_I2cClient->dev, "*** %s() ***\n", __func__);

    return DrvPlatformLyrTouchDeviceRemove(pClient);
}

void MsDrvInterfaceTouchDeviceSetIicDataRate(struct i2c_client *pClient, U32 nIicDataRate)
{
    DBG(&g_I2cClient->dev, "*** %s() ***\n", __func__);

    DrvPlatformLyrSetIicDataRate(pClient, nIicDataRate);
}    
