#include "lcd/lcd.h"
#include "led.h"
#include "memory.h"
#include "ioinit.h"
#include "sound.h"
#include "breakout.h"
#include "breakoutButton.h"
#include "ball.h"
#include "block.h"


unsigned int BLOCK_COLOR_NORMAL=WHITE;
unsigned int BLOCK_COLOR_INCBALL= BLUE;


u32 Speed =5;

#define BLOCK_CNT_H	7
#define BLOCK_CNT_V 21
#define BLOCK_SIZE_W 11
#define BLOCK_SIZE_H 6




// ブロックのテーブル
struct BLOCKINFO {
	u8 item;
	u8 x1;
	u8 y1;
	u8 x2;
	u8 y2;
} blockmtx[BLOCK_CNT_H][BLOCK_CNT_V];
int blkCnt = 0;			// 総ブロック数
int blkBrk = 0;			// 残りブロック数

// 調整用
unsigned char nonBlockHit;			// ブロックを壊さないでパドルに当たった数。これが多すぎるとイレギュラーを増やす。

void AddBlock(int x,int y,u8 BlkType)
{
	blockmtx[x][y].item = BlkType;
	blockmtx[x][y].x1 = x * BLOCK_SIZE_W + (GAMEAREA_X0 + 2);
	blockmtx[x][y].y1 = y * BLOCK_SIZE_H + (GAMEAREA_Y0 + 2);
	blockmtx[x][y].x2 = blockmtx[x][y].x1 + BLOCK_SIZE_W  - 2 ;
	blockmtx[x][y].y2 = blockmtx[x][y].y1 + BLOCK_SIZE_H  - 3 ;
}

#define STAGE_DEFINED 6
static char blkinfo[STAGE_DEFINED][(BLOCK_CNT_H+1)*BLOCK_CNT_V] = 
{ // ---1--- ---2--- ---3--- ---4--- ---5--- ---6--- ---7--- ---8--- ---9--- --10--- ---11-- ---12-- ---13-- ---14-- ---15-- ---16-- ---17-- ---18-- ---19-- ---20--- --21---
	"0000000:1111111:1211121:1111111:1111111:1111111:0000000:0000000:0000000:0000000:0000000:0000000:0000000:0000000:0000000:0000000:0000000:0000000:0000000:0000000:0000000",
	"0000000:0000000:1111111:1211121:3111113:1111111:1111111:0000000:0000000:0000000:0000000:0000000:0000000:0000000:0000000:0000000:0000000:0000000:0000000:0000000:0000000",
	"0000000:0000000:0000000:1111111:1211121:3111113:1131311:1111111:0000000:0000000:0000000:0000000:0000000:0000000:0000000:0000000:0000000:0000000:0000000:0000000:0000000",
	"0000000:0000000:0000000:0000000:1111111:1211121:3141413:1131311:1111111:0000000:0000000:0000000:0000000:0000000:0000000:0000000:0000000:0000000:0000000:0000000:0000000",
	"0000000:0000000:0000000:0000000:0000000:1111111:1211121:3141413:1131311:3111113:0000000:0000000:0000000:0000000:0000000:0000000:0000000:0000000:0000000:0000000:0000000",
	"0000000:0000000:0000000:0000000:0000000:0000000:1111111:1211121:3141413:1131311:3111113:0000000:0000000:0000000:0000000:0000000:0000000:0000000:0000000:0000000:0000000",
};


void InitBlock()
{
	memset((void *)blockmtx,0,sizeof(blockmtx));

	u8 stgWk = (Stage -1) % STAGE_DEFINED;

	char* pBlock = blkinfo[stgWk];
	blkCnt = 0;
	blkBrk = 0;
	
	for (u8 i=0;i< BLOCK_CNT_V;i++) {
		for (u8 j=0; j< BLOCK_CNT_H;j++) {
			if (*pBlock == '0') {
				// 何もなし
			} else if (*pBlock == '1') {
				AddBlock(j,i,1);
				blkCnt++;
			} else if (*pBlock == '2') {
				AddBlock(j,i,2);
				blkCnt++;
			} else if (*pBlock == '3') {
				AddBlock(j,i,3);
				blkCnt++;
			} else if (*pBlock == '4') {
				AddBlock(j,i,4);
				blkCnt++;
			} else {
				// 今のところ何もなし
			}
			pBlock++;
		}
		pBlock++;	// : を読み飛ばす
	}
	blkBrk = blkCnt;
	nonBlockHit = 0;
}
// ブロックを描画する
void DrawBlock()
{
	static u16 colTbl[]  = {RED,  BLUE,     GREEN     ,MAGENTA,CYAN,     YELLOW};

	for (u8 i = 0 ; i< BLOCK_CNT_H;i++) {
		for (u8 j = 0; j<BLOCK_CNT_V;j++) {
			u16 col = colTbl[j % 6];
			if (blockmtx[i][j].item == 1) {
				LCD_Fill(blockmtx[i][j].x1,blockmtx[i][j].y1,blockmtx[i][j].x2,blockmtx[i][j].y2,col);
			} else if (blockmtx[i][j].item == 2) {
				LCD_Fill(blockmtx[i][j].x1,blockmtx[i][j].y1,blockmtx[i][j].x2,blockmtx[i][j].y2,col);
				LCD_Fill(blockmtx[i][j].x1+1,blockmtx[i][j].y1+1,blockmtx[i][j].x2-1,blockmtx[i][j].y2-1,BLACK);
			} else if (blockmtx[i][j].item == 3) {
				LCD_Fill(blockmtx[i][j].x1,blockmtx[i][j].y1,blockmtx[i][j].x2,blockmtx[i][j].y2,col);
				LCD_Fill(blockmtx[i][j].x1+1,blockmtx[i][j].y1+1,blockmtx[i][j].x2-1,blockmtx[i][j].y2-1,BROWN);
			} else if (blockmtx[i][j].item == 4) {
				LCD_Fill(blockmtx[i][j].x1,blockmtx[i][j].y1,blockmtx[i][j].x2,blockmtx[i][j].y2,col);
				LCD_Fill(blockmtx[i][j].x1+1,blockmtx[i][j].y1+1,blockmtx[i][j].x2-1,blockmtx[i][j].y2-1,WHITE);
			} else {
				LCD_Fill(blockmtx[i][j].x1,blockmtx[i][j].y1,blockmtx[i][j].x2,blockmtx[i][j].y2,BACK_COLOR);
			}
		}
	}
	

}

