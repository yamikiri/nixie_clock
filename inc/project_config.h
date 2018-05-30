/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */
#ifndef __PROJECT_CONFIG_H__
#define __PROJECT_CONFIG_H__

#include <time.h>
#include <stdint.h>
#define NTP_SERVER1 "time.stdtime.gov.tw"
#define NTP_SERVER2 "clock.stdtime.gov.tw"
#define TIMEZONE_OFFSET     8
#define CURR_CENTURY 2000
#define US2TICK(us) (us/(1000*portTICK_RATE_MS))
#define MS2TICK(ms) (ms/(portTICK_RATE_MS))

/* max supported connection number */
#define BT_CONNECTION_MAX   16

/* max timer count */
#define BT_TIMER_NUM 10

#define BT_TX_BUF_SIZE 256
#define BT_RX_BUF_SIZE 1024

#define BT_TIMER_BUF_SIZE (BT_TIMER_NUM * BT_CONTROL_BLOCK_SIZE_OF_TIMER)
#define BT_CONNECTION_BUF_SIZE (BT_CONNECTION_MAX* BT_CONTROL_BLOCK_SIZE_OF_LE_CONNECTION)

#define DEVICE_MANUFACTURER           "BiotrumpSystem"
#define DEVICE_MANUFACTURER_LEN       (14)

#define APP_BLE_DEVICE_NAME           "IoT NixieClock"

#define APP_BLE_SERVICE_UUID          (0x1801)

#define VFIFO_SIZE                    (128)
#define SEND_THRESHOLD_SIZE           (8)
#define RECEIVE_THRESHOLD_SIZE        (8)
#define RECEIVE_ALERT_SIZE            (30)
#define CLOCK_CONFIG_VERSION          (4)
#define MAX_AMOUNT_ALARMS             (3)

#define FIX_DIGI_0                    (1 << 0)
#define FIX_DIGI_1                    (1 << 1)
#define FIX_DIGI_2                    (1 << 2)
#define FIX_DIGI_3                    (1 << 3)
#define NIXIE_PCB_FIX                 (FIX_DIGI_0 | FIX_DIGI_1 | FIX_DIGI_2 | FIX_DIGI_3)

typedef struct _task_enables {
    unsigned short broadcast;
    unsigned short wifiReady;
} task_enables;

typedef struct _notification_tasklet {
    unsigned char connected;
    unsigned short conn_handle;
    task_enables enables;
} notification_tasklet;

typedef struct _alarm_config {
    unsigned char AlarmEnable;
    unsigned char AlarmType;
    unsigned char AlarmMusicIndex;
    unsigned char AlarmMusicPlayMode;
    unsigned char AlarmSchedule[7];
    unsigned char AlarmHour;
    unsigned char AlarmMinute;
} alarm_config;

typedef struct _clock_configurations {
    unsigned char magic[2];
    unsigned long version;
    unsigned long length;
    int timeZone;
    char ssid[33];
    char pwd[64];
    unsigned char nixieFix;
    unsigned long nAlarms;
    alarm_config alarms[MAX_AMOUNT_ALARMS];
} clock_configurations;

#define NVDM_GRP "NIXIE_CLOCK"
#define NVDM_GLOBAL_CONFIG "GLOBAL_CONFIGS"
#define NVDM_DEFAULT_SSID "ssid"
#define NVDM_DEFAULT_PWD "password"

extern volatile clock_configurations gConfig;
extern volatile notification_tasklet gNotiTasklet;
extern char volatile gTimeStringCache[20];
extern volatile void* gBLE_BroadcastNotiIndication;
extern volatile void* gBLE_WifiConnectedNotiIndication;
extern volatile uint8_t gTouched;

void send_DFPlayerCmd(uint8_t *cmd, int32_t cmdLen);

#endif