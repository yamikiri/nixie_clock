/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* device.h includes */
#include "mt7687.h"
#include "system_mt7687.h"
#include "nvdm.h"

#include "sys_init.h"
#include "task_def.h"

#include "project_config.h"
#include "alarm.h"
#include "dfplayer.h"

log_create_module(alarm, PRINT_LEVEL_INFO);

volatile uint8_t gAlarmMode;

void checkAlarm(hal_rtc_time_t* t)
{
    uint32_t i = 0, j = 0;
    alarm_config *pac = NULL;
    static uint8_t preAlarmFired = 0;
    nvdm_status_t nvdmStatus;
    // static uint8_t current
    for (i = 0; i < gConfig.nAlarms; i++) {
        pac = &(gConfig.alarms[i]);
        if (pac->AlarmEnable > 0) {
            if (pac->AlarmSchedule[t->rtc_week] == 1 &&
                pac->AlarmHour == t->rtc_hour && 
                pac->AlarmMinute == t->rtc_min) {
                if (preAlarmFired == 0) {
                    if (pac->AlarmMusicPlayMode == AlarmMusicOneshot)
                        specifyTrackId(pac->AlarmMusicIndex + 1);
                    else if (pac->AlarmMusicPlayMode == AlarmMusicRepeat)
                        specifyTrackIdRepeat(pac->AlarmMusicIndex + 1);

                    if (pac->AlarmType == AlarmTypeOneshot) {
                        pac->AlarmEnable = 0;
                        nvdmStatus = nvdm_write_data_item(NVDM_GRP, NVDM_GLOBAL_CONFIG, NVDM_DATA_ITEM_TYPE_RAW_DATA, (uint8_t *)&gConfig, sizeof(gConfig));
                    }

                    gAlarmMode = 1;
                    preAlarmFired = 1;
                }
                continue;
            } else {
                preAlarmFired = 0;
                gAlarmMode = 0;
            }
        }
    }
}