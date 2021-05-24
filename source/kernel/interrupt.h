#ifndef __INTERTUPT_H
#define __INTERTUPT_H
#include "global.h"

typedef void* int_handler;
void IdtInit();

enum int_status
{
    INT_OFF,
    INT_ON
};

enum int_status SetIntStatus(enum int_status status);
enum int_status GetIntStatus();
enum int_status DisableInt();
enum int_status EnableInt();
#endif
