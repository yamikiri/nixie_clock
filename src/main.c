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

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* device.h includes */
#include "mt7687.h"
#include "system_mt7687.h"

#include "sys_init.h"
#include "task_def.h"

#include "wifi_lwip_helper.h"
#include "wifi_api.h"

/* ble related header */
#include "bt_init.h"
#include "bt_gatts.h"
#include "bt_gap_le.h"
#include "bt_uuid.h"
#include "bt_debug.h"
#include "bt_type.h"
#include "bt_notify.h"

/* app-wise config */
#include "project_config.h"

/* hal */
#include "nvdm.h"

/* Create the log control block as user wishes. Here we use 'template' as module name.
 * User needs to define their own log control blocks as project needs.
 * Please refer to the log dev guide under /doc folder for more details.
 */
log_create_module(app, PRINT_LEVEL_INFO);

/* ---------------------------------------------------------------------------- */
static void app_btnotify_init(void);

/* ---------------------------------------------------------------------------- */

#define APP_BLE_MAX_INTERVAL          0x00C0    /*The range is from 0x0020 to 0x4000.*/
#define APP_BLE_MIN_INTERVAL          0x00C0    /*The range is from 0x0020 to 0x4000.*/
#define APP_BLE_CHANNEL_NUM           7
#define APP_BLE_FILTER_POLICY         0
#define APP_BLE_AD_FLAG_LEN           2
#define APP_BLE_AD_UUID_LEN           3


static void app_start_advertising(void)
{
    bt_hci_cmd_le_set_advertising_enable_t enable;
    bt_hci_cmd_le_set_advertising_parameters_t adv_param = {
            .advertising_interval_min = APP_BLE_MIN_INTERVAL,
            .advertising_interval_max = APP_BLE_MAX_INTERVAL,
            .advertising_type = BT_HCI_ADV_TYPE_CONNECTABLE_UNDIRECTED,
            .own_address_type = BT_ADDR_RANDOM,
            .advertising_channel_map = APP_BLE_CHANNEL_NUM,
            .advertising_filter_policy = APP_BLE_FILTER_POLICY
        };
    bt_hci_cmd_le_set_advertising_data_t adv_data;

    adv_data.advertising_data[0] = APP_BLE_AD_FLAG_LEN;
    adv_data.advertising_data[1] = BT_GAP_LE_AD_TYPE_FLAG;
    adv_data.advertising_data[2] = BT_GAP_LE_AD_FLAG_BR_EDR_NOT_SUPPORTED | BT_GAP_LE_AD_FLAG_GENERAL_DISCOVERABLE;

    adv_data.advertising_data[3] = APP_BLE_AD_UUID_LEN;
    adv_data.advertising_data[4] = BT_GAP_LE_AD_TYPE_16_BIT_UUID_COMPLETE;
    adv_data.advertising_data[5] = APP_BLE_SERVICE_UUID & 0x00FF;
    adv_data.advertising_data[6] = (APP_BLE_SERVICE_UUID & 0xFF00)>>8;

    adv_data.advertising_data[7] = 1+strlen(APP_BLE_DEVICE_NAME);
    adv_data.advertising_data[8] = BT_GAP_LE_AD_TYPE_NAME_COMPLETE;
    memcpy(adv_data.advertising_data+9, APP_BLE_DEVICE_NAME, strlen(APP_BLE_DEVICE_NAME));

    adv_data.advertising_data_length = 9 + strlen(APP_BLE_DEVICE_NAME);

    enable.advertising_enable = BT_HCI_ENABLE;
    bt_gap_le_set_advertising(&enable, &adv_param, &adv_data, NULL);
}

extern bt_bd_addr_t local_public_addr;

