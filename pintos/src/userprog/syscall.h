#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);

bool userAddressValid (void * virtualAddress, struct thread *t);
void hald (void);
void exit (int status, struct thread *t);
pid_t exec (const char * cmd_line);
int wait (pid_t pid);
  
#endif /* userprog/syscall.h */
