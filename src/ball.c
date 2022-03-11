#include "lcd/lcd.h"
#include "led.h"
#include "memory.h"
#include "ioinit.h"
#include "sound.h"
#include "breakout.h"
#include "breakoutButton.h"
#include "ball.h"
#include "block.h"
#include "wall.h"
#include "paddle.h"


struct BALLINFO ball[5];
struct BALLINFO* GetBallInfo(unsigned int idx)
{
    if (ball[idx].x == 0 && ball[idx].y == 0) return NULL;
    return &ball[idx];
}

unsigned char ballLive = 0;
/*
ボール構造体についての関数。C++で作ればよかった…
*/
// ボールを１つ進める
void BallStep(struct BALLINFO *bi) 
{
	bi->oldx = bi->x;
	bi->oldy = bi->y;
	bi->x += bi->dx;
	bi->y += bi->dy;
}
//ボールを1つ前の位置に戻す
void BallBack(struct BALLINFO *bi) 
{
	bi->x = bi->oldx;
	bi->y = bi->oldy;
}
void BallDead(struct BALLINFO *bi) 
{
	if (bi->x != 0) {
		ballLive--;
	}
	bi->oldx = 0;
	bi->oldy = 0;
	bi->x = 0;
	bi->y = 0;
}


#define BALL_DX_DEFAULT 1*2
#define BALL_DY_DEFAULT 2*2
#define BALL_DX_MIN  (1*2)
#define BALL_DX_MAX  (4*2)
#define BALL_DY_MIN  (1*2)
#define BALL_DY_MAX  (4*2)

//
// ボールを初期化する
//
void InitBallPos(unsigned char mode, struct BALLINFO *bi)
{
	if (mode ==  0) {		// 完全初期化して最初のボールを生きにする。 ballIdxは使われない
		memset(ball,0,sizeof(ball));
		ball[0].x = (GAMEAREA_X0 + GAMEAREA_X1/2)*8;
		ball[0].y = (GAMEAREA_Y0 + (GAMEAREA_Y1-GAMEAREA_Y0)/2)*8;
		ball[0].dxBase = BALL_DX_DEFAULT;
		ball[0].dyBase = BALL_DY_DEFAULT;
		ball[0].iInvisible = 0;
		updateBallSpeed(&ball[0]);
		ballLive=1;	
	} else if (mode == 1)  { // ballIdxの位置を元にしてボールを１つ追加する。　ボールの速度・角度は元のボールと変える。
		for (int j = 1; j < 5;j++ ) {
			if (ball[j].x == 0) {			// このボールで行こう
				ball[j].x = bi->x;
				ball[j].y = bi->y;
				ball[j].dyBase = BALL_DY_DEFAULT;
				ball[j].dx = -bi->dx;
				ball[j].dxBase = -bi->dxBase;
				ball[j].SpeedMask = 0;
				ball[j].iInvisible = 0;
				updateBallSpeed(&ball[j]);
				ballLive++;
				break;
			}
		}
	}
}


void updateBallSpeed(struct BALLINFO *bi)
{
	int speedlvl = 1;
	if (bi->SpeedMask & SPDMSK_BACKWALL) speedlvl += 1; 
	if (bi->SpeedMask & SPDMSK_BLOCKCNT_1) speedlvl += 1;
	if (bi->SpeedMask & SPDMSK_BLOCKCNT_2) speedlvl += 1;
	bi->dx = bi->dxBase * speedlvl;
	bi->dy = bi->dyBase * speedlvl;
}

// ボールを描画/削除する
void drawDeleteBall(enum DRAWMODE mode)
{
	for (u8 i = 0 ; i < 5;i++) {
		struct BALLINFO *bi = &ball[i];
		if(bi->x !=0) {
			int yVal = CVT_AXIS(bi->y);
			if (isDemo() && yVal > 80 &&yVal < 100 && isDemoText == 1) {
				// 何もしない
			} else 	{
				/*
				char buf[100];
				sprintf(buf, "%d\r\n", bi->iInvisible);
				LCD_ShowString(0,0,buf,WHITE);
				*/
				u16 c;
				if (mode == BALL_REMOVE) {
					c = BackColor;
				} else {
					if (bi->iInvisible > 0) {
						if (bi->iInvisible > (INVISIBLE_CNT - INVISIBLE_CNT/5) || bi->iInvisible < (INVISIBLE_CNT/5)) {
							led_off(LED_R);
							led_on(LED_G);
							if ((bi->iInvisible % 20) >=10) {
								c = BackColor;
							} else {
								c = ForeColor;
							}
						} else {
							led_on(LED_R);
							c = BackColor;
						}
						bi->iInvisible--;
					} else {
						bi->iInvisible = 0;
						led_off(LED_R|LED_G);
						c = ForeColor;
					}
				}
				LCD_Fill(CVT_AXIS(bi->x)-1,CVT_AXIS(bi->y)-1,CVT_AXIS(bi->x)+1,CVT_AXIS(bi->y)+1,c);
			}
		}
	}
}


