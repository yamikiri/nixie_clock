/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* device.h includes */
#include "mt7687.h"
#include "system_mt7687.h"

#include "sys_init.h"
#include "task_def.h"

#include "project_config.h"
#include "dfplayer.h"

log_create_module(dfplayer, PRINT_LEVEL_INFO);

void calculateChecksum(dfplayer_instrction *inst)
{
    uint16_t sum = inst->version + inst->length + inst->cmd + inst->feedback + inst->paraMSB + inst->paraLSB;
    sum = 0 - sum;
    inst->checksumMSB = (uint8_t)(((uint16_t)sum & 0xFF00) >> 8);
    inst->checksumLSB = (uint8_t)((uint16_t)sum & 0x00FF);
}

void specifySD(void)
{
    dfplayer_instrction inst = {
        .startByte = 0x7e,
        .version = 0xff,
        .length = 6,
        .cmd = 0x09,
        .feedback = 0,
        .paraMSB = 0,
        .paraLSB = 2,
//        .checksumMSB = 0xFE,
//        .checksumLSB = 0xF0,
        .endByte = 0xef
    };
    calculateChecksum(&inst);
    send_DFPlayerCmd((uint8_t *)&inst, sizeof(inst));
}

void specifyTrackId(uint16_t trackId)
{
    dfplayer_instrction inst = {
        .startByte = 0x7e,
        .version = 0xff,
        .length = 6,
        .cmd = 0x14,
        .feedback = 0,
        .paraMSB = 0,
        .paraLSB = 0,
//        .checksumMSB = 0xFE,
//        .checksumLSB = 0xF0,
        .endByte = 0xef
    };
    inst.paraMSB = (trackId & 0xFF00) >> 8;
    inst.paraLSB = trackId & 0xFF;
    inst.paraMSB = inst.paraMSB | (DEFAULT_SDCARD_MP3_FOLDER_NAME << 4);
    calculateChecksum(&inst);
    send_DFPlayerCmd((uint8_t *)&inst, sizeof(inst));
    gCurrentTrack = trackId;
}

void setRepeatMode(uint8_t en)
{
    dfplayer_instrction inst = {
        .startByte = 0x7e,
        .version = 0xff,
        .length = 6,
        .cmd = 0x19,
        .feedback = 0,
        .paraMSB = 0,
        .paraLSB = 0,
//        .checksumMSB = 0xFE,
//        .checksumLSB = 0xF0,
        .endByte = 0xef
    };
    inst.paraLSB = (en == 0)?1:0;
    calculateChecksum(&inst);
    send_DFPlayerCmd((uint8_t *)&inst, sizeof(inst));
}

void specifyTrackIdRepeat(uint16_t trackId)
{
    setRepeatMode(1);
    vTaskDelay(MS2TICK(10));
    specifyTrackId(trackId);
}

void stopPlayback(void)
{
    dfplayer_instrction inst = {
        .startByte = 0x7e,
        .version = 0xff,
        .length = 6,
        .cmd = 0x16,
        .feedback = 0,
        .paraMSB = 0,
        .paraLSB = 0,
//        .checksumMSB = 0xFE,
//        .checksumLSB = 0xF0,
        .endByte = 0xef
    };
    calculateChecksum(&inst);
    send_DFPlayerCmd((uint8_t *)&inst, sizeof(inst));
    vTaskDelay(MS2TICK(10));
    setRepeatMode(0);
    gCurrentTrack = -1;
}

void queryTotalTracks(void)
{
    dfplayer_instrction inst = {
        .startByte = 0x7e,
        .version = 0xff,
        .length = 6,
        .cmd = 0x4E,
        .feedback = 0,
        .paraMSB = 0,
        .paraLSB = 0,
//        .checksumMSB = 0xFE,
//        .checksumLSB = 0xF0,
        .endByte = 0xef
    };
    inst.paraLSB = DEFAULT_SDCARD_MP3_FOLDER_NAME;
    calculateChecksum(&inst);
    send_DFPlayerCmd((uint8_t *)&inst, sizeof(inst));
}

void specifyVolume(uint8_t vol)
{
    dfplayer_instrction inst = {
        .startByte = 0x7e,
        .version = 0xff,
        .length = 6,
        .cmd = 0x06,
        .feedback = 0,
        .paraMSB = 0,
        .paraLSB = vol,
//        .checksumMSB = 0xFE,
//        .checksumLSB = 0xF0,
        .endByte = 0xef
    };
    calculateChecksum(&inst);
    send_DFPlayerCmd((uint8_t *)&inst, sizeof(inst));
}

void playback(void)
{
    //0x0D
    dfplayer_instrction inst = {
        .startByte = 0x7e,
        .version = 0xff,
        .length = 6,
        .cmd = 0x0F,
        .feedback = 0,
        .paraMSB = 0,
        .paraLSB = 0,
//        .checksumMSB = 0xFE,
//        .checksumLSB = 0xEE,
        .endByte = 0xef
    };
    inst.paraLSB = DEFAULT_SDCARD_MP3_FOLDER_NAME;
    calculateChecksum(&inst);
    send_DFPlayerCmd((uint8_t *)&inst, sizeof(inst));
}

void DFPlayerReset(void)
{
    //0x0D
    dfplayer_instrction inst = {
        .startByte = 0x7e,
        .version = 0xff,
        .length = 6,
        .cmd = 0x0C,
        .feedback = 0,
        .paraMSB = 0,
        .paraLSB = 0,
//        .checksumMSB = 0xFE,
//        .checksumLSB = 0xEE,
        .endByte = 0xef
    };
    calculateChecksum(&inst);
    send_DFPlayerCmd((uint8_t *)&inst, sizeof(inst));
}

void queryOnlineStatus(void)
{
    //0x0D
    dfplayer_instrction inst = {
        .startByte = 0x7e,
        .version = 0xff,
        .length = 6,
        .cmd = 0x3F,
        .feedback = 0,
        .paraMSB = 0,
        .paraLSB = 0,
//        .checksumMSB = 0xFE,
//        .checksumLSB = 0xEE,
        .endByte = 0xef
    };
    calculateChecksum(&inst);
    send_DFPlayerCmd((uint8_t *)&inst, sizeof(inst));
}

void responseParser(uint8_t* resp)
{
    static uint8_t a3cnt = 0;
    dfplayer_instrction *inst = (dfplayer_instrction *)resp;
    switch (inst->cmd) {
    case 0x4E:
        gTotalTracks = inst->paraMSB << 8 | inst->paraLSB;
        LOG_I(dfplayer, "total tracks: %d", gTotalTracks);
        break;
    case 0x3A:
        if ((a3cnt++ %2) == 1) {
            vTaskDelay(MS2TICK(1000));
            queryOnlineStatus();
        }
        break;
    case 0x3F:
        if (inst->paraLSB == 0x02) {
            queryTotalTracks();
        }
        break;
    case 0x3B:
        gTotalTracks = 0;
        break;
    case 0x3C:
    case 0x3D:
        setRepeatMode(0);
        gCurrentTrack = -1;
        if (gAlarmMode == 1) {
            gAlarmMode = 0;
        }
        break;
    };
}
