#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include <stdio.h>
#include "threads/thread.h"

void syscall_init (void);

bool userAddressValid (void * virtualAddress, struct thread *t);
void halt (void);
void sysexit (int status, struct thread *t);
tid_t exec (const char * cmd_line);
int wait (tid_t tid);
//static int get_user(const uint8_t *uaddr);
//static bool put_user(uint8_t *udst, uint8_t byte);

bool create(const char * file, unsigned initial_size);
bool remove(const char * file);
int open(const char * file);
int filesize(int fd);
int read(int fd, void * buffer, unsigned size);
int write(int fd, const void * buffer, unsigned size);
void seek(int fd, unsigned position);
unsigned tell(int fd);
void close(int fd);

/* A struct to keep file descriptor -> file pointer mapping FIX THIS SHIT*/ 
struct file_desc
{
  struct file * fp;
  int fd;
  struct list_elem elem;
};


#endif /* userprog/syscall.h */
