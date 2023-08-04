#include "timer.h"

#include <chrono>

using HiResClock = std::chrono::high_resolution_clock;

struct Timer {
  HiResClock::duration   accumulated_time;
  HiResClock::time_point mark;
};

Timer *NewTimer(void) {
  Timer *timer = new Timer();
  return timer;
}

void DestroyTimer(Timer *timer) {
  delete timer;
}

void ResumeTimer(Timer *timer) {
  timer->mark = HiResClock::now();
}

void PauseTimer(Timer *timer) {
  HiResClock::time_point end = HiResClock::now();
  timer->accumulated_time += end - timer->mark;
}

uint64_t TimerDurationNSec(Timer *timer) {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(
    timer->accumulated_time).count();
}