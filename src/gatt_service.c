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

#include "bt_uuid.h"
#include "bt_system.h"
#include "bt_gattc.h"
#include "bt_gatt.h"
#include "bt_gatts.h"
#include "bt_att.h"
#include <string.h>
#include "project_config.h"
#include "wifi_lwip_helper.h"
#include "wifi_api.h"
#include "FreeRTOS.h"
#include "nvdm.h"
#include "sntp.h"
#include "dfplayer.h"

volatile notification_tasklet gNotiTasklet;
volatile void *gBLE_BroadcastNotiIndication;
volatile void *gBLE_WifiConnectedNotiIndication;

//Declare every record here
//service collects all bt_gatts_service_rec_t
//IMPORTAMT: handle:0x0000 is reserved, please start your handle from 0x0001

#if 0
//GATT 0x0014 - 0x0017
/*---------------------------------------------*/
const bt_uuid_t APP_BLE_FOTA_CHAR_UUID128 = BT_UUID_INIT_WITH_UUID16(APP_BLE_FOTA_CHAR_UUID);

extern uint32_t app_ble_fota_charc_value_callback (const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);
extern uint32_t app_ble_fota_client_config_callback (const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);

BT_GATTS_NEW_PRIMARY_SERVICE_16(_bt_if_dtp_primary_service, APP_BLE_FOTA_SERVICE_UUID);

BT_GATTS_NEW_CHARC_16(_bt_if_dtp_char,
                      BT_GATT_CHARC_PROP_WRITE | BT_GATT_CHARC_PROP_INDICATE, 
                      APP_BLE_FOTA_CHAR_VALUE_HANDLE, APP_BLE_FOTA_CHAR_UUID);

BT_GATTS_NEW_CHARC_VALUE_CALLBACK(_bt_if_dtp_char_value, APP_BLE_FOTA_CHAR_UUID128,
                                  BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE, 
                                  app_ble_fota_charc_value_callback);

BT_GATTS_NEW_CLIENT_CHARC_CONFIG(_bt_if_dtp_client_config,
                                 BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE,
                                 app_ble_fota_client_config_callback);

static const bt_gatts_service_rec_t *_bt_if_ble_fota_service_rec[] = {
    (const bt_gatts_service_rec_t *) &_bt_if_dtp_primary_service,
    (const bt_gatts_service_rec_t *) &_bt_if_dtp_char,
    (const bt_gatts_service_rec_t *) &_bt_if_dtp_char_value,
    (const bt_gatts_service_rec_t *) &_bt_if_dtp_client_config
};

static const bt_gatts_service_t _bt_if_ble_fota_service = {
    .starting_handle = 0x0014,
    .ending_handle = 0x0017,
    .required_encryption_key_size = 0,
    .records = _bt_if_ble_fota_service_rec
};
#endif
/************************************************
*   Macro
*************************************************/
#define CLOCK_PRIMARY_SERVICE_UUID                  (0x180A)          /* Data Transfer Over Gatt Service UUID. */
#define CLOCK_PRIMARY_MANUFACTURER_CHAR_UUID        (0x2A29)          /* MANUFACTURER String UUID */

/************************************************
*   Global
*************************************************/
const bt_uuid_t CLOCK_PRIMARY_HANDLE_MANUFACTURER_CHAR_UUID128 = BT_UUID_INIT_WITH_UUID16(CLOCK_PRIMARY_MANUFACTURER_CHAR_UUID);

enum {
    CLOCK_PRIMARY_HANDLE_START = 0x0001,
    CLOCK_PRIMARY_HANDLE_PRIMARY_SERVICE = CLOCK_PRIMARY_HANDLE_START,
    CLOCK_PRIMARY_HANDLE_MANUFACTURER_CHAR_STRING,
    CLOCK_PRIMARY_HANDLE_MANUFACTURER_CHAR_STRING_VALUE,
    CLOCK_PRIMARY_HANDLE_END
};

/************************************************
*   Utilities
*************************************************/
BT_GATTS_NEW_PRIMARY_SERVICE_16(bt_if_clock_primary_service, CLOCK_PRIMARY_SERVICE_UUID);
BT_GATTS_NEW_CHARC_16(bt_if_clock_manufacturer_char, \
                      BT_GATT_CHARC_PROP_READ, CLOCK_PRIMARY_HANDLE_MANUFACTURER_CHAR_STRING, \
                      CLOCK_PRIMARY_MANUFACTURER_CHAR_UUID);
BT_GATTS_NEW_CHARC_VALUE_STR16(bt_if_clock_manufacturer_char_value, CLOCK_PRIMARY_HANDLE_MANUFACTURER_CHAR_UUID128, \
                    BT_GATTS_REC_PERM_READABLE, DEVICE_MANUFACTURER_LEN, DEVICE_MANUFACTURER);

static const bt_gatts_service_rec_t *bt_if_clock_primary_service_rec[] = {
    (const bt_gatts_service_rec_t *) &bt_if_clock_primary_service,
    (const bt_gatts_service_rec_t *) &bt_if_clock_manufacturer_char,
    (const bt_gatts_service_rec_t *) &bt_if_clock_manufacturer_char_value,
};

