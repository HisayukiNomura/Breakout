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

// パドル
struct PADDLEINFO Paddle[2];

struct PADDLEINFO* GetPaddleInfo(int idx)
{
    if (Paddle[idx].y1 == 0) return NULL;
    return &Paddle[idx];
}

void InitPaddle(void)
{
    memset(Paddle,0,sizeof(Paddle));

	Paddle[0].Width = 20;
	Paddle[0].x1 = (GAMEAREA_X1 /2) - Paddle[0].Width /2 ;
	Paddle[0].y1 = GAMEAREA_Y1  - 15;
	Paddle[0].x2 = (GAMEAREA_X1 /2) + Paddle[0].Width /2 ;
	Paddle[0].y2 = GAMEAREA_Y1  - 10;

}
void drawPaddle()
{
	for (u8 i = 0 ; i < 2;i++) {
		struct PADDLEINFO* pi = &Paddle[i];
		if (pi->y1 == 0) continue;
		LCD_Fill(pi->x1,pi->y1,pi->x2,pi->y2, ForeColor);		
		if (pi->prevx1 >= (GAMEAREA_X0+1) && pi->x1 > pi->prevx1) {
			LCD_Fill(pi->prevx1,pi->y1,pi->x1,pi->y2,BackColor);		
		} else if (pi->x1 < pi->prevx1) {
			LCD_Fill(pi->x2,pi->y1,pi->prevx2,pi->y2,BackColor);		
		} 
	}
}
void setPaddle(int pdlId , int cx) {
	u8 padLimit = 0;

	Paddle[pdlId].prevx1 = Paddle[pdlId].x1;
	Paddle[pdlId].prevx2 = Paddle[pdlId].x2;

	u8 cxMin = GAMEAREA_X0 + (Paddle[pdlId].Width  / 2);
	u8 cxMax = GAMEAREA_X1 - (Paddle[pdlId].Width  / 2);
	if (cx >= cxMax) cx = cxMax;
	if (cx <= cxMin) cx = cxMin;
	
	Paddle[pdlId].x1 = cx - Paddle[pdlId].Width  / 2;
	Paddle[pdlId].x2 = cx + Paddle[pdlId].Width  / 2;


    // ボールが壁とパドルに挟まれないように、ボールの位置によってパドルの動きを制限する
	for (u8 i = 0 ; i < 5;i++) {
		struct BALLINFO *bi = GetBallInfo(i);
		if (bi == NULL) continue;

        // パドルのy座標とボールのｙ座標を比べ、横並びになっていたら
		if ( Paddle[pdlId].y1 <= CVT_AXIS(bi->y) && CVT_AXIS(bi->y) <= Paddle[pdlId].y2) {
			// ボールがパドルと横並び
			if (CVT_AXIS(bi->x) <=4 && Paddle[pdlId].x1 <=4) {			//ボールが左端４ドット内にいる場合
	        	Paddle[pdlId].x1 = 4;
		        Paddle[pdlId].x2 = Paddle[pdlId].x1 + Paddle[pdlId].Width;
			} else if (CVT_AXIS(bi->x) >= (GAMEAREA_X1 - 4) && Paddle[pdlId].x2 >= (GAMEAREA_X1 - 4)) {
		    	Paddle[pdlId].x2 =  (GAMEAREA_X1 - 4);
    			Paddle[pdlId].x1 = Paddle[pdlId].x2 - Paddle[pdlId].Width;
			}
		}
	}
}


void demoPaddleMove(){
	for (u8 i = 0 ; i < 2;i++) {
		struct PADDLEINFO* pi = &Paddle[i];
		if (pi->y1 == 0) continue;
		u8 cx = pi->x1 + (pi->Width  / 2);
        //追いかけるべきボールを探す
        // ボールの実質的な距離をベースにする
        int distMax = INT16_MAX;
        int ballIdx = -1;
        for (int j =0;j <5; j++) {
			struct BALLINFO* bi = GetBallInfo(j);
			if (bi == NULL) continue;
            int dist;
            if (bi->dy < 0) {       // ボールが遠ざかっているなら
                dist = CVT_AXIS(bi->y) + pi->y1;
            }  else {               // ボールが近づいているなら
                dist =  CVT_AXIS(pi->y1) - bi->y;
            }
            if (dist < distMax) {
                ballIdx = j;
                distMax = dist;
            }
    	}
        struct BALLINFO* bi = GetBallInfo(ballIdx);
    	if (CVT_AXIS(bi->x) > cx) {
    		setPaddle(i,cx+1);
		} else if (CVT_AXIS(bi->x) < cx) {
    		setPaddle(i,cx-1);
	    }
	}
	/*
	for (u8 i = 0 ; i < 2;i++) {
		struct PADDLEINFO* pi = &Paddle[i];
		if (pi->y1 == 0) continue;
		u8 cx = pi->x1 + (pi->Width  / 2);
		for (int j =0;j <5; j++) {
			struct BALLINFO* bi = GetBallInfo(j);
			if (bi == NULL) continue;
			if (CVT_AXIS(bi->x) > cx) {
				setPaddle(i,cx+1);
			} else if (CVT_AXIS(bi->x) < cx) {
				setPaddle(i,cx-1);
			}
			

		}
	}
	*/
}
void PaddleMove(u16 padPos)
{
	for (u8 i = 0 ; i < 2;i++) {
		struct PADDLEINFO* pi = &Paddle[i];
		if (pi->y1 == 0) continue;

		u8 newpadx = (GAMEAREA_X1 - GAMEAREA_X0) * padPos / 1024;
		setPaddle(i,newpadx);
		
	}
}
// パドルを動かす
void breakout_PaddleCtrl(void)
{
	static u8 tick =0 ;
	if (isDemo()  && (tick % 1 == 0)) {
		demoPaddleMove();
		if (tick > 200) tick = 0;		
	} else {
		u16 padPos = getPaddlePos();
		PaddleMove(padPos);
	}

	drawPaddle();	
	tick++;

}