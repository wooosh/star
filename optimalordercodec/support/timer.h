#ifndef TIMER_H_
#define TIMER_H_

#include <stdint.h>

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

typedef struct Timer Timer;

EXTERNC Timer *NewTimer(void);
EXTERNC void DestroyTimer(Timer *timer);
EXTERNC void ResumeTimer(Timer *timer);
EXTERNC void PauseTimer(Timer *timer);
EXTERNC uint64_t TimerDurationNSec(Timer *timer);

#undef EXTERNC
#endif