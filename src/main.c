// https://github.com/takamame0205/LonganNanoLCD 
// から、fatfs と lcd、led.c systick.c をダウンロードしてプロジェクトに追加する
// lcdフォルダ以外は、Sipeedのサンプルと同じ
// lcd.hの#define LCD_AFONTと　LCD_KFONTを書き換える。
// lcd.hの FONT_WIDTH にフォントの半角横幅(12x12の日本語なら6)を追加				// 自動改行時の1行の高さ
// lcd.hの FONT_HEIGHTを使用するフォントに合わせて変更


#include "lcd/lcd.h"
#include "gd32vf103.h"
#include "fatfs/tf_card.h"
#include <string.h>
#include "breakout.h"
#include "led.h"
#include "ioinit.h"
#include "sound.h"


#define MAX_MODE   4


u8 GameMode;


u8 prevMode = -1;
u8 MenuText[][20] = {"ＤＥＭＯ" , "ＥＡＳＹ" , "ＨＡＲＤ","                 "};



void GS_draw(u8 a_GameMode)
{
	u16 c;
	if (prevMode == a_GameMode) {
		return;
	}
	Beep(10);
	prevMode = a_GameMode;
	strcpy((char *)MenuText[3],isSound ? "SOUND: ON":"SOUND :OFF");
	for (u8 i=0;i<MAX_MODE;i++) {
		
		if (a_GameMode == i) {
			BACK_COLOR = WHITE;
			c  = BLUE;
		} else {
			BACK_COLOR = BLUE;
			c = RED;
		}
		
		
		LCD_ShowString( FONT_WIDTH * 1, 60 + (FONT_HEIGHT + 10)*i,MenuText[i], c);
		LCD_ShowString( FONT_WIDTH * 1+1, 60 + (FONT_HEIGHT + 10)*i,MenuText[i], c);
	}
	/*

	if(s==0){BACK_COLOR=WHITE;c=BLUE;}else{BACK_COLOR=BLUE;c=RED;}
	LCD_ShowString( FONT_WIDTH * 1, FONT_HEIGHT *4, (const u8*)"Demo ", c);
	delay_1ms(10);
	if(s==1){BACK_COLOR=WHITE;c=BLUE;}else{BACK_COLOR=BLUE;c=RED;}
	LCD_ShowString( FONT_WIDTH * 1, FONT_HEIGHT *7, (u8*)"GAME A", c);
	delay_1ms(10);
	if(s==2){BACK_COLOR=WHITE;c=BLUE;}else{BACK_COLOR=BLUE;c=RED;}
	LCD_ShowString( FONT_WIDTH * 1, FONT_HEIGHT *10, (u8*)"GAME B", c);
	delay_1ms(10);
	*/
}
#include "logo.h"
void GameSelect(void)
{
//	int t=0;
	int cnt = 0;
	LCD_Clear(BLUE);
	BACK_COLOR=BLUE;
	LCD_Address_Set(0,0,79,44);
	for (int i = 0; i < 3600 ; i++) {
		LCD_WR_DATA(bmp[i]);
	}
	//LCD_ShowString(  FONT_WIDTH*0+3, FONT_HEIGHT*1, (u8*)"|ＢＲＥＡＫ|", WHITE);
	//LCD_ShowString(  FONT_WIDTH*0+3, FONT_HEIGHT*2, (u8*)"|　ＯＵＴ　|", WHITE);
	
	//SoundPlay("E3200 E3200 F3200 G3200 G3200 F3200 E3200 D3200 C3200 C3200 D3200 E3200 D3300 C3100 C3400",10);	
	//SoundPlay("B1400 B1300 B1100 B1400 D2300 c2100 c2300B1100 B1300 a1100 B1400",10);


	GS_draw(GameMode);
	
	
	do{
		printf("test");
		volatile u16 adcRes = Get_adc(0);
		adcRes = adcRes >> 7;			// 0～4096を0～32に丸める
		GameMode = adcRes / 10;
		GS_draw(GameMode);
		delay_1ms(10);
		cnt++;
		if (cnt >= 2000) {
			GameMode = 0xFF;
			break;
		} 
	}while(Get_P1Button());
	if (GameMode != 0xff)  {
		while(Get_P1Button());
		delay_1ms(1000);
	}
}



unsigned char image[12800];
int _put_char(int ch)
{
    usart_data_transmit(USART0, (uint8_t) ch );
    while ( usart_flag_get(USART0, USART_FLAG_TBE)== RESET){
    }
    return ch;
}
int main( void ) 
{
	// タイマーの許可
	//MODE_VERTICAL = 0;

	IO_init();	// I/O init
	
	rcu_periph_clock_enable(RCU_USART0);                // USART0にクロックを供給
	gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9); // USART0 TX
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10); // USART0 RX
	{	// USARTの最低限の設定。ここではTXを使って垂れ流しなので、特にバッファリングなどは不要
    	usart_deinit(USART0);
    	usart_baudrate_set(USART0, 115200U);
    	usart_word_length_set(USART0, USART_WL_8BIT);
    	usart_stop_bit_set(USART0, USART_STB_1BIT);
    	usart_parity_config(USART0, USART_PM_NONE);
    	usart_hardware_flow_rts_config(USART0, USART_RTS_DISABLE);
    	usart_hardware_flow_cts_config(USART0, USART_CTS_DISABLE);
    	usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    	usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    	usart_enable(USART0);
	}


	timer_pwm_init();
    timer_disable(TIMER4);
	LCD_Clear(BLACK);
	led_off(LED_B);	
	while(1){
		gameState = STATE_TOPMENU;
		
		GameSelect();
		LCD_Clear(BLACK);
		if (GameMode == 0xFF) {		// 時間切れデモ突入
			bool pushSnd = isSound;
			isSound = FALSE;		// オートデモの時は音を出さない
			breakout(GAMEMODE_AUTODEMO);
			isSound = pushSnd;
			prevMode = -1;
			continue;
		} else if (GameMode == 3) {	//　サウンドモード設定
			isSound = !isSound;
			prevMode = -1;
			continue;
		}

		breakout(GameMode);
		prevMode = -1;
	}
};	