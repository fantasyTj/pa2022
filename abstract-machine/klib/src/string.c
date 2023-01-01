#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t len = 0;

  while(*(s++)!= '\0') len += 1;

  return len;
}

char *strcpy(char *dst, const char *src) {
  size_t i;

  for(i = 0; src[i] != '\0'; i++) dst[i] = src[i];
  dst[i] = '\0';

  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  size_t i;

  for(i = 0; i < n && src[i] != '\0'; i++) dst[i] = src[i];
  for( ; i < n; i++) dst[i] = '\0';

  return dst;
}

char *strcat(char *dst, const char *src) {
  size_t dst_len = strlen(dst);
  size_t i;

  for(i = 0; src[i] != '\0'; i++) dst[dst_len + i] = src[i];
  dst[dst_len + i] = '\0';

  return dst;
}

int strcmp(const char *s1, const char *s2) {
  int delta;
  size_t i = 0;

  while(s1[i] != '\0' && s2[i] != '\0'){
    if((delta = s1[i] - s2[i])) return delta;
    i++;
  }
  return s1[i] - s2[i];
}

int strncmp(const char *s1, const char *s2, size_t n) {
  int delta;
  size_t i = 0;

  while(s1[i] != '\0' && s2[i] != '\0'){
    if(i == n) return 0;
    if((delta = s1[i] - s2[i])) return delta;
    i++;
  }
  return s1[i] - s2[i];
}

// void *memset(void *s, int c, size_t n) {
//   unsigned char c_ = c;
//   char *s_ = (char *)s;
  
//   for(size_t i = 0; i < n; i++) s_[i] = c_;

//   return s_;
// }

void *memset(void *s, int c, size_t n) {
  size_t major = n / 4;
  size_t remain = n % 4;
  uint32_t *u32_s = (uint32_t *)s;
  uint8_t one_c = (uint8_t)c;
  uint32_t four_c = c<<24 | c<<16 | c<<8 | c;
  if(major > 0) {
    for(size_t i = 0; i < major; i++) u32_s[i] = four_c;
    u32_s += major;
  }
  char *char_s = (char *)(void *)u32_s;
  for(size_t i = 0; i < remain; i++) char_s[i] = one_c;
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  char *dst_ = (char *)dst, *src_ = (char *)src;

  if(dst_ >= src_ && dst_ - src_ < n){
    for(size_t i = n ; i > 0; i--) dst_[i - 1] = src_[i - 1];
  }else for(size_t i = 0; i < n; i++) dst_[i] = src_[i];
  
  return dst_;
}

void *memcpy(void *out, const void *in, size_t n) {
  char *out_ = (char *)out, *in_ = (char *)in;
  
  for(size_t i = 0; i < n; i++) out_[i] = in_[i];

  return out_;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  char *s1_ = (char *)s1, *s2_ = (char *)s2;
  size_t i = 0;
  int delta;
  
  while(i < n){
    if((delta = s1_[i] - s2_[i])) return delta;
    i++;
  }
  return 0;
}

#endif