const bt_gatts_service_t _bt_if_clock_primary_service = {
    .starting_handle = CLOCK_PRIMARY_HANDLE_START,
    .ending_handle = CLOCK_PRIMARY_HANDLE_END - 1,
    .required_encryption_key_size = 0,
    .records = bt_if_clock_primary_service_rec
};

/************************************************
*   Macro
*************************************************/
#define CLOCK_CURRENT_TIME_SERVICE_UUID             (0x1805)          /* Data Transfer Over Gatt Service UUID. */
#define CLOCK_CURRENT_TIME_TIMEZONE_CHAR_UUID       (0x2A0E)          /* MANUFACTURER String UUID */
#define CLOCK_CURRENT_TIME_BROADCAST_CHAR_UUID      (0x2A15)          /* MANUFACTURER String UUID */

/************************************************
*   Global
*************************************************/
const bt_uuid_t CLOCK_CURRENT_TIME_HANDLE_TIMEZONE_CHAR_UUID128 = BT_UUID_INIT_WITH_UUID16(CLOCK_CURRENT_TIME_TIMEZONE_CHAR_UUID);
const bt_uuid_t CLOCK_CURRENT_TIME_HANDLE_BROADCAST_CHAR_UUID128 = BT_UUID_INIT_WITH_UUID16(CLOCK_CURRENT_TIME_BROADCAST_CHAR_UUID);

enum {
    CLOCK_CURRENT_TIME_HANDLE_START = 0x0011,
    CLOCK_CURRENT_TIME_HANDLE_PRIMARY_SERVICE = CLOCK_CURRENT_TIME_HANDLE_START,
    CLOCK_CURRENT_TIME_HANDLE_TIMEZONE_CHAR,
    CLOCK_CURRENT_TIME_HANDLE_TIMEZONE_CHAR_VALUE,
    CLOCK_CURRENT_TIME_HANDLE_BROADCAST_CHAR,
    CLOCK_CURRENT_TIME_HANDLE_BROADCAST_CHAR_VALUE,
    CLOCK_CURRENT_TIME_HANDLE_BROADCAST_CHAR_CCCD,
    CLOCK_CURRENT_TIME_HANDLE_END
};

/************************************************
*   Utilities
*************************************************/
static uint32_t ble_clock_current_time_timezone_char_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);
static uint32_t ble_clock_current_time_broadcast_char_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);
static uint32_t ble_clock_current_time_broadcast_char_cccd_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);

BT_GATTS_NEW_PRIMARY_SERVICE_16(bt_if_clock_current_time_service, CLOCK_CURRENT_TIME_SERVICE_UUID);

BT_GATTS_NEW_CHARC_16(bt_if_clock_current_time_timezone_char, \
                      BT_GATT_CHARC_PROP_READ, CLOCK_CURRENT_TIME_HANDLE_TIMEZONE_CHAR, \
                      CLOCK_CURRENT_TIME_TIMEZONE_CHAR_UUID);

BT_GATTS_NEW_CHARC_VALUE_CALLBACK(bt_if_clock_current_time_timezone_char_value, CLOCK_CURRENT_TIME_HANDLE_TIMEZONE_CHAR_UUID128, \
                    BT_GATTS_REC_PERM_READABLE, ble_clock_current_time_timezone_char_callback);

BT_GATTS_NEW_CHARC_16(bt_if_clock_current_time_broadcast_char,
                      BT_GATT_CHARC_PROP_READ | BT_GATT_CHARC_PROP_NOTIFY, CLOCK_CURRENT_TIME_HANDLE_BROADCAST_CHAR, \
                      CLOCK_CURRENT_TIME_BROADCAST_CHAR_UUID);

BT_GATTS_NEW_CHARC_VALUE_CALLBACK(bt_if_clock_current_time_broadcast_char_value, CLOCK_CURRENT_TIME_HANDLE_BROADCAST_CHAR_UUID128, \
                    BT_GATTS_REC_PERM_READABLE, ble_clock_current_time_broadcast_char_callback);

BT_GATTS_NEW_CLIENT_CHARC_CONFIG(bt_if_clock_current_time_broadcast_char_cccd,
                                 BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE,
                                 ble_clock_current_time_broadcast_char_cccd_callback);

static const bt_gatts_service_rec_t *bt_if_clock_current_time_service_rec[] = {
    (const bt_gatts_service_rec_t *) &bt_if_clock_current_time_service,
    (const bt_gatts_service_rec_t *) &bt_if_clock_current_time_timezone_char,
    (const bt_gatts_service_rec_t *) &bt_if_clock_current_time_timezone_char_value,
    (const bt_gatts_service_rec_t *) &bt_if_clock_current_time_broadcast_char,
    (const bt_gatts_service_rec_t *) &bt_if_clock_current_time_broadcast_char_value,
    (const bt_gatts_service_rec_t *) &bt_if_clock_current_time_broadcast_char_cccd,
};