// called by bt_app_event_callback@bt_common.c
bt_status_t app_bt_event_callback(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    LOG_I(app, "---> bt_event_callback(0x%08X,%d)", msg, status);

    switch(msg)
    {
    case BT_POWER_ON_CNF:
        LOG_I(app, "[BT_POWER_ON_CNF](%d)", status);

        // set random address before advertising
        LOG_I(app, "bt_gap_le_set_random_address()");    
        bt_gap_le_set_random_address((bt_bd_addr_ptr_t)local_public_addr);

        app_btnotify_init();
        break;

    case BT_GAP_LE_SET_RANDOM_ADDRESS_CNF: 
        LOG_I(app, "[BT_GAP_LE_SET_RANDOM_ADDRESS_CNF](%d)", status);

        // start advertising
        app_start_advertising();
        break;

    case BT_GAP_LE_SET_ADVERTISING_CNF:
        LOG_I(app, "[BT_GAP_LE_SET_ADVERTISING_CNF](%d)", status);
        break;

    case BT_GAP_LE_DISCONNECT_IND:
        LOG_I(app, "[BT_GAP_LE_DISCONNECT_IND](%d)", status);

        // start advertising
        app_start_advertising();
        break;

    case BT_GAP_LE_CONNECT_IND:
        LOG_I(app, "[BT_GAP_LE_CONNECT_IND](%d)", status);

        bt_gap_le_connection_ind_t *connection_ind = (bt_gap_le_connection_ind_t *)buff;
        LOG_I(app, "-> connection handle = 0x%04x, role = %s", connection_ind->connection_handle, (connection_ind->role == BT_ROLE_MASTER)? "master" : "slave");

        LOG_I(app, "************************");
        LOG_I(app, "BLE connected!!");
        LOG_I(app, "************************");
        break;
    }

    LOG_I(app, "<--- bt_event_callback(0x%08X,%d)", msg, status);
    return BT_STATUS_SUCCESS;
}

/* ---------------------------------------------------------------------------- */

/* command, hardcoded at mobile-side application */
#define SERIAL_EXTCMD_UPDATE_BIN_SENDER           "myserial"
#define SERIAL_EXTCMD_UPDATE_BIN_RECEIVER         "myserial"

static bt_bd_addr_t g_remote_bt_addr;

static void app_btnotify_msg_hdlr(void *data)
{
    bt_notify_callback_data_t *p_data = (bt_notify_callback_data_t *)data;

    LOG_I(app, "> app_btnotify_msg_hdlr(evt_id=%d)", p_data->evt_id);

    switch (p_data->evt_id) 
    {
    case BT_NOTIFY_EVENT_CONNECTION:
        LOG_I(app, "  >BT_NOTIFY_EVENT_CONNECTION");
        memcpy(g_remote_bt_addr, p_data->bt_addr, 6);
        break;

    case BT_NOTIFY_EVENT_DISCONNECTION:
        LOG_I(app, "  >BT_NOTIFY_EVENT_DISCONNECTION");
        memset(g_remote_bt_addr, 0, 6);
        break;

    case BT_NOTIFY_EVENT_SEND_IND:
        /*send  new/the rest data flow start*/
        LOG_I(app, "  >BT_NOTIFY_EVENT_SEND_IND");
        break;

    case BT_NOTIFY_EVENT_DATA_RECEIVED:
        /*receive data*/
        LOG_I(app, "  >BT_NOTIFY_EVENT_DATA_RECEIVED(code=%d,len=%d)", p_data->event_data.error_code, p_data->event_data.length);
        if (strcmp(p_data->event_data.sender_id, SERIAL_EXTCMD_UPDATE_BIN_SENDER) ||
            strcmp(p_data->event_data.receiver_id, SERIAL_EXTCMD_UPDATE_BIN_RECEIVER))
        {
            LOG_E(app, "sender (%s) or receiver (%s) error", p_data->event_data.sender_id, p_data->event_data.receiver_id);
            break;
        }

        char buf[65];
        int32_t len = p_data->event_data.length;
        if(len > 64)
            len = 64;
        memcpy(buf, p_data->event_data.data, len);
        buf[len] = 0;

        LOG_I(app, "Receive data(%s)", buf);

        // reverse content
        int32_t i,j;
        for(i=0,j=len-1;i<j;i++,j--)
        {
            char t;
            t = buf[i];
            buf[i] = buf[j];
            buf[j] = t;
        }

        // send it back
        static char fullcmd[128];
        sprintf(fullcmd, "%s %s %d %d %s",
                SERIAL_EXTCMD_UPDATE_BIN_SENDER,
                SERIAL_EXTCMD_UPDATE_BIN_RECEIVER,
                0,
                len,
                buf);

        int32_t written = bt_notify_send_data(&g_remote_bt_addr, fullcmd, strlen(fullcmd), true);
        if (written != strlen(fullcmd)) {
            LOG_E(app, "Send data unfinished, plan to write(%d), actual written(%d)", strlen(fullcmd), written);
        }
        break;
    default:
        break;
    }

    LOG_I(app, "< app_btnotify_msg_hdlr(evt_id=%d)", p_data->evt_id);
}

