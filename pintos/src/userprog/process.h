#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H
#include "threads/thread.h"

tid_t process_execute (const char *file_name_plus_arguments);
int process_wait (tid_t, struct thread * t);
void process_exit (void);
void process_activate (void);

struct child * get_child(tid_t tid, struct thread *t);


#endif /* userprog/process.h */