const bt_gatts_service_t _bt_if_clock_current_time_service = {
    .starting_handle = CLOCK_CURRENT_TIME_HANDLE_PRIMARY_SERVICE,
    .ending_handle = CLOCK_CURRENT_TIME_HANDLE_END - 1,
    .required_encryption_key_size = 0,
    .records = bt_if_clock_current_time_service_rec
};

static uint32_t ble_clock_current_time_timezone_char_callback (const uint8_t rw, uint16_t handle,
    void *data, uint16_t size, uint16_t offset)
{
    switch (rw) {
        case BT_GATTS_CALLBACK_READ:
            /* To handle read request. */
            if (size == 0) {
                return sizeof(gConfig.timeZone);
            }
            *(int *)data = gConfig.timeZone;
            return sizeof(gConfig.timeZone);
        default:
            return BT_STATUS_SUCCESS;
    }
}

static uint32_t ble_clock_current_time_broadcast_char_callback (const uint8_t rw, uint16_t handle,
    void *data, uint16_t size, uint16_t offset)
{
    uint32_t str_size = strlen(gTimeStringCache);
    uint32_t copy_size;
    LOG_I(app, "ble_clock_current_time_broadcast_char_callback:%s",
                rw?"BT_GATTS_CALLBACK_WRITE":"BT_GATTS_CALLBACK_READ");
    switch (rw) {
        case BT_GATTS_CALLBACK_READ:
            copy_size = (str_size > offset)?(str_size - offset):0;
            if (size == 0) {
                return str_size;
            }
            copy_size = (size > copy_size)?copy_size:size;
            memcpy(data, gTimeStringCache+offset, copy_size);
            return copy_size;
        case BT_GATTS_CALLBACK_WRITE:
        default:
            return BT_STATUS_SUCCESS;
    }
}

//BT_GATTC_NEW_PREPARE_WRITE_REQ(NotiIndication, CLOCK_CURRENT_TIME_HANDLE_BROADCAST_CHAR_VALUE, 0, gTimeStringCache, 19);

static uint32_t ble_clock_current_time_broadcast_char_cccd_callback (const uint8_t rw, uint16_t handle,
    void *data, uint16_t size, uint16_t offset)
{
    unsigned char en = 0;
    LOG_I(app, "ble_clock_current_time_broadcast_char_cccd_callback:%s, size: %d, data[0]=%u",
                rw?"BT_GATTS_CALLBACK_WRITE":"BT_GATTS_CALLBACK_READ", size, *(unsigned char *)data);
    switch (rw) {
        case BT_GATTS_CALLBACK_WRITE:
            // gNotiTasklet.conn_handle = handle;
            if (size >= 1) {
                en = *(unsigned short *)data;
            }
            if (gBLE_BroadcastNotiIndication == NULL) {
                bt_gattc_prepare_write_charc_req_t *NotiIndication = malloc(sizeof(bt_gattc_prepare_write_charc_req_t));
                bt_att_prepare_write_req_t *att_req = malloc(sizeof(bt_att_prepare_write_req_t));
                NotiIndication->attribute_value_length = sizeof(gTimeStringCache);
                att_req->opcode = BT_ATT_OPCODE_HANDLE_VALUE_NOTIFICATION;//BT_ATT_OPCODE_PREPARE_WRITE_REQUEST;
                att_req->attribute_handle = CLOCK_CURRENT_TIME_HANDLE_BROADCAST_CHAR;
                att_req->value_offset = 0x2020;
                att_req->part_attribute_value = gTimeStringCache;
                NotiIndication->att_req = att_req;
                gBLE_BroadcastNotiIndication = NotiIndication;
            }
            gNotiTasklet.enables.broadcast = en;
            return sizeof(gNotiTasklet.enables.broadcast);
        case BT_GATTS_CALLBACK_READ:
            *(uint16_t *)data = gNotiTasklet.enables.broadcast;
            return sizeof(gNotiTasklet.enables.broadcast);
        default:
            return BT_STATUS_SUCCESS;
    }
}

/************************************************
*   Macro
*************************************************/
#define CLOCK_SERVICE_UUID                  (0x1801)          /* Data Transfer Over Gatt Service UUID. */
#define CLOCK_WIFI_SSID_CHAR_UUID           (0x331D)          /* SSID Characteristic UUID. */
#define CLOCK_WIFI_PWD_CHAR_UUID            (0x9A33)          /* PASSWORD Characteristic UUID. */
#define CLOCK_TIMEZONE_CHAR_UUID            (0x2A0E)          /* TimeZone Characteristic UUID. */
#define CLOCK_WIFI_UPDATED_CHAR_UUID        (0x2A05)          /* Update Wifi Characteristic UUID. */

/************************************************
*   Global
*************************************************/
const bt_uuid_t CLOCK_HANDLE_WIFI_SSID_CHAR_UUID128 = BT_UUID_INIT_WITH_UUID16(CLOCK_WIFI_SSID_CHAR_UUID);
const bt_uuid_t CLOCK_HANDLE_WIFI_PWD_CHAR_UUID128 = BT_UUID_INIT_WITH_UUID16(CLOCK_WIFI_PWD_CHAR_UUID);
const bt_uuid_t CLOCK_HANDLE_TIMEZONE_CHAR_UUID128 = BT_UUID_INIT_WITH_UUID16(CLOCK_TIMEZONE_CHAR_UUID);
const bt_uuid_t CLOCK_HANDLE_WIFI_UPDATED_CHAR_UUID128 = BT_UUID_INIT_WITH_UUID16(CLOCK_WIFI_UPDATED_CHAR_UUID);

