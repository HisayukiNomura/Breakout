#ifndef __IOINIT_H__
#define __IOINIT_H__
#include "gd32vf103.h"

void Inp_init(void);
void Outp_init(void);
void Adc_init(void) ;
void IO_init(void);

FlagStatus Get_BOOT0SW(void);
FlagStatus Get_P1Button(void);
FlagStatus Get_P2Button(void);
uint16_t Get_adc(int ch);
void timer_pwm_init(void);
#endif