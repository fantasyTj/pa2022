#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

typedef struct fmt_info{
  char type;
  int32_t width;
} fmt_info;

static int32_t num2str_inv(char *start, int num){
  int32_t idigit = 0;
  while(1){
    start[idigit++] = (num % 10) + '0';
    if(!(num = num / 10)) break; 
  }
  if(num < 0) start[idigit++] = '-';

  return idigit - 1;
}

static int32_t str2num(char *numstr){
  size_t len = strlen(numstr);
  int32_t num = 0;
  for(size_t i = 0; i < len; i++) num = num * 10 + (numstr[i] - '0');
  return num;
}

static int grl_vnp(bool is_str, char *out, size_t n, const char *fmt, va_list ap){
  if(is_str) assert(out);
  char *s; int d;
  size_t idx = 0;

  while(*fmt){
    if(idx >= n) break;
    if(*fmt == '%'){
      fmt+=1;
      putch(*fmt);
      fmt_info temp_info;
      char numstr[8];
      uint32_t t_idx = 0;
      while(*fmt >= '0' && *fmt <= '9'){
        numstr[t_idx++] = *fmt;
        fmt+=1;
      }
      numstr[t_idx] = '\0';
      temp_info.width = (t_idx == 0)?(0):(str2num(numstr));
      temp_info.type = *fmt;
      putch(temp_info.type);
      fmt+=1;
      putch(*fmt);
      switch(*(fmt-1)){
        // case '\0': 
        case '%':{
          if(is_str) out[idx++] = '%';
          else{
            putch('%');
            idx++;
          }
          break;
        }
        case 's':{
          s = va_arg(ap, char *);
          if(idx + strlen(s) > n) break;
          if(is_str) for(; *s; s++) out[idx++] = *s;
          else {
            for(; *s; s++) putch(*s);
            idx += strlen(s);
          }
          break;
        }
        case 'd':{
          d = va_arg(ap, int);
          char d_str[21];
          int32_t digit = num2str_inv(d_str, d);
          if(idx + digit > n) break;
          // int32_t delta = temp_info.width - digit;
          // if(delta > 0){
          //   while(delta--){
          //     if(is_str) out[idx++] = '0';
          //     else{
          //       putch('0');
          //       idx++;
          //     }
          //   }
            
          // }
          if(is_str) for(; digit >= 0; digit--) out[idx++] = d_str[digit];
          else{
            for(; digit >= 0; digit--) putch(d_str[digit]);
            idx += digit+1;
          }
          break;
        }
      }
      putch(*fmt);
      // fmt+=2;
    }else {
      if(is_str) out[idx++] = *fmt;
      else{
        putch(*fmt);
        idx++;
      }
      fmt += 1;
    }
  }
  if(is_str) out[idx] = '\0';
  return idx;
}

int printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int res = grl_vnp(false, NULL, -1, fmt, ap);

  va_end(ap);
  return res;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  return vsnprintf(out, -1, fmt, ap);
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int res = vsprintf(out, fmt, ap);
  
  va_end(ap);
  return res;
}
  
int snprintf(char *out, size_t n, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int res = vsnprintf(out, n, fmt, ap);

  va_end(ap);
  return res;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  return grl_vnp(true, out, n, fmt, ap);
}

#endif