//
// ブロック反射チェック
//　この時点では、BALLINFOの座標は移動済みの座標になっている。
//
void blockCheck(struct BALLINFO* bi)
{
	int chkx = CVT_AXIS(bi->oldx);
	int chky = CVT_AXIS(bi->oldy);
	// 座標がある位置のブロック番号を求める
	int xNow = CVT_AXIS(bi->x);
	int yNow = CVT_AXIS(bi->y);
	int xidx = (xNow - (GAMEAREA_X0+2)) /BLOCK_SIZE_W;
	int yidx = (yNow - (GAMEAREA_Y0+2)) / BLOCK_SIZE_H;
	


	//　求めたブロック番号にブロックが存在するなら、そのブロックを消して、ボールを跳ね返す。
	if (blockmtx[xidx][yidx].item != 0 && GetOrthant(xNow,yNow, blockmtx[xidx][yidx].x1 ,blockmtx[xidx][yidx].y1,blockmtx[xidx][yidx].x2,blockmtx[xidx][yidx].y2) == 5) {
		blkBrk--;							// 残ブロック数を１つ減らす
		nonBlockHit = 0;					// ブロックに当たらなかったターン数をリセットする
		// ボールの新しい位置は、ブロックの内側なので、座標はひとつ前の位置に戻さないといけない。
		BallBack(bi);

		// 難易度調整
		if (blkBrk <= (blkCnt / 2)) {				// 残ブロックスが全ブロック数の半分以下になったら
			bi->SpeedMask  |= SPDMSK_BLOCKCNT_1;	// スピードレベル１
		} 
		if (blkBrk <= (blkCnt / 4)) {				// 残ブロックスが全ブロック数の1/4分以下になったら
			bi->SpeedMask |= SPDMSK_BLOCKCNT_2;		// スピードレベル２
		}

		// ボールを跳ね返す。
		u8 pos = GetOrthant(chkx,chky, blockmtx[xidx][yidx].x1 ,blockmtx[xidx][yidx].y1,blockmtx[xidx][yidx].x2,blockmtx[xidx][yidx].y2);
		if (pos == 2 || pos== 8) {
			bi->dyBase = -bi->dyBase;
		} else if (pos == 4 || pos == 6) {
			bi->dxBase = -bi->dxBase;
		} else {
			bi->dxBase = -bi->dxBase ;
			bi->dyBase  = -bi->dyBase ;
		}


		// 特殊ブロックの処理
		if (blockmtx[xidx][yidx].item == 2) {			// ボールが増える
			InitBallPos(1,bi);
		} else if (blockmtx[xidx][yidx].item == 3)  {	// 角度が増える
			bi->dxBase = bi->dxBase + ((bi->dxBase > 0) ? 1 : -1);
		} else if (blockmtx[xidx][yidx].item == 4)  { // ボールが消える
			bi->iInvisible = INVISIBLE_CNT;						// 100フレームの間ボールが消える
		}
		// ブロックは消す
		blockmtx[xidx][yidx].item = 0;	 
		LCD_Fill(blockmtx[xidx][yidx].x1,blockmtx[xidx][yidx].y1,blockmtx[xidx][yidx].x2,blockmtx[xidx][yidx].y2,BackColor);

		// スコアの処理
		if (bi->SpeedMask & SPDMSK_BACKWALL) {		// 裏に入っていたらスコアは増量
			Score = Score + 2;
		} else {
			Score = Score + 1;
		}
		if (HiScore < Score) {
			HiScore = Score;
		}
		u8 scr[12];
		sprintf((char *)scr,"%5d0",Score);

		LCD_ShowString(38,0,scr,WHITE);
		BallSound(2,0);
	}
}
