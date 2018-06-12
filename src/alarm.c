/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* device.h includes */
#include "mt7687.h"
#include "system_mt7687.h"

#include "sys_init.h"
#include "task_def.h"

#include "project_config.h"
#include "alarm.h"
#include "dfplayer.h"

log_create_module(alarm, PRINT_LEVEL_INFO);

volatile uint8_t gAlarmMode;

void checkAlarm(hal_rtc_time_t* t)
{
    ;
}