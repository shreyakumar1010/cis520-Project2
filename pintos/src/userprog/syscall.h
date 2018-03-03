#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include "threads/thread.h"
#include "threads/interrupt.h"
#include <stdio.h>
#include <syscall-nr.h>

void syscall_init (void);

bool userAddressValid (void * virtualAddress, struct thread *t);
void hald (void);
void exit (int status, struct thread *t);
pid_t exec (const char * cmd_line);
int wait (pid_t pid);

bool create(const char * file, unsigned initial_size);
bool remove(const char * file);
int open(const char * file);
int filesize(int fd);
int read(int fd, void * buffer, unsigned size);
int write(int fd, const void * buffer, unsigned size);
void seek(int fd, unsigned position);
unsigned tell(int fd);
void close(int fd);



#endif /* userprog/syscall.h */
