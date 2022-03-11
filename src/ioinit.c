#include "lcd/lcd.h"
#include "gd32vf103.h"
#include "fatfs/tf_card.h"
#include <string.h>


void Inp_init(void)
{
	//gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_8);
	gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ,GPIO_PIN_7|GPIO_PIN_6|GPIO_PIN_8);
}
void Outp_init(void) 
{
    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8);	// B8をデバッグに使う
  
	
	// 音が出っぱなしになっていたたら止めるため、タイマーを停止してA3をGPIOに戻す
    rcu_periph_clock_enable(RCU_GPIOA);	
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3);
	gpio_bit_reset(GPIOA,GPIO_PIN_3);
    delay_1ms(100);
	gpio_bit_set(GPIOA,GPIO_PIN_3);


}
// ボリューム入力の設定
void Adc_init(void) 
{
	// 	GPIOポートA　を、アナログ入力モード、５０MHｚ、ピン０とピン１
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_0|GPIO_PIN_1);
	// ADCのクロックプリスケーラを APB2/12 に設定する
	rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV12);

	// ADC0にクロックを供給する
    rcu_periph_clock_enable(RCU_ADC0);

	// ADCをクリアする
	adc_deinit(ADC0);
	// ADCをフリーモード（全ADCを独立して動作させる）にする。(ADC_CTL0のSYNCMを0b000)
	adc_mode_config(ADC_MODE_FREE);
	// ADCのデータを右詰めにする
	adc_data_alignment_config(ADC0,  ADC_DATAALIGN_RIGHT);
	// チャンネルグループのデータ長を１に設定する。（単発）
	adc_channel_length_config(ADC0,  ADC_REGULAR_CHANNEL,1);
	// ADCの外部トリガーソースを無効にする
 	adc_external_trigger_source_config(ADC0, ADC_REGULAR_CHANNEL, ADC0_1_EXTTRIG_INSERTED_NONE);	
	// ADCの外部トリガーのレギュラーチャンネルを有効にする
	adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, ENABLE);
	// ADCを有効にする
	adc_enable(ADC0);
    delay_1ms(1);
	// キャリブレーションを行う。この関数は、中でキャリブレーションの完了を待つので、呼び出すだけでよい
    adc_calibration_enable(ADC0);    
/*

	// RCU(Reset and Control Unit) の設定
	// http://blueeyes.sakura.ne.jp/2019/10/18/2649/
	// ポートA0,A1をAnalog in に接続
	
	// 	GPIOポートA　を、アナログ入力モード、５０MHｚ、ピン０とピン１
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_0|GPIO_PIN_1);
	// ADCのクロックプリスケーラ設定　
	// RCU_CFG0|=(0b10<<14)|(1<<28);
	// ADC prescaler select CK_APB2/12。
	rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV12);

	
    rcu_periph_clock_enable(RCU_ADC0);

	adc_data_alignment_config(ADC0,  ADC_DATAALIGN_RIGHT);
	// ADC0 をオンにする
    ADC_CTL1(ADC0)|=ADC_CTL1_ADCON;
*/
}

void IO_init(void)
{
	Inp_init();	// inport init
	Outp_init();	// 出力ポート
	Adc_init();	// A/D init
	Lcd_Init();	// LCD init
}



FlagStatus Get_BOOT0SW(void)
{
	return(gpio_input_bit_get(GPIOA, GPIO_PIN_8));
}
FlagStatus Get_P1Button(void)
{
	return (gpio_input_bit_get(GPIOA,GPIO_PIN_6));
}
FlagStatus Get_P2Button(void)
{
	return (gpio_input_bit_get(GPIOA,GPIO_PIN_7));
}
uint16_t Get_adc(int ch)
{
    ADC_RSQ2(ADC0)=0;
    ADC_RSQ2(ADC0)=ch;
    ADC_CTL1(ADC0)|=ADC_CTL1_ADCON;
    while(!(ADC_STAT(ADC0)&ADC_STAT_EOC));
    uint16_t ret=ADC_RDATA(ADC0)&0xFFFF;
    ADC_STAT(ADC0)&=~ADC_STAT_EOC;
    return(ret);



}

// PWMの初期化（TIMER4 , GPIOA5) 
void timer_pwm_init(void) 
{ // PA1, no AF remapping needed
  	timer_oc_parameter_struct timer_ocinitpara;
	timer_parameter_struct timer_initpara;
	timer_deinit(TIMER4);
    timer_struct_para_init(&timer_initpara);


   	// rcu_periph_clock_enable(RCU_AF);
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3);
    rcu_periph_clock_enable(RCU_TIMER4);

    timer_initpara.prescaler         = 539;                 // 54MHz/540 で、およそ100KHzが得られるハズ。
															// ところが、なぜかここは200KHzになる。TIMER4は、54Mのバスにつながっているはずなのに。
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;  // count alignment edge = 0,1,2,3,0,1,2,3... center align = 0,1,2,3,2,1,0
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;    // Counter direction
    timer_initpara.period            = 3;		            // 200KHz/4 で、おおよそ50KHzが得られるハズ。ただ、これはSoundSetで動的に書き換えられちゃうけどね。
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;    // This is used by deadtime, and digital filtering (not used here though)
    timer_initpara.repetitioncounter = 0;                   // Runs continiously	
    timer_init(TIMER4, &timer_initpara);  					// Apply settings to timer   

 	timer_channel_output_struct_para_init(&timer_ocinitpara);
    timer_ocinitpara.outputstate  = TIMER_CCX_ENABLE;        // Channel enable
    timer_ocinitpara.outputnstate = TIMER_CCXN_DISABLE;      // Disable complementary channel
    timer_ocinitpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;  // Active state is high
    timer_ocinitpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;  
    timer_ocinitpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW; // Idle state is low
    timer_ocinitpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;
    timer_channel_output_config(TIMER4, TIMER_CH_3,&timer_ocinitpara);   // Apply settings to channel

    timer_channel_output_pulse_value_config(TIMER4,TIMER_CH_3,0);                   // Set pulse width
    timer_channel_output_mode_config(TIMER4,TIMER_CH_3,TIMER_OC_MODE_PWM0);         // Set pwm-mode
    timer_channel_output_shadow_config(TIMER4,TIMER_CH_3,TIMER_OC_SHADOW_DISABLE);

    /* auto-reload preload enable */
    timer_auto_reload_shadow_enable(TIMER4);

    /* start the timer */
    timer_enable(TIMER4);
}