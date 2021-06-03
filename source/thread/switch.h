#ifndef __THREAD_SWITCH_H
#define __THREAD_SWITCH_H

void switch_to(struct task_struct *current_thread, struct task_struct *next_thread);

#endif
