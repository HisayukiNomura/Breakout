#include "lcd/lcd.h"
#include "led.h"
#include "memory.h"
#include "ioinit.h"



bool 	isSound = TRUE;

void StartSound(void) {
    if (isSound == FALSE) return;
    timer_enable(TIMER4);		
}
void StopSound(void) {
    if (isSound == FALSE) return;
    timer_disable(TIMER4);		
}
void SoundSet(int Period)
{
    TIMER_CH3CV(TIMER4) = Period / 2;	   
    TIMER_CAR(TIMER4) = Period;	 	
}

void Beep(int ms)
{
    SoundSet(800);
    StartSound();
    delay_1ms(ms);
    StopSound();
}

/*
ド	261.63
レ	293.67
ミ	329.63
ファ	349.23
ソ	392.00
ラ	440.00
シ	493.88
ド	523.23
レ	587.34
ミ	659.25
ファ	698.45
ソ	783.98
ラ	879.99
シ	987.75
*/
//               C   C#    D    D#   E    F   F#   G    G#    A    A#   B  
//const int FreqTbl[]={1528,1443,1362,1286,1213,1145,1081,1020, 963, 909, 858, 809};
const int FreqTbl[]={3058,2886,2724,2571,2427,2291,2162,2041,1926,1818,1716,1620};
const char* sndChar =  "CcDdEFfGgAaB";
#define TOVAL(__C__) ((__C__) - '0')

void SoundPlay(char *Snd,int waitCnt)
{

    char* pWork = Snd;
    while (*pWork != 0) {
        char S1 = *pWork++;
        if (S1 == ' ') {
            delay_1ms(waitCnt);
            continue;
        }
        char S2 = *pWork++;
        int idx = strchr(sndChar,(int)S1) - sndChar;
        int Freq = FreqTbl[idx] / (1 << (int)TOVAL(S2));
        int len = TOVAL(*pWork)*100 + TOVAL(*(pWork+1)) *10 +  TOVAL(*(pWork+2));
        pWork+=3;
        SoundSet(Freq);
        StartSound();
        delay_1ms(len);
        StopSound();
    }
}