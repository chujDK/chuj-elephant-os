#ifndef __USERPROG_PROCESS_H
#define __USERPROG_PROCESS_H
#include "global.h"
#include "thread.h"

#define DEFAULT_PRIORITY 31
#define USER_STACK3_VADDR  (0x7FFFE000 - 0x1000)
#define USER_VADDR_START 0x8048000

void StartProcess(void* filename);
void ActiveProcess(PCB* pthread);
void ExecProcess(void* filename, char* name);

#endif