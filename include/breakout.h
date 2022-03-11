#ifndef __brakeout_h__
#define __brakeout_h__
#include "ioinit.h"
enum GAMESTATE {
	STATE_INIT,			// 初期化処理中
	STATE_TOPMENU,		// メニュー画面
    STATE_INITGAME,    // 最初の初期化中
	STATE_STARTGAME,	// ゲーム開始（初期化中）
	STATE_INGAME,		// ゲーム中
	STATE_GAMEOVER,		// ゲームオーバー処理中
	STATE_BALLMISS,		// 新ボール
    STATE_NEXTSTAGE     // ステージクリア
};
enum INGAMESTATE {
	STATE_NONE,
	STATE_INITBALL,
	STATE_BALLCONTROL,
	STATE_PADDLECONTROL,
	STATE_STATEUPDATE,
};
enum  GAMEMODE {
	GAMEMODE_DEMO,
	GAMEMODE_EASY,
	GAMEMODE_HARD,
	GAMEMODE_AUTODEMO,		// 一定時間終了すると自動的に元に戻るデモ
};

#define GAMEAREA_X0   0
#define GAMEAREA_Y0   20
#define GAMEAREA_X1   79
#define GAMEAREA_Y1  148



extern enum GAMESTATE gameState;
extern enum INGAMESTATE inGameState;
extern void breakout(enum  GAMEMODE);
unsigned char  GetOrthant(int x , int y , int x1, int y1 , int x2 , int y2);

extern bool isSound;
extern unsigned char isDemoText;

extern int Score;
extern int HiScore;
extern unsigned char Stage;

extern unsigned int ForeColor;
extern unsigned int BackColor;

extern bool isDemo();
#endif