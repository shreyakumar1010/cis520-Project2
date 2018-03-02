#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);

struct lock file_sys_lock;

void
syscall_init (void) 
{
  lock_init(&file_sys_lock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

/* Reads a byte at user virtual address UADDR.UADDR must be below PHYS_BASE.
Returns the byte value if successful, -1 if a segfault occurred. */
static int get_user (const uint8_t *uaddr)
{
    int result;
    asm ("movl $1f, %0; movzbl %1, %0; 1:": "=&a" (result) : "m" (*uaddr));
    return result;
}

/* Writes BYTE to user address UDST.UDST must be below PHYS_BASE.
Returns true if successful, false if a segfault occurred. */
static bool put_user (uint8_t *udst, uint8_t byte)
{
    int error_code;
    asm ("movl $1f, %0; movb %b2, %1; 1:": "=&a" (error_code), "=m" (*udst) : "q" (byte));
    return error_code != -1;
}

// Validates the user address
bool userAddressValid (void * virtualAddress, struct thread * t)
{
    if (pagedir_get_page(t->pagedir, virtualAddress)!= NULL)
    {
        if (is_user_vaddr(virtualAddress))
            return true;
    }
    else
        return false; 
}

static void
syscall_handler (struct intr_frame *f) 
{
  esp = f->esp;
  
  //if the stack pointer isn't valid, exit with a status of -1
  if(!userAddressValid(esp, thread_current()))
      exit(-1, thread_current());
  
  int syscall_id = *p;
  //printf ("system call!\n");
  //thread_exit ();
}

void halt (void)
{
  shutdown_power_off();
}

void exit (int status, struct thread *t)
{
    if(thread_alive(t->parent))
        t->cp->status = status;
    thread_exit();
       
}

pid_t exec (const char * cmd_line)
{

}

int wait (pid_t pid)
{
    return (process_wait (pid));
}

bool create (const char * file, unsigned initial_size)
{

}

bool remove (const char * file)
{

}

int open (const char * file) 
{

}

int filesize (int fd)
{

}

int read (int fd, void * buffer, unsigned size)
{

}

int write (int fd, const void * buffer, unsigned size)
{

}

void seek (int fd, unsigned position)
{

}

unsigned tell (int fd)
{

}

void close (int fd)
{

}




