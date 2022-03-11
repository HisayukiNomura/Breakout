#include "lcd/lcd.h"
#include "led.h"
#include "memory.h"
#include "ioinit.h"
#include "sound.h"
#include "breakout.h"
#include "breakoutButton.h"
#include "ball.h"
#include "block.h"
#include "paddle.h"

unsigned int ForeColor=WHITE;
unsigned int BackColor=BLACK;

// ステージ
unsigned char Stage=1;




int Score;				// スコアとハイスコア
int HiScore;

int LifeCnt;			// ライフ数

enum GAMESTATE gameState;	//　ゲームの状態
enum INGAMESTATE inGameState;	// 

enum GAMEMODE  gameMode;	// 1... demo / 2... easy / 3... hard
unsigned char isDemoText=0;			// 1... 文字出ている

// 
void BallSound(u8 Mode , u8 SndNo);


bool isDemo()
{ 
	return (gameMode == GAMEMODE_DEMO || gameMode == GAMEMODE_AUTODEMO);
}


void Init()
{
	InitBallPos(0,NULL);
	InitPaddle();

	Score = 0;
}
void DrawBORDER()
{
	LCD_DrawLine(GAMEAREA_X0,GAMEAREA_Y1,GAMEAREA_X0,GAMEAREA_Y0,WHITE);
	LCD_DrawLine(GAMEAREA_X0,GAMEAREA_Y0,GAMEAREA_X1,GAMEAREA_Y0,WHITE);
	LCD_DrawLine(GAMEAREA_X1,GAMEAREA_Y0,GAMEAREA_X1,GAMEAREA_Y1,WHITE);

	LCD_ShowString(0,0,(const u8 *)"SCORE:",WHITE);
	LCD_ShowString(10,160-12,(const u8 *)"LIFE:",WHITE);
	char life[3];
	sprintf(life,"%1d",LifeCnt);
	LCD_ShowString(45,160-12,(u8 *)life,WHITE);
	u8 scr[12];
	sprintf((char *)scr,"%5d0",Score);
	LCD_ShowString(38,0,scr,WHITE);	
}





// 座標が、矩形の外側に対して、どの象限にいるのかを返す関数
//    1         2        3
//        +---------+
//   4    |    5    |    6
//        +---------+
//   7         8         9    
unsigned char  GetOrthant(int x , int y , int x1, int y1 , int x2 , int y2)
{
	bool bLowerX1 =  (x < x1);
	bool bUpperX2 =  (x > x2);
	bool bLowerY1 =  (y < y1);
	bool bUpperY2 =  (y > y2);

	if (bLowerX1) {
		if ( bLowerY1) {
			return 1;
		} else if  (bUpperY2) {
			return 7;
		} else {
			return 4;
		}
	} else if (bUpperX2) {
		if (bLowerY1) {
			return 3;
		} else if  (bUpperY2) {
			return 9;
		} else {
			return 6;
		}
	} else {
		if (bLowerY1) {
			return 2;
		} else if  (bUpperY2) {
			return 8;
		} else {
			return 5;
		}
	}
}



volatile u8 WakeFlag = 0;
void TIMER5_IRQHandler(void)
{
	static u8 oeFlag;
    if(SET == timer_interrupt_flag_get(TIMER5, TIMER_INT_FLAG_UP)){
		//p1Button = 0;
        timer_interrupt_flag_clear(TIMER5, TIMER_INT_FLAG_UP);

        if (oeFlag == 0) {
            gpio_bit_reset(GPIOB, GPIO_PIN_8); //OE#
            oeFlag = 1;
        } else {
            gpio_bit_set(GPIOB, GPIO_PIN_8); //OE#
            oeFlag = 0;
        }
		WakeFlag = 1;
		CheckP1Button();
    }
}

void timer5_config(int Cnt)
{
	//タイマーの16ビットプリスケーラには54MHzが入っている
	// それを10000分周すれば5.4kHzが16ビットタイマ本体の入力クロックとなる
	
    timer_parameter_struct timer_initpara;
    rcu_periph_clock_enable(RCU_TIMER5);
    timer_deinit(TIMER5);
    timer_struct_para_init(&timer_initpara);
    timer_initpara.prescaler         = 10000 - 1;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = Cnt;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_init(TIMER5, &timer_initpara);
    timer_auto_reload_shadow_enable(TIMER5);
    timer_interrupt_enable(TIMER5, TIMER_INT_UP);

	// これはどこでやれば・・・
	eclic_global_interrupt_enable();
	eclic_set_nlbits(ECLIC_GROUP_LEVEL3_PRIO1);
	eclic_irq_enable(TIMER5_IRQn,1,0);
    timer_enable(TIMER5);
}


