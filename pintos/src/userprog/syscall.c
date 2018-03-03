#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/process.h"
#include "userprog/pagedir.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"
#include "filesys/filesys.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

// Validates the user address
bool userAddressValid (void * virtualAddress, struct thread * t)
{
    if (pagedir_get_page(t->pagedir, virtualAddress)!= NULL)
    {
        if (is_user_vaddr(virtualAddress))
            return true;
	else 
	    return false;
    }
    else
        return false; 
}

static void
syscall_handler (struct intr_frame *f) 
{
  int *sp = f->esp;
  
  //if the stack pointer isn't valid, exit with a status of -1
  if(!userAddressValid(sp, thread_current()))
      sysexit(-1, thread_current());
  
  int syscall_id = *sp;
  //printf ("system call!\n");
  //thread_exit ();

        //System call handler based on syscall_id
	switch(syscall_id)
	{
  		case SYS_EXEC:
			if(!userAddressValid(sp+1, thread_current()) || !userAddressValid((void *)(sp+1), thread_current()))
 	  			sysexit(-1, thread_current());
			f->eax = exec((void *)(sp+1));
		break;

  		case SYS_HALT:
			halt();
		break;

  		case SYS_EXIT:
			if(!userAddressValid(sp+1, thread_current()))
 	  			sysexit(-1, thread_current());
			sysexit(*(sp+1), thread_current());
		break;

  		case SYS_WAIT:
      			if(!userAddressValid(sp+1, thread_current()))
        			sysexit(-1, thread_current()); 
      			f->eax = wait(*(sp+1));
      		break;
    
  		case SYS_CREATE:
      			if(!userAddressValid(sp+4, thread_current()) || !userAddressValid(sp+5, thread_current()) 
			   					     || !userAddressValid((void *)(sp+4), thread_current()))
        			sysexit(-1, thread_current());
      			f->eax = create ((void*)(sp+4), (void *)(sp+5));
      		break;
    
    		case SYS_REMOVE:
      			if(!userAddressValid(sp+1, thread_current()) || !userAddressValid((void *)(sp+1), thread_current()))
        			sysexit(-1, thread_current());
      			f->eax = remove((void *)(sp+1));
      		break;
    
    		case SYS_OPEN:
      			if(!userAddressValid (sp+1, thread_current()) || !userAddressValid((void *)(sp+1), thread_current()))
        			sysexit(-1, thread_current());
      			f->eax = open ((void *)(sp+1));
      		break;
   
    		case SYS_FILESIZE:
      			if (!userAddressValid(sp+1, thread_current()))
        			sysexit(-1, thread_current());
      			f->eax = filesize(*(sp+1));
      		break;

    		case SYS_READ:
      			if (!userAddressValid(sp+5, thread_current()) || !userAddressValid (sp+6, thread_current()) 
			    			                      || !userAddressValid (sp+7, thread_current()) 
			    					      || !userAddressValid ((void *)(sp+6), thread_current()))
        			sysexit(-1, thread_current());
      			f->eax=read((void *)(sp+5),(void *)(sp+6),(void *)(sp+7));
      		break;
		
    		case SYS_WRITE:
      			if (!userAddressValid(sp+5, thread_current()) || !userAddressValid(sp+6, thread_current()) 
			    				              || !userAddressValid (sp+7, thread_current())
			    					      || !userAddressValid((void *)(sp+6), thread_current()))
        			sysexit(-1, thread_current());
      			f->eax = write((void *)(sp+5),(void *)(sp+6),(void *)(sp+7));
      		break;
    
    		case SYS_SEEK:
      			if(!userAddressValid(sp+4, thread_current()) || !userAddressValid(sp+5, thread_current()))
        			sysexit(-1, thread_current());
      		seek(*(sp+4),*(sp+5));
      		break;
    
	
    		case SYS_TELL:
      			if(!userAddressValid(sp+1, thread_current()))
        			sysexit(-1, thread_current());
      			f->eax = tell(*(sp+1));
      		break;
		
    		case SYS_CLOSE:
      			if (!userAddressValid(sp+1, thread_current()))
        			sysexit(-1, thread_current());
      			close(*(sp+1));
      		break;
    
    		default:
      			hex_dump(sp,sp,64,true);
      			printf("Invalid SysCall ID\n");
      			sysexit(-1, thread_current());   
   
    		break;
	}
}

void halt (void)
{
  shutdown_power_off();
}

void sysexit (int status, struct thread *t)
{
    //if(thread_alive(t->parent))
        //t->cp->status = status;
    thread_exit();
       
}

tid_t exec (const char * cmd_line)
{
  tid_t tid = 0;
  return tid;
}

int wait (tid_t tid)
{
    return (process_wait (tid));
}

bool create (const char * file, unsigned initial_size)
{
	return (false);
}

bool remove (const char * file)
{
	return (false);
}

int open (const char * file) 
{
	return (-1);
}

int filesize (int fd)
{
	return (-1);
}

int read (int fd, void * buffer, unsigned size)
{
	return (-1);
}

int write (int fd, const void * buffer, unsigned size)
{
	return (-1);
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

struct file_desc * get_file(int);




