#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  int d;
  char *s;
  size_t i = 0;

  va_start(ap, fmt);
  while(*fmt){
    if(*fmt == '%'){
      switch(*(fmt+1)){
        // case '\0': 
        case '%': out[i++] = '%'; break;
        case 's': 
          s = va_arg(ap, char *);
          strcpy(&out[i], s);
          i += strlen(s);
          break;
        case 'd':
          d = va_arg(ap, int);
          int tmp_d = d;
          if(d < 0){
            out[i++] = '-';
            tmp_d = -tmp_d;
          } 
          char d_str[21];
          d_str[20] = '\0';
          int digit = 0;
          while(1){
            digit++;
            d_str[20 - digit] = (tmp_d % 10) + '0';
            if((tmp_d = tmp_d / 10)) break; 
          }
          strcpy(&out[i], &d_str[20 - digit]);
          i += digit;
          break;
      }
      fmt += 2;
    }
    else {
      out[i++] = *fmt;
      fmt += 1;
    }
  }
      
  va_end(ap);
  out[i] = '\0';  

  return i-1;
}
  


int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
