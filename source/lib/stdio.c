#include "stdio.h"
#include "syscall.h"

#define va_start(ap, v) ap = (va_list) &v
#define va_arg(ap, t) *((t*)(ap+=4))
#define va_end(ap) ap = NULL