enum {
    CLOCK_HANDLE_START = 0x0021,
    CLOCK_HANDLE_PRIMARY_SERVICE = CLOCK_HANDLE_START,
    CLOCK_HANDLE_WIFI_SSID_CHAR_STRING,
    CLOCK_HANDLE_WIFI_SSID_CHAR_STRING_VALUE,
    CLOCK_HANDLE_WIFI_PWD_CHAR_STRING,
    CLOCK_HANDLE_WIFI_PWD_CHAR_STRING_VALUE,
    CLOCK_HANDLE_TIMEZONE_CHAR,
    CLOCK_HANDLE_TIMEZONE_CHAR_VALUE,
    CLOCK_HANDLE_WIFI_UPDATED_CHAR_SERVICE_UPDATED,
    CLOCK_HANDLE_WIFI_UPDATED_CHAR_SERVICE_UPDATED_VALUE,
    CLOCK_HANDLE_WIFI_UPDATED_CHAR_SERVICE_UPDATED_CCCD,
    CLOCK_HANDLE_END
};

/************************************************
*   Utilities
*************************************************/
static uint32_t ble_clock_wifi_ssid_char_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);
static uint32_t ble_clock_wifi_pwd_char_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);
static uint32_t ble_clock_timezone_char_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);
static uint32_t ble_clock_wifi_updated_char_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);
static uint32_t ble_clock_wifi_updated_char_cccd_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);

BT_GATTS_NEW_PRIMARY_SERVICE_16(bt_if_clock_setting_service, CLOCK_SERVICE_UUID);

BT_GATTS_NEW_CHARC_16(bt_if_clock_wifi_ssid_char,
                      BT_GATT_CHARC_PROP_READ, CLOCK_HANDLE_WIFI_SSID_CHAR_STRING,
                      CLOCK_WIFI_SSID_CHAR_UUID);

BT_GATTS_NEW_CHARC_VALUE_CALLBACK(bt_if_clock_wifi_ssid_char_value, CLOCK_HANDLE_WIFI_SSID_CHAR_UUID128,
                    BT_GATTS_REC_PERM_WRITABLE|BT_GATTS_REC_PERM_READABLE, ble_clock_wifi_ssid_char_callback);

BT_GATTS_NEW_CHARC_16(bt_if_clock_wifi_pwd_char,
                      BT_GATT_CHARC_PROP_READ, CLOCK_HANDLE_WIFI_PWD_CHAR_STRING,
                      CLOCK_WIFI_PWD_CHAR_UUID);

BT_GATTS_NEW_CHARC_VALUE_CALLBACK(bt_if_clock_wifi_pwd_char_value, CLOCK_HANDLE_WIFI_PWD_CHAR_UUID128,
                    BT_GATTS_REC_PERM_WRITABLE|BT_GATTS_REC_PERM_READABLE, ble_clock_wifi_pwd_char_callback);

BT_GATTS_NEW_CHARC_16(bt_if_clock_timezone_char,
                      BT_GATT_CHARC_PROP_READ, CLOCK_HANDLE_TIMEZONE_CHAR,
                      CLOCK_TIMEZONE_CHAR_UUID);

BT_GATTS_NEW_CHARC_VALUE_CALLBACK(bt_if_clock_timezone_char_value, CLOCK_HANDLE_TIMEZONE_CHAR_UUID128,
                    BT_GATTS_REC_PERM_WRITABLE|BT_GATTS_REC_PERM_READABLE, ble_clock_timezone_char_callback);

BT_GATTS_NEW_CHARC_16(bt_if_clock_wifi_updated_char,
                      BT_GATT_CHARC_PROP_READ | BT_GATT_CHARC_PROP_NOTIFY, CLOCK_HANDLE_WIFI_UPDATED_CHAR_SERVICE_UPDATED,
                      CLOCK_WIFI_UPDATED_CHAR_UUID);

BT_GATTS_NEW_CHARC_VALUE_CALLBACK(bt_if_clock_wifi_updated_char_service_changed, CLOCK_HANDLE_WIFI_UPDATED_CHAR_UUID128,
                    BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE, ble_clock_wifi_updated_char_callback);

BT_GATTS_NEW_CLIENT_CHARC_CONFIG(bt_if_clock_wifi_updated_char_service_changed_cccd,
                                 BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE,
                                 ble_clock_wifi_updated_char_cccd_callback);