//  Mode == 0...初期化 , 1...処理 , 2...音を出す , 9... 終了
//  Option = 音の番号。Modeが２の時だけ有効
void BallSound(u8 Mode , u8 SndNo)
{
	static bool bSound[2];
	static int iSoundIdx=0;
	static int iSoundStep[2][2];
	static int iSoundTim[2][2] = {{400,800},{1000,600}};		//200Hzに対して、これで分週する。400なら、500Hzになる。
	
	if (isSound == FALSE) return;				// サウンドなしなら何もしない
	if (Mode == 0) {
		for (u8 i = 0;i<2;i++) {
			iSoundIdx = 0;
			bSound[i] = FALSE;
			iSoundStep[i][0] = iSoundStep[i][1] = 0;
		}
		timer_disable(TIMER4);			
	} else if (Mode == 1) {
		for (u8 i = 0 ; i < 2;i++){
			if (bSound[i]) {
				if (iSoundStep[i][iSoundIdx] == 0) {		// 音のなり始め
					SoundSet(iSoundTim[i][iSoundIdx]);
					timer_enable(TIMER4);
					iSoundStep[i][iSoundIdx]++;					
				} else if (iSoundStep[i][iSoundIdx]<3) {
					iSoundStep[i][iSoundIdx]++;
				} else if (iSoundStep[i][iSoundIdx] == 3) {	// 次の音へ
					if (iSoundIdx == 1) {			//　全部の音を出し終わったら終了
						BallSound(9,0);
					} else {
						iSoundStep[i][iSoundIdx] = INT16_MAX;
						iSoundIdx++;
					}
				}
			}
		}
	} else if (Mode ==2)  {
		BallSound(9,0);
		bSound[SndNo] = TRUE;
	}else if (Mode == 9) {
		for (u8 i = 0;i<2;i++) {
			iSoundIdx = 0;
			bSound[i] = FALSE;
			iSoundStep[i][0] = iSoundStep[i][1] = 0;
		}
		timer_disable(TIMER4);			
	}
}

//ボールの位置を初期化して、コートやブロックを書き直す
void breakout_GameInit(void) 
{
	InitBallPos(0,NULL);
	LCD_Clear(BackColor);
	DrawBORDER();						
	DrawBlock();
}



