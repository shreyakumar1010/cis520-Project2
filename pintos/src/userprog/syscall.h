#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include <stdio.h>
#include "threads/thread.h"

void syscall_init (void);

bool userAddressValid (void * virtualAddress, struct thread *t);
void sys_halt (void);
void sys_exit (int status, struct thread *t); //renamed from exit to sysexit
tid_t sys_exec (const char * cmd_line);
int sys_wait (tid_t tid, struct thread * t);
//static int get_user(const uint8_t *uaddr);
//static bool put_user(uint8_t *udst, uint8_t byte);

bool sys_create(const char * file, unsigned initial_size);
bool sys_remove(const char * file);
int sys_open(const char * file, struct thread * t);
int sys_filesize(int fd);
int sys_read(int fd, void * buffer, unsigned size);
int sys_write(int fd, const void * buffer, unsigned size);
void sys_seek(int fd, unsigned position);
unsigned sys_tell(int fd);
void sys_close(int fd);

struct child * get_child(tid_t tid, struct thread *t);

/* A struct to keep file descriptor -> file pointer mapping FIX THIS SHIT*/ 
struct file_desc
{
  struct file * fp;
  int fd;
  struct list_elem elem;
};


#endif /* userprog/syscall.h */