static const bt_gatts_service_rec_t *bt_if_clock_service_rec[] = {
    (const bt_gatts_service_rec_t *) &bt_if_clock_setting_service,
    (const bt_gatts_service_rec_t *) &bt_if_clock_wifi_ssid_char,
    (const bt_gatts_service_rec_t *) &bt_if_clock_wifi_ssid_char_value,
    (const bt_gatts_service_rec_t *) &bt_if_clock_wifi_pwd_char,
    (const bt_gatts_service_rec_t *) &bt_if_clock_wifi_pwd_char_value,
    (const bt_gatts_service_rec_t *) &bt_if_clock_timezone_char,
    (const bt_gatts_service_rec_t *) &bt_if_clock_timezone_char_value,
    (const bt_gatts_service_rec_t *) &bt_if_clock_wifi_updated_char,
    (const bt_gatts_service_rec_t *) &bt_if_clock_wifi_updated_char_service_changed,
    (const bt_gatts_service_rec_t *) &bt_if_clock_wifi_updated_char_service_changed_cccd
};

const bt_gatts_service_t _bt_if_clock_service = {
    .starting_handle = CLOCK_HANDLE_START,
    .ending_handle = CLOCK_HANDLE_END - 1,
    .required_encryption_key_size = 0,
    .records = bt_if_clock_service_rec
};

static uint32_t ble_clock_wifi_ssid_char_callback (const uint8_t rw, uint16_t handle,
    void *data, uint16_t size, uint16_t offset)
{
    uint32_t str_size = strlen(gConfig.ssid);
    uint32_t buf_size = sizeof(gConfig.ssid);
    uint32_t copy_size;
    switch (rw) {
        case BT_GATTS_CALLBACK_READ:
            /* To handle read request. */
            copy_size = (str_size > offset)?(str_size - offset):0;
            if (size == 0) {
                return str_size;
            }
            copy_size = (size > copy_size)?copy_size:size;
            memcpy(data, gConfig.ssid+offset, copy_size);
            return copy_size;
        case BT_GATTS_CALLBACK_WRITE:
            /* To handle write request */
            copy_size = (size > buf_size)?buf_size:size;
            memcpy(gConfig.ssid, data, copy_size);
            memset(gConfig.ssid + copy_size, 0x0, 1);
            wifi_config_set_ssid(WIFI_PORT_STA, gConfig.ssid, copy_size);
            return copy_size;
        default:
            return BT_STATUS_SUCCESS;
    }
}

static uint32_t ble_clock_wifi_pwd_char_callback (const uint8_t rw, uint16_t handle,
    void *data, uint16_t size, uint16_t offset)
{
    uint32_t str_size = strlen(gConfig.pwd);
    uint32_t buf_size = sizeof(gConfig.pwd);
    uint32_t copy_size;
    switch (rw) {
        case BT_GATTS_CALLBACK_READ:
            /* To handle read request. */
            copy_size = (str_size > offset)?(str_size - offset):0;
            if (size == 0) {
                return str_size;
            }
            copy_size = (size > copy_size)?copy_size:size;
            memcpy(data, gConfig.pwd+offset, copy_size);
            return copy_size;
        case BT_GATTS_CALLBACK_WRITE:
            /* To handle write request */
            copy_size = (size > buf_size)?buf_size:size;
            memcpy(gConfig.pwd, data, copy_size);
            memset(gConfig.pwd + copy_size, 0x0, 1);
            wifi_config_set_wpa_psk_key(WIFI_PORT_STA, gConfig.pwd, copy_size);
            return copy_size;
        default:
            return BT_STATUS_SUCCESS;
    }
}

static uint32_t ble_clock_timezone_char_callback (const uint8_t rw, uint16_t handle,
    void *data, uint16_t size, uint16_t offset)
{
    uint32_t copy_size;
    switch (rw) {
        case BT_GATTS_CALLBACK_READ:
            /* To handle read request. */
            if (size == 0) {
                return sizeof(gConfig.timeZone);
            }
            //memcpy(data, &gConfig.timeZone, sizeof(gConfig.timeZone));
            *(int *)data = gConfig.timeZone;
            return sizeof(gConfig.timeZone);
        case BT_GATTS_CALLBACK_WRITE:
            /* To handle write request */
            copy_size = sizeof(gConfig.timeZone);
            // memcpy(&gConfig.timeZone, data, copy_size);
            gConfig.timeZone = *(int *)data;
            return copy_size;
        default:
            return BT_STATUS_SUCCESS;
    }
}

static uint32_t ble_clock_wifi_updated_char_callback (const uint8_t rw, uint16_t handle,
    void *data, uint16_t size, uint16_t offset)
{
    nvdm_status_t nvdmStatus;
    uint32_t ssid_len = strlen(gConfig.ssid);
    uint32_t pwd_len = strlen(gConfig.pwd);
    uint8_t wifi_status;
    if (rw == BT_GATTS_CALLBACK_WRITE && ssid_len > 0 && pwd_len > 0) {
        LOG_I(app, "now reload wifi settings");
        wifi_config_reload_setting();
        vTaskDelay(MS2TICK(10));
        LOG_I(app, "now stop lwip");
        lwip_net_stop(WIFI_MODE_STA_ONLY);
        vTaskDelay(MS2TICK(10));
        LOG_I(app, "now start lwip");
        lwip_net_start(WIFI_MODE_STA_ONLY);
        nvdmStatus = nvdm_write_data_item(NVDM_GRP, NVDM_GLOBAL_CONFIG, NVDM_DATA_ITEM_TYPE_RAW_DATA, (uint8_t *)&gConfig, sizeof(gConfig));
        if (nvdmStatus == NVDM_STATUS_OK) {
            LOG_I(app, "saved global configurations.");
        } else {
            LOG_I(app, "failed to save global configurations.(0x%X)", nvdmStatus);
        }
        sntp_init();
        return BT_STATUS_SUCCESS;
    } else if ( rw == BT_GATTS_CALLBACK_READ) {
        wifi_connection_get_link_status(&wifi_status);
        *(uint8_t *)data = wifi_status;
        return sizeof(uint8_t);
    } else {
        return BT_STATUS_SUCCESS;
    }
}

