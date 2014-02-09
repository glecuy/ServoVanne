
#ifndef _TIMER_H_
#define _TIMER_H_

int timer1_init(void);
unsigned long timer1_GetTicks(void);

void UsSleep( unsigned long Delay );
#define MsSleep(d) UsSleep((unsigned long)d*1000U)

void timerO_PWM_Init(void);
void timerO_PWM_SetValue(unsigned char Val);


#endif // _TIMER_H_
