#ifndef __ball_h__
#define __ball_h__

#define SPDMSK_BACKWALL 0x01
#define SPDMSK_BLOCKCNT_1 0x02
#define SPDMSK_BLOCKCNT_2 0x04

#define INVISIBLE_CNT	500

struct BALLINFO {
	int oldx;	// 8倍座標
	int oldy;	// 8倍座標
	int x;		// 8倍座標
	int y;		// 8倍座標
	int dx;
	int dy;
	int dxBase;
	int dyBase;
	unsigned char  SpeedMask;
	int iInvisible; 
};
extern unsigned char ballLive;

void BallStep(struct BALLINFO *bi);
void BallBack(struct BALLINFO *bi);
void BallDead(struct BALLINFO *bi);

void InitBallPos(unsigned char mode, struct BALLINFO *bi);
void updateBallSpeed(struct BALLINFO *bi);
struct BALLINFO* GetBallInfo(unsigned int idx);
int breakout_BallCtrl(void);


unsigned char moveBall();
#define CVT_AXIS(__x) ((__x) >> 3)
enum DRAWMODE {
	BALL_DRAW = 0,
	BALL_REMOVE = 1
};
void drawDeleteBall(enum DRAWMODE mode);

#endif