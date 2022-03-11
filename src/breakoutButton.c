#include "lcd/lcd.h"
#include "led.h"
#include "memory.h"
#include "ioinit.h"

FlagStatus p1ButtonPushed = RESET;
FlagStatus p2ButtonPushed = RESET;

// ボタンが押され続けた
static const uint32_t PortTBL[] = {GPIO_PIN_6,GPIO_PIN_7};
FlagStatus IsButtonPushed(int PlayerNo)
{
	static u8 Counter[2];

	FlagStatus  fr = gpio_input_bit_get (GPIOA,PortTBL[PlayerNo]);
	if (!fr) {
		if (Counter[PlayerNo] == 0xFF) Counter[PlayerNo]=0xFE;
		Counter[PlayerNo]++;
	} else {
		Counter[PlayerNo]=0;
	}
	if (Counter[PlayerNo] >= 50) {
		return SET;
	} 
	return RESET;
}
// ボタンが離され続けた
FlagStatus IsButtonReleased(int PlayerNo)
{
	static u8 Counter[2];
	FlagStatus  fr = gpio_input_bit_get (GPIOA,PortTBL[PlayerNo]);
	if (fr) {
		if (Counter[PlayerNo] == 0xFF) Counter[PlayerNo]=0xFE;
		Counter[PlayerNo]++;
	} else {
		Counter[PlayerNo]=0;
	}
	if (Counter[PlayerNo] >= 50) {
		return SET;
	} 
	return RESET;
}

// 
void CheckP1Button()
{
	p1ButtonPushed = RESET;			// ボタンの変化は一回だけトリガーされれば良い
	static FlagStatus isButton1 = RESET;
	if (isButton1 == RESET) {
		if (IsButtonPushed(0)) { //ボタンがOFF→ONに変化
			//led_off(LED_G | LED_R);
			//led_on(LED_G);
			isButton1 = SET; 								// ボタンが押された
		} else {
			//ボタンはOFFのまま。何もしない
			//led_off(LED_G | LED_R);
		}
	}  else {
		if (IsButtonReleased(0)) {//ボタンがON→OFFに変化
			//led_off(LED_G | LED_R);
			//led_on(LED_R);
			isButton1 = RESET;
			p1ButtonPushed = SET;
		}
	}
}

#define MAX_PADDLE_SPEED  100
u16 getPaddlePos()
{
	static u16 actualValue = -1;

	// 0～4096の値を、4で割って0～1024くらいにしておく。端っこのほうが怪しいから。
	short CurrentPos = Get_adc(0);
	CurrentPos = CurrentPos >> 2;
	if (actualValue == -1) {
		actualValue = CurrentPos;
	 } else {
		int dif = abs(actualValue - CurrentPos);
		if (dif < MAX_PADDLE_SPEED) {
			actualValue = CurrentPos;
		} else {
			if (actualValue > CurrentPos) {
				actualValue-= MAX_PADDLE_SPEED;
			} else {
				actualValue+= MAX_PADDLE_SPEED;
			} 
		}
	 }
	 return actualValue;
}