static void app_btnotify_init(void)
{
    bt_notify_init(0);
    bt_notify_register_callback(NULL, SERIAL_EXTCMD_UPDATE_BIN_SENDER, app_btnotify_msg_hdlr);    

    log_config_print_switch(NOTIFY, DEBUG_LOG_OFF);
    log_config_print_switch(NOTIFY_SRV, DEBUG_LOG_OFF);
    log_config_print_switch(DOGP, DEBUG_LOG_OFF);
    log_config_print_switch(DOGP_ADP, DEBUG_LOG_OFF);
    log_config_print_switch(DOGP_CM, DEBUG_LOG_OFF);
}

/* ---------------------------------------------------------------------------- */

extern void bt_common_init(void);

#include "hal.h"
#include "sntp.h"
#include <time.h>
#define NTP_SERVER1 "time.stdtime.gov.tw"
#define NTP_SERVER2 "clock.stdtime.gov.tw"
#define TIMEZONE_OFFSET     8
#define CURR_CENTURY 2000
#define AP_SSID "solidyear_B10"
#define AP_PWD "B10trumP"
#define US2TICK(us) (us/(1000*portTICK_RATE_MS))
#define MS2TICK(ms) (ms/(portTICK_RATE_MS))

static void _timezone_shift(hal_rtc_time_t* t, int offset_hour)
{
    struct tm gt;
    struct tm *nt;
    time_t secs;
    gt.tm_year = t->rtc_year + (CURR_CENTURY-1900);
    gt.tm_mon = t->rtc_mon-1;
    gt.tm_mday = t->rtc_day;
    gt.tm_wday = t->rtc_week;
    gt.tm_hour = t->rtc_hour;
    gt.tm_min = t->rtc_min;
    gt.tm_sec = t->rtc_sec;
    secs = mktime(&gt);
    secs += offset_hour * 3600;
    nt = gmtime(&secs);
    if (!nt) {
        nt = localtime(&secs);
    }
    t->rtc_year = (nt->tm_year % 100);
    t->rtc_mon = nt->tm_mon + 1;
    t->rtc_day = nt->tm_mday;
    t->rtc_week = nt->tm_wday;
    t->rtc_hour = nt->tm_hour;
    t->rtc_min = nt->tm_min;
    t->rtc_sec = nt->tm_sec;
}

static void _sntp_check_loop(void)
{
    hal_rtc_time_t r_time;
    hal_rtc_status_t ret;

    while(1) {
        ret = hal_rtc_get_time(&r_time);
        if (ret == 0)
        {
            char buf[20];

            _timezone_shift(&r_time, TIMEZONE_OFFSET);
            LOG_I(app, "%04d/%d/%d %02d:%02d:%02d", r_time.rtc_year+CURR_CENTURY, r_time.rtc_mon, r_time.rtc_day, r_time.rtc_hour, r_time.rtc_min, r_time.rtc_sec);

            snprintf(buf, 19, "%04d/%d/%d", r_time.rtc_year+CURR_CENTURY, r_time.rtc_mon, r_time.rtc_day);
            snprintf(buf, 19, "%02d:%02d:%02d", r_time.rtc_hour, r_time.rtc_min, r_time.rtc_sec);
        }

        // wait 1 sec and retry
        vTaskDelay(MS2TICK(1000));
    }
}

