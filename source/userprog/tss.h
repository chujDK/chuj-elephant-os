#ifndef __USERPROG_TSS_H
#define __USERPROG_TSS_H
#include "thread.h"
void UpdateTssEsp(PCB* pthread);
void TssInit();
#endif