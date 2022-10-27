#include <am.h>
#include <nemu.h>

static uint64_t boot_time = 0;

void __am_timer_init() {
  uint64_t h_32 = inl(RTC_ADDR + 4);
  uint32_t l_32 = inl(RTC_ADDR);
  boot_time = (h_32 << 32) + l_32;
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  uint64_t h_32 = inl(RTC_ADDR + 4);
  uint32_t l_32 = inl(RTC_ADDR);
  uptime->us = ((h_32 << 32) + l_32) - boot_time;
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
