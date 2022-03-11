#ifndef __SOUND_H__
#define __SOUND_H__
void StartSound(void);
void StopSound(void);
void SoundSet(int Period);
void Beep(int ms);
void SoundPlay(char *Snd,int);
extern bool isSound;
#endif