static uint32_t ble_clock_wifi_updated_char_cccd_callback (const uint8_t rw, uint16_t handle,
    void *data, uint16_t size, uint16_t offset)
{
    unsigned char en = 0;
    LOG_I(app, "ble_clock_wifi_updated_char_cccd_callback:%s, size: %d, data[0]=%u",
                rw?"BT_GATTS_CALLBACK_WRITE":"BT_GATTS_CALLBACK_READ", size, *(unsigned char *)data);
    switch (rw) {
        case BT_GATTS_CALLBACK_WRITE:
            // gNotiTasklet.conn_handle = handle;
            if (size >= 1) {
                en = *(unsigned short *)data;
            }
            if (gBLE_WifiConnectedNotiIndication == NULL) {
                bt_gattc_write_charc_req_t *NotiIndication = malloc(sizeof(bt_gattc_write_charc_req_t));
                bt_att_write_req_t *att_req = malloc(sizeof(bt_att_write_req_t));
                NotiIndication->attribute_value_length = 1;
                att_req->opcode = BT_ATT_OPCODE_HANDLE_VALUE_NOTIFICATION;//BT_ATT_OPCODE_PREPARE_WRITE_REQUEST;
                att_req->attribute_handle = CLOCK_HANDLE_WIFI_UPDATED_CHAR_SERVICE_UPDATED;
                att_req->attribute_value[0] = 0;
                NotiIndication->att_req = att_req;
                gBLE_WifiConnectedNotiIndication = NotiIndication;
            }
            gNotiTasklet.enables.wifiReady = en;
            return sizeof(gNotiTasklet.enables.wifiReady);
        case BT_GATTS_CALLBACK_READ:
            *(uint16_t *)data = gNotiTasklet.enables.wifiReady;
            return sizeof(gNotiTasklet.enables.wifiReady);
        default:
            return BT_STATUS_SUCCESS;
    }
}

/************************************************
*   Macro
*************************************************/
#define DFPLAYER_SERVICE_UUID                  (0x1800)          /* Data Transfer Over Gatt Service UUID. */
#define DFPLAYER_TOTAL_TRACK_CHAR_UUID         (0x3310)          /* SSID Characteristic UUID. */
#define DFPLAYER_PLAY_TRACK_CHAR_UUID          (0x3311)          /* PASSWORD Characteristic UUID. */

/************************************************
*   Global
*************************************************/
const bt_uuid_t DFPLAYER_HANDLE_TOTAL_TRACK_CHAR_UUID128 = BT_UUID_INIT_WITH_UUID16(DFPLAYER_TOTAL_TRACK_CHAR_UUID);
const bt_uuid_t DFPLAYER_HANDLE_PLAY_TRACK_CHAR_UUID128 = BT_UUID_INIT_WITH_UUID16(DFPLAYER_PLAY_TRACK_CHAR_UUID);

enum {
    DFPLAYER_HANDLE_START = 0x0031,
    DFPLAYER_HANDLE_PRIMARY_SERVICE = DFPLAYER_HANDLE_START,
    DFPLAYER_HANDLE_TOTAL_TRACK_CHAR_STRING,
    DFPLAYER_HANDLE_TOTAL_TRACK_CHAR_STRING_VALUE,
    DFPLAYER_HANDLE_PLAY_TRACK_CHAR_STRING,
    DFPLAYER_HANDLE_PLAY_TRACK_CHAR_STRING_VALUE,
    DFPLAYER_HANDLE_END
};

/************************************************
*   Utilities
*************************************************/
static uint32_t ble_dfplayer_total_track_char_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);
static uint32_t ble_dfplayer_play_track_char_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);

BT_GATTS_NEW_PRIMARY_SERVICE_16(bt_if_dfplayer_service, DFPLAYER_SERVICE_UUID);

BT_GATTS_NEW_CHARC_16(bt_if_dfplayer_total_track_char,
                      BT_GATT_CHARC_PROP_READ, DFPLAYER_HANDLE_TOTAL_TRACK_CHAR_STRING,
                      DFPLAYER_TOTAL_TRACK_CHAR_UUID);

BT_GATTS_NEW_CHARC_VALUE_CALLBACK(bt_if_dfplayer_total_track_char_value, DFPLAYER_HANDLE_TOTAL_TRACK_CHAR_UUID128,
                    BT_GATTS_REC_PERM_READABLE, ble_dfplayer_total_track_char_callback);

