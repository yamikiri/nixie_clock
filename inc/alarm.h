#ifndef __ALARM_H__
#define __ALARM_H__

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "hal_rtc.h"

void checkAlarm(hal_rtc_time_t* t);

#endif