void breakout(enum GAMEMODE a_isDemo) 
{

	led_init();
	static  u16 waitCounter = -1;			// 途中で状態をキープする必要がある場合のカウンタ
	static  u16 HardbeatCounter  = 0;
	static  u32 longCounter = 0;

	gameState = STATE_INITGAME;
	gameMode = a_isDemo;

	// 効果音関連の初期化
	bool bSound[2];
	int iSoundIdx=0;
	int iSoundStep[2][2];
	int iSoundTim[2][2] = {{40,80},{80,40}};
	
	for (u8 i = 0;i<2;i++) {
		bSound[i] = FALSE;
		iSoundStep[i][0] = iSoundStep[i][1] = 0;
	}


	Stage=1;
	/*
	while(1) {
		timer_channel_output_pulse_value_config(TIMER4,TIMER_CH_3,2);
		SoundSet(40);
        delay_1ms(5000);

		)
	*/

	BallSound(0,0);



	if (isDemo()) {
		timer5_config(30);
	}
	if (gameMode ==  GAMEMODE_EASY) {
		timer5_config(50);
	} else if (gameMode == GAMEMODE_HARD) {
		timer5_config(20);
	} else {
		timer5_config(30);
	}

	//while(1) {	
	while (gameState != STATE_TOPMENU) {
		WakeFlag = 0;
		timer_enable(TIMER5);
		while(WakeFlag == 0) {
			if (WakeFlag == 1) {
				break;
			}	
			delay_1ms(1);		// for debugger
			TIMER5_IRQHandler();
		}
		timer_disable(TIMER5);
		HardbeatCounter = ((HardbeatCounter+1) & 0x3FF);
		longCounter = (longCounter+1) & 0xFFFFFFFF;
		BallSound(1,0);
		if (gameMode == GAMEMODE_AUTODEMO && longCounter == 0x3000) {
			gameState = STATE_TOPMENU;
		}
		// 音の処理
		for (u8 i = 0 ; i < 2;i++){
			if (bSound[i]) {
				if (iSoundStep[i][iSoundIdx] == 0) {		// 音のなり始め
					SoundSet(iSoundTim[i][iSoundIdx]);
					timer_enable(TIMER4);
				} else if (iSoundStep[i][iSoundIdx]<100) {
					iSoundStep[i][iSoundIdx]++;
				} else if (iSoundStep[i][iSoundIdx] == 100) {	// 次の音へ
					if (iSoundIdx == 1) {
						iSoundStep[i][0] = iSoundStep[i][1] = 0;
						bSound[i] = FALSE;
						timer_disable(TIMER4);
					} else {
						iSoundStep[i][iSoundIdx] = INT16_MAX;
						iSoundIdx++;
					}
				}
			}
		}

		switch (gameState) {
			case STATE_INITGAME: {
				Init();
				LCD_Clear(BackColor);
				BACK_COLOR=BLACK;
				LifeCnt  = 3;
				breakout_GameInit();
				gameState = STATE_STARTGAME;
				break;
			}
			case STATE_STARTGAME: {
				DrawBORDER();
				InitBlock();
				DrawBlock();
				gameState = STATE_INGAME;
				inGameState = STATE_INITBALL;
				break;
			}
			case STATE_INGAME:{
				switch(inGameState) {
					case STATE_INITBALL: {
						breakout_GameInit();
						inGameState = STATE_BALLCONTROL;						
						break;
					}
					case STATE_BALLCONTROL:{
						u8 ret = breakout_BallCtrl();
						if (ret == 0) {	// ミス。すべてのボールがなくなった
							LifeCnt--;
							if (LifeCnt == 0) {
								gameState = STATE_GAMEOVER;		// ライフがゼロならゲームオーバー
							} else {
								gameState = STATE_BALLMISS;// ボールを初期化して継続
							}
						} else if (ret == 2) {
							gameState = STATE_NEXTSTAGE;
						} 
						inGameState = STATE_PADDLECONTROL;
						break;
					}
					case STATE_PADDLECONTROL:{
						breakout_PaddleCtrl();
						inGameState = STATE_STATEUPDATE;
						break;
					}
					case STATE_STATEUPDATE:{

						inGameState = STATE_BALLCONTROL;
						break;
					}
					case STATE_NONE: {
						break;		// ここには来ないはず
					}
				}
				if (isDemo()) {
					if (p1ButtonPushed == SET) {
						LCD_Clear(BACK_COLOR);
						gameState = STATE_TOPMENU;
						break;
					}
					if (HardbeatCounter == 400) {
						isDemoText = 1;
						LCD_ShowString(3,80,(u8 *)"PUSH BUTTON",WHITE);							
					} else if (HardbeatCounter == 800) {
						isDemoText = 0;
						LCD_Fill(3,80,75,80+24,BACK_COLOR);
					}
				}				
				break;
			}
			case STATE_BALLMISS:{
				if (waitCounter == -1) {
					waitCounter = 0;
					LCD_Fill(3,80,78,80+24,BACK_COLOR);
					LCD_ShowString(10,80,(u8 *)"- MISS!! -",WHITE);
					SoundPlay("B1800",10);
				} else {
					waitCounter++;
					if (waitCounter >= 1000) {
						waitCounter = -1;
						gameState = STATE_INGAME;
						inGameState = STATE_INITBALL;
					}
				}
				break;
			}
			case STATE_GAMEOVER:{
				if (waitCounter == -1) {
					waitCounter = 0;
					LCD_Fill(3,80,78,80+24,BACK_COLOR);
					LCD_ShowString(10,80,(u8 *)"GAME OVER",WHITE);
					SoundPlay("B1400 B1300 B1100 B1400 D2300 c2100 c2300B1100 B1300 a1100 B1400",10);						
				} else {
					waitCounter++;
					if (waitCounter >= 20) {
						gameState = STATE_TOPMENU;			
						waitCounter = -1;
						break;
					}
				}
				break;
			}
			case STATE_NEXTSTAGE:{
				if (waitCounter == -1) {
					waitCounter = 0;
					LCD_Fill(3,80,78,80+24,BACK_COLOR);
					LCD_ShowString(10,80,(u8 *)"- GREAT!! -",WHITE);	
					SoundPlay("E3200 E3200 F3200 G3200 G3200 F3200 E3200 D3200 C3200 C3200 D3200 E3200 D3300 C3100 C3400",10);	
				} else {	
					waitCounter++;								
					if (waitCounter >= 3000) {
						waitCounter = -1;
						Stage++;
						gameState  = STATE_STARTGAME;						
					}
				}
				break;
			}
			case STATE_TOPMENU:{
				break;
			}
			case STATE_NONE:{
				break;
			}

		}
	}
	timer_disable(TIMER5);
	BallSound(9,0);
}

