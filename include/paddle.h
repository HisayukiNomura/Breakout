#ifndef __paddle_h__
#define __paddle_h__
struct PADDLEINFO {
	unsigned char x1;
	unsigned char  y1;
	unsigned char  x2;
	unsigned char  y2;
	unsigned char  prevx1;
	unsigned char  prevx2;
	unsigned char  Width;
};

struct PADDLEINFO* GetPaddleInfo(int idx);
void InitPaddle(void);
void breakout_PaddleCtrl(void);
#endif