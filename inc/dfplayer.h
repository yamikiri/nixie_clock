#ifndef __DFPLAYER_H__
#define __DFPLAYER_H__

#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct _dfplayer_instruct {
    uint8_t startByte;
    uint8_t version;
    uint8_t length;
    uint8_t cmd;
    uint8_t feedback;
    uint8_t paraMSB;
    uint8_t paraLSB;
    uint8_t checksumMSB;
    uint8_t checksumLSB;
    uint8_t endByte;
} dfplayer_instrction;

void playback(void);
void specifyTrackId(uint16_t trackId);
void specifySD(void);
void specifyVolume(uint8_t vol);
void queryTotalTracks(void);
void DFPlayerReset(void);
void queryOnlineStatus(void);

extern volatile uint16_t gCurrentTrack;
extern volatile uint16_t gTotalTracks;

#endif