static void main_task(void *args)
{
    //SNTP start.
    sntp_setservername(0, NTP_SERVER1);
    sntp_setservername(1, NTP_SERVER2);
    sntp_init();
    LOG_I(app, "SNTP inited");

    vTaskDelay(MS2TICK(1000));
    _sntp_check_loop();
}

/**
* @brief       Main function
* @param[in]   None.
* @return      None.
*/
int main(void)
{
    /* Do system initialization, eg: hardware, nvdm and random seed. */
    system_init();

    /* system log initialization.
     * This is the simplest way to initialize system log, that just inputs three NULLs
     * as input arguments. User can use advanved feature of system log along with NVDM.
     * For more details, please refer to the log dev guide under /doc folder or projects
     * under project/mtxxxx_hdk/apps/.
     */
    log_init(NULL, NULL, NULL);

    LOG_I(app, "start to create task.\n");

    /* User initial the parameters for wifi initial process,  system will determin which wifi operation mode
     * will be started , and adopt which settings for the specific mode while wifi initial process is running*/
    wifi_config_t config = {0};
    config.opmode = WIFI_MODE_STA_ONLY;
    strcpy((char *)config.sta_config.ssid, (const char *)AP_SSID);
    strcpy((char *)config.sta_config.password, (const char *)AP_PWD);
    config.sta_config.ssid_length = strlen((const char *)config.sta_config.ssid);
    config.sta_config.password_length = strlen((const char *)config.sta_config.password);


    /* Initialize wifi stack and register wifi init complete event handler,
     * notes:  the wifi initial process will be implemented and finished while system task scheduler is running.*/
    wifi_init(&config, NULL);

    /* Tcpip stack and net interface initialization,  dhcp client, dhcp server process initialization*/
    lwip_network_init(config.opmode);
    lwip_net_start(config.opmode);

    bt_create_task();
    bt_common_init();
	
	/* As for generic HAL init APIs like: hal_uart_init(), hal_gpio_init() and hal_spi_master_init() etc,
     * user can call them when they need, which means user can call them here or in user task at runtime.
     */

    /* Create a user task for demo when and how to use wifi config API to change WiFI settings,
    Most WiFi APIs must be called in task scheduler, the system will work wrong if called in main(),
    For which API must be called in task, please refer to wifi_api.h or WiFi API reference.
    xTaskCreate(user_wifi_app_entry,
                UNIFY_USR_DEMO_TASK_NAME,
                UNIFY_USR_DEMO_TASK_STACKSIZE / 4,
                NULL, UNIFY_USR_DEMO_TASK_PRIO, NULL);
    user_wifi_app_entry is user's task entry function, which may be defined in another C file to do application job.
    UNIFY_USR_DEMO_TASK_NAME, UNIFY_USR_DEMO_TASK_STACKSIZE and UNIFY_USR_DEMO_TASK_PRIO should be defined
    in task_def.h. User needs to refer to example in task_def.h, then makes own task MACROs defined.
    */
	xTaskCreate(main_task, "main task", 1024, NULL, 1, NULL);


    /* Call this function to indicate the system initialize done. */
    SysInitStatus_Set();

    /* Start the scheduler. */
    vTaskStartScheduler();

    /* If all is well, the scheduler will now be running, and the following line
    will never be reached.  If the following line does execute, then there was
    insufficient FreeRTOS heap memory available for the idle and/or timer tasks
    to be created.  See the memory management section on the FreeRTOS web site
    for more details. */
    for( ;; );
}