BT_GATTS_NEW_CHARC_16(bt_if_dfplayer_play_track_char,
                      BT_GATT_CHARC_PROP_WRITE|BT_GATT_CHARC_PROP_READ, DFPLAYER_HANDLE_PLAY_TRACK_CHAR_STRING,
                      DFPLAYER_PLAY_TRACK_CHAR_UUID);

BT_GATTS_NEW_CHARC_VALUE_CALLBACK(bt_if_dfplayer_play_track_char_value, DFPLAYER_HANDLE_PLAY_TRACK_CHAR_UUID128,
                    BT_GATTS_REC_PERM_WRITABLE|BT_GATTS_REC_PERM_READABLE, ble_dfplayer_play_track_char_callback);

static const bt_gatts_service_rec_t *bt_if_dfplayer_service_rec[] = {
    (const bt_gatts_service_rec_t *) &bt_if_dfplayer_service,
    (const bt_gatts_service_rec_t *) &bt_if_dfplayer_total_track_char,
    (const bt_gatts_service_rec_t *) &bt_if_dfplayer_total_track_char_value,
    (const bt_gatts_service_rec_t *) &bt_if_dfplayer_play_track_char,
    (const bt_gatts_service_rec_t *) &bt_if_dfplayer_play_track_char_value
};

const bt_gatts_service_t _bt_if_dfplayer_service = {
    .starting_handle = DFPLAYER_HANDLE_START,
    .ending_handle = DFPLAYER_HANDLE_END - 1,
    .required_encryption_key_size = 0,
    .records = bt_if_dfplayer_service_rec
};

static uint32_t ble_dfplayer_total_track_char_callback (const uint8_t rw, uint16_t handle,
    void *data, uint16_t size, uint16_t offset)
{
    switch (rw) {
        case BT_GATTS_CALLBACK_READ:
            /* To handle read request. */
            // queryTotalTracks();
            if (size == 0) {
                return sizeof(uint16_t);
            }
            *(uint16_t*)data = gTotalTracks;
            return sizeof(uint16_t);
        default:
            return BT_STATUS_SUCCESS;
    }
}

static uint32_t ble_dfplayer_play_track_char_callback (const uint8_t rw, uint16_t handle,
    void *data, uint16_t size, uint16_t offset)
{
    uint16_t req = *(uint16_t *)data;
    switch (rw) {
        case BT_GATTS_CALLBACK_READ:
            /* To handle read request. */
            *(uint16_t*)data = gCurrentTrack;
            return sizeof(uint16_t);
        case BT_GATTS_CALLBACK_WRITE:
            /* To handle write request */
            // queryTotalTracks();
            LOG_I(app, "request to play track:%d, total:%d", req, gTotalTracks);
            if (req >= gTotalTracks) {
                return BT_STATUS_SUCCESS;
            }
            specifyTrackId(req + 1);
            return sizeof(uint16_t);
        default:
            return BT_STATUS_SUCCESS;
    }
}

/************************************************
*   Macro
*************************************************/
#define ALARM_SERVICE_UUID                     (0x1811)          /* Data Transfer Over Gatt Service UUID. */
#define ALARM_QUERY_CHAR_UUID                  (0xa1a1)          /* Query alarm Characteristic UUID. */
#define ALARM_SET_CHAR_UUID                    (0xa1a0)          /* Set alarm Characteristic UUID. */

/************************************************
*   Global
*************************************************/
const bt_uuid_t ALARM_HANDLE_QUERY_CHAR_UUID128 = BT_UUID_INIT_WITH_UUID16(ALARM_QUERY_CHAR_UUID);
const bt_uuid_t ALARM_HANDLE_SET_CHAR_UUID128   = BT_UUID_INIT_WITH_UUID16(ALARM_SET_CHAR_UUID);

enum {
    ALARM_HANDLE_START = 0x0041,
    ALARM_HANDLE_PRIMARY_SERVICE = ALARM_HANDLE_START,
    ALARM_HANDLE_QUERY_CHAR_STRING,
    ALARM_HANDLE_QUERY_CHAR_STRING_VALUE,
    ALARM_HANDLE_SET_CHAR_STRING,
    ALARM_HANDLE_SET_CHAR_STRING_VALUE,
    ALARM_HANDLE_END
};

/************************************************
*   Utilities
*************************************************/
static uint32_t ble_alarm_query_char_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);
static uint32_t ble_alarm_set_char_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset);

BT_GATTS_NEW_PRIMARY_SERVICE_16(bt_if_alarm_service, ALARM_SERVICE_UUID);

BT_GATTS_NEW_CHARC_16(bt_if_alarm_query_char,
                      BT_GATT_CHARC_PROP_READ, ALARM_HANDLE_QUERY_CHAR_STRING,
                      ALARM_QUERY_CHAR_UUID);

BT_GATTS_NEW_CHARC_VALUE_CALLBACK(bt_if_alarm_query_char_value, ALARM_HANDLE_QUERY_CHAR_UUID128,
                    BT_GATTS_REC_PERM_READABLE, ble_alarm_query_char_callback);

BT_GATTS_NEW_CHARC_16(bt_if_alarm_set_char,
                      BT_GATT_CHARC_PROP_WRITE|BT_GATT_CHARC_PROP_READ, ALARM_HANDLE_SET_CHAR_STRING,
                      ALARM_SET_CHAR_UUID);