void CheckPaddle(struct BALLINFO *bi)
{
			int chkx = CVT_AXIS(bi->x);
			int chky = CVT_AXIS(bi->y);
			for (int j=0 ; j<2;j++) {
				struct PADDLEINFO* pi = GetPaddleInfo(j);
				if (pi == NULL) continue;
				u8 pos = GetOrthant(chkx,chky,pi->x1,pi->y1,pi->x2,pi->y2);
				if (pos == 5) {
					nonBlockHit++;

					//ひとつ前のボール座標が、パドルのどこにあったかを求める。
					chkx = CVT_AXIS(bi->oldx);
					chky = CVT_AXIS(bi->oldy);
					u8 prevpos = GetOrthant(chkx,chky,pi->x1,pi->y1,pi->x2,pi->y2);

					//　ボールの新しい位置は、パドルの内側なので、ボールの座標をもとに戻さないといけない
					BallBack(bi);
					
					//　パドルにボールが反射する処理
					if (prevpos == 2 || prevpos == 8) {     // パドルの長径に当たった場合、y座標を反転
						bi->dyBase = -bi->dyBase;
						u8 pdlcx = pi->x1  + pi->Width/2;
						u8 ballpdldif = abs(chkx - pdlcx);
						
						if (ballpdldif > ( pi->Width/2) / 3) {	// 端だったら角度を増やす
							u8 dxwk = abs(bi->dxBase);			// 現在の角度の絶対値
							int cx = pi->x1 + pi->Width / 2;	// パドルの中央位置
							int ballX = CVT_AXIS(bi->x);		// ボールの位置

							if (bi->dxBase > 0) {					// ボールは右に移動中
								if (ballX > cx) {					// ボールはパドルの右側に当たった
									bi->dxBase++;
								} else {							// ボールはパドルの左側に当たった
									bi->dxBase--;
								}
							} else {							// ボールは左に移動中
								if (ballX > cx) {					// ボールはパドルの右側に当たった
									bi->dxBase--;
								} else {							// ボールはパドルの左側に当たった
									bi->dxBase++;
								}
							}
							if (abs(bi->dxBase) < BALL_DX_MIN) {
								bi->dxBase = bi->dxBase > 0 ? BALL_DX_MIN : -BALL_DX_MIN;
							} else if (abs(bi->dxBase) > BALL_DX_MAX) {
								bi->dxBase = bi->dxBase > 0 ? BALL_DX_MAX : -BALL_DX_MAX;
							}
						} 
					} else if (prevpos == 4 || prevpos == 6) {
							//　x反転
							bi->dxBase = -bi->dxBase;
					
					} else  {
						bi->dyBase = -bi->dyBase;
						bi->dxBase = -bi->dxBase;						
					} 
					// 千日手チェック
					if (nonBlockHit > 8) {
						bi->dxBase = bi->dxBase > 0 ? (rand() % (BALL_DX_MAX - BALL_DX_MIN) + BALL_DX_MIN) : -(rand() % (BALL_DX_MAX - BALL_DX_MIN) + BALL_DX_MIN);
					}
					BallSound(2,1);
				}
			}
}
// 0... ミス
// 1... 継続
// 2... クリア
unsigned char moveBall()
{
	for (u8 i = 0 ; i < 5;i++) {
		struct BALLINFO *bi = &ball[i];		
		if (bi->x == 0) continue;

		//ボールを動かす
		BallStep(bi);
		// 壁反射チェック
		{
			u8 ret = checkWall(bi);
			if (ret == 0) {
				return 0;
			} else if (ret == 2) {
				continue;
			}
		}


		//ブロック反射チェック
		// ボールの進行方向の隅がブロックに接しているかを調べる
		blockCheck(bi);
		if (blkBrk == 0) {
			return 2;
		}
		// パドル反射チェック
		CheckPaddle(bi);
		updateBallSpeed(bi);
	}
	return 1;
}

// 0...ボールミス
// 1...そのまま継続
// 2...全ブロック破壊
int breakout_BallCtrl(void) 
{
	u8 ret;
	drawDeleteBall(BALL_REMOVE);			// ひとつ前のボールを消す
	ret= moveBall();
	drawDeleteBall(BALL_DRAW);
	return ret;
}

