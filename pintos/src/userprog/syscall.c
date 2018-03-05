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
#include <user/syscall.h>
#include "threads/malloc.h"

struct lock file_sys_lock;
static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
	lock_init(&file_sys_lock);
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
      sys_exit(-1, thread_current());
  
  int syscall_id = *sp;
  //printf ("system call!\n");
  //thread_exit ();

        //System call handler based on syscall_id
	switch(syscall_id)
	{
  		case SYS_EXEC:
			if(!userAddressValid(sp+1, thread_current()) || !userAddressValid((void *)(sp+1), thread_current()))
 	  			sys_exit(-1, thread_current());
			f->eax = sys_exec((void *)(sp+1));
		break;

  		case SYS_HALT:
			sys_halt();
		break;

  		case SYS_EXIT:
			if(!userAddressValid(sp+1, thread_current()))
 	  			sys_exit(-1, thread_current());
			sys_exit(*(sp+1), thread_current());
		break;

  		case SYS_WAIT:
      			if(!userAddressValid(sp+1, thread_current()))
        			sys_exit(-1, thread_current()); 
      			f->eax = sys_wait(*(sp+1), thread_current());
      		break;
    
  		case SYS_CREATE:
      			if(!userAddressValid(sp+4, thread_current()) || !userAddressValid(sp+5, thread_current()) 
			   					     || !userAddressValid((void *)(sp+4), thread_current()))
        			sys_exit(-1, thread_current());
      			f->eax = sys_create ((void*)(sp+4), *(sp+5));
      		break;
    
    		case SYS_REMOVE:
      			if(!userAddressValid(sp+1, thread_current()) || !userAddressValid((void *)(sp+1), thread_current()))
        			sys_exit(-1, thread_current());
      			f->eax = sys_remove((void *)(sp+1));
      		break;
    
    		case SYS_OPEN:
      			if(!userAddressValid (sp+1, thread_current()) || !userAddressValid((void *)(sp+1), thread_current()))
        			sys_exit(-1, thread_current());
      			f->eax = sys_open ((void *)(sp+1), thread_current());
      		break;
   
    		case SYS_FILESIZE:
      			if (!userAddressValid(sp+1, thread_current()))
        			sys_exit(-1, thread_current());
      			f->eax = filesize(*(sp+1));
      		break;

    		case SYS_READ:
      			if (!userAddressValid(sp+5, thread_current()) || !userAddressValid (sp+6, thread_current()) 
			    			                      || !userAddressValid (sp+7, thread_current()) 
			    					      || !userAddressValid ((void *)(sp+6), thread_current()))
        			sys_exit(-1, thread_current());
      			f->eax=sys_read(*(sp+5),(void *)(sp+6),*(sp+7));
      		break;
		
    		case SYS_WRITE:
      			if (!userAddressValid(sp+5, thread_current()) || !userAddressValid(sp+6, thread_current()) 
			    				              || !userAddressValid (sp+7, thread_current())
			    					      || !userAddressValid((void *)(sp+6), thread_current()))
			{
        			sys_exit(-1, thread_current());
			}
      			f->eax = sys_write(*(sp+5),(void *)(sp+6),*(sp+7));
      		break;
    
    		case SYS_SEEK:
      			if(!userAddressValid(sp+4, thread_current()) || !userAddressValid(sp+5, thread_current()))
        			sys_exit(-1, thread_current());
      			seek(*(sp+4),*(sp+5));
      		break;
    
	
    		case SYS_TELL:
      			if(!userAddressValid(sp+1, thread_current()))
        			sys_exit(-1, thread_current());
      			f->eax = sys_tell(*(sp+1));
      		break;
		
    		case SYS_CLOSE:
      			if (!userAddressValid(sp+1, thread_current()))
        			sys_exit(-1, thread_current());
      			close(*(sp+1));
      		break;
    
    		default:
      			hex_dump((int)sp, sp, 64, true);
      			printf("Invalid SysCall ID\n");
      			sys_exit(-1, thread_current());   
   
    		break;
	}
}

void sys_halt (void)
{
  shutdown_power_off();
}

void sys_exit (int status, struct thread *t)
{
    //if(thread_alive(t->parent))
        //t->cp->status = status;
    thread_exit();
       
}

pid_t sys_exec (const char * cmd_line)
{
	lock_acquire(&file_sys_lock);
	tid_t tid = process_execute(cmd_line);
	lock_release(&file_sys_lock);
	return (tid);
}

int sys_wait (tid_t tid, struct thread * t)
{
	struct child * child = get_child(tid, t);
	if(list_empty(&t->children) || (child == NULL))
		return (-1);
	
	t->kid_being_waited_on = tid;
	
	if (child->dirty == false)
		sema_down(&t->child_semaphore);
	int savehiscookies = child->cookies; //we want the kid's cookies, but not the kid
	list_remove(&child->childelem);
	free(child); //be free, son!
	
    	return (savehiscookies);
}

bool sys_create (const char * file, unsigned initial_size)
{
	if(file !=NULL)
	{
		lock_acquire(&file_sys_lock);
		bool coolinboolin = filesys_create(file, initial_size);
		lock_acquire(&file_sys_lock);
		return (coolinboolin);
	}
	return (false);
}

bool sys_remove (const char * file)
{
	if (file!= NULL)
	{
		lock_acquire(&file_sys_lock);
		bool removed = filesys_remove(file);
		lock_release(&file_sys_lock);
		return(removed);
	}
	return (false);
}

int sys_open (const char * file, struct thread * t) 
{
	lock_acquire(&file_sys_lock);
	struct file * temp = filesys_open(file);
	lock_release(&file_sys_lock);
	if(temp!= NULL)
	{
		struct file_desc * temp_fd = malloc(sizeof(file_desc));
		temp_fd->fd = ++t->fd_count;
		temp_fd->fp = temp;
		list_push_front(&t->file_list, &temp_fd->elem);
		return (fd_temp->fd);
	}
	return (-1);
}

int sys_filesize (int fd)
{
	return (-1);
}

int sys_read (int fd, void * buffer, unsigned size)
{
	return (-1);
}

int sys_write (int fd, const void * buffer, unsigned size)
{
	return (-1);
}

void sys_seek (int fd, unsigned position)
{
	
}

unsigned sys_tell (int fd)
{
	return 0;
}

void sys_close (int fd)
{

}

struct file_desc * get_file(int);

struct child *  get_child(tid_t tid, struct thread *t)
{
	return NULL;
}



