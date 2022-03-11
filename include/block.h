#ifndef __block_h__
#define __block_h__

extern int blkCnt;			// 総ブロック数
extern int blkBrk;			// 残りブロック数
extern unsigned char nonBlockHit;

void InitBlock();
void blockCheck(struct BALLINFO* bi);
void DrawBlock();
#endif