#ifndef __FMOPL_H_
#define __FMOPL_H_

#include <QtGlobal>

#define OPL_SAMPLE_BITS 16
typedef qint16 OPLSAMPLE;

typedef void (*OPL_TIMERHANDLER)(int channel,double interval_Sec);
typedef void (*OPL_IRQHANDLER)(int param,int irq);
typedef void (*OPL_UPDATEHANDLER)(int param,int min_interval_us);
typedef void (*OPL_PORTHANDLER_W)(int param,unsigned char data);
typedef unsigned char (*OPL_PORTHANDLER_R)(int param);

int  YM3812Init(int num, int clock, int rate);
void YM3812Shutdown(void);
void YM3812ResetChip(int which);
int  YM3812Write(int which, int a, int v);
unsigned char YM3812Read(int which, int a);
void YM3812Mute(int which,int channel,int mute);
int  YM3812TimerOver(int which, int c);
void YM3812UpdateOne(int which, qint16 *buffer, int length);

void YM3812SetTimerHandler(int which, OPL_TIMERHANDLER TimerHandler, int channelOffset);
void YM3812SetIRQHandler(int which, OPL_IRQHANDLER IRQHandler, int param);
void YM3812SetUpdateHandler(int which, OPL_UPDATEHANDLER UpdateHandler, int param);

#endif /* __FMOPL_H_ */