BT_GATTS_NEW_CHARC_VALUE_CALLBACK(bt_if_alarm_set_char_value, ALARM_HANDLE_SET_CHAR_UUID128,
                    BT_GATTS_REC_PERM_WRITABLE|BT_GATTS_REC_PERM_READABLE, ble_alarm_set_char_callback);

static const bt_gatts_service_rec_t *bt_if_alarm_service_rec[] = {
    (const bt_gatts_service_rec_t *) &bt_if_alarm_service,
    (const bt_gatts_service_rec_t *) &bt_if_alarm_query_char,
    (const bt_gatts_service_rec_t *) &bt_if_alarm_query_char_value,
    (const bt_gatts_service_rec_t *) &bt_if_alarm_set_char,
    (const bt_gatts_service_rec_t *) &bt_if_alarm_set_char_value
};

const bt_gatts_service_t _bt_if_alarm_service = {
    .starting_handle = ALARM_HANDLE_START,
    .ending_handle = ALARM_HANDLE_END - 1,
    .required_encryption_key_size = 0,
    .records = bt_if_alarm_service_rec
};

static void encapData(uint8_t *data, uint8_t *size, unsigned long nData)
{
    int32_t i;
    uint8_t temp[sizeof(alarm_config) * MAX_AMOUNT_ALARMS];
    uint8_t cs;
    memcpy(temp, data, *size);
    *data = 0xAA;
    *(data + 1) = 0x55;
    *(data + 2) = *size + 1;
    *(data + 3) = nData;
    for (i = 0, cs = 0; i < *size; i++) cs += temp[i];
    cs += *(data + 2);
    cs += *(data + 3);
    *(data + *size + 4) = cs;
    memcpy(data + 4, temp, *size);
    *size = *size + 5;
}

static uint32_t ble_alarm_query_char_callback (const uint8_t rw, uint16_t handle,
    void *data, uint16_t size, uint16_t offset)
{
    uint8_t temp[sizeof(alarm_config) * MAX_AMOUNT_ALARMS + 5];
    uint8_t pkgSze = gConfig.nAlarms * sizeof(alarm_config);
    int32_t i;
    switch (rw) {
        case BT_GATTS_CALLBACK_READ:
            /* To handle read request. */
            // queryTotalTracks();
            if (size == 0 || gConfig.nAlarms == 0) {
                return sizeof(uint16_t);
            }
            for(i = 0; i < gConfig.nAlarms; i++) {
                memcpy(temp, &(gConfig.alarms[i]), sizeof(alarm_config));
            }
            encapData(temp, &pkgSze, gConfig.nAlarms);
            memcpy(data, temp, pkgSze);
            return pkgSze;
        default:
            return BT_STATUS_SUCCESS;
    }
}

static uint8_t decapData(uint8_t *data, alarm_config* out, int8_t* index)
{
    return 0;
}

static uint32_t ble_alarm_set_char_callback (const uint8_t rw, uint16_t handle,
    void *data, uint16_t size, uint16_t offset)
{
    static int8_t currentAlarmIndex = -1;
    int8_t index;
    uint8_t pkgSze;
    uint8_t temp[sizeof(alarm_config) + 5];
    alarm_config setup;
    switch (rw) {
        case BT_GATTS_CALLBACK_READ:
            /* To handle read request. */
            if (size == 0 || gConfig.nAlarms == 0) {
                return sizeof(uint16_t);
            }
            if (currentAlarmIndex == -1) {
                index = 0;
            }
            memcpy(temp, &(gConfig.alarms[index]), sizeof(alarm_config));
            pkgSze = sizeof(alarm_config);
            encapData(temp, &pkgSze, 1);
            memcpy(data, temp, pkgSze);
            return pkgSze;
        case BT_GATTS_CALLBACK_WRITE:
            /* To handle write request */
            if (size == 0) {
                return sizeof(uint16_t);
            }
            if (decapData(data, &setup, &index) != 0) {
                LOG_I(app, "decap data failed.");
                return sizeof(uint16_t);
            }
            LOG_I(app, "request to setup alarm:%d", index);
            return BT_STATUS_SUCCESS;
        default:
            return BT_STATUS_SUCCESS;
    }
}

static const bt_gatts_service_t * _bt_if_gatt_server[] = {
    //&bt_if_gap_service,//0x0001
    //&bt_if_gatt_service,//0x0011
    //&_bt_if_ble_fota_service, //0x0014-0x0017
    //&bt_if_dogp_service,//0x0020-0x0025
    &_bt_if_clock_primary_service,
    &_bt_if_clock_current_time_service,
    &_bt_if_clock_service,
    &_bt_if_dfplayer_service,
    &_bt_if_alarm_service,
    NULL
};

//When GATTS get req from remote client, GATTS will call bt_get_gatt_server() to get application's gatt service DB.
//You have to return the DB(bt_gatts_service_t pointer) to gatts stack.
const bt_gatts_service_t** bt_get_gatt_server()
{
    return _bt_if_gatt_server;
}

