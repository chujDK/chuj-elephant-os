#ifndef __KERNEL_DEBUG_H
#define __KERNEL_DEBUG_H
#define DEBUG 1

void PanicSpin(char* filename, int line, const char* func, const char* condition);
#define PANIC(...) PanicSpin (__FILE__, __LINE__, __func__, __VA_ARGS__)

#if !DEBUG
    #define ASSERT(CONDITION)
#else
    #define ASSERT(CONDITION) \
    if(CONDITION){} \
    else{ \
        PANIC(#CONDITION); \
    }
#endif /* DEBUG */

#endif /* __KERNEL_DEBUG_H */
