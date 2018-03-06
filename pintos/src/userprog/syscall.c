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
#include "filesys/file.h"
#include <user/syscall.h>
#include "threads/malloc.h"
#include "devices/input.h"


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
  struct thread *t = thread_current();
  //if the stack pointer isn't valid, exit with a status of -1
  if(!userAddressValid(sp, t))
      sys_exit(-1, t);
  
  int syscall_id = *sp;
  //printf ("system call!\n");
  //thread_exit ();

        //System call handler based on syscall_id
	switch(syscall_id)
	{
  		case SYS_EXEC:
			if(!userAddressValid(sp+1, t) || !userAddressValid((void *)(sp+1), t))
 	  			sys_exit(-1, t);
			f->eax = sys_exec((void *)(sp+1));
		break;

  		case SYS_HALT:
			sys_halt();
		break;

  		case SYS_EXIT:
			if(!userAddressValid(sp+1, t))
 	  			sys_exit(-1, t);
			sys_exit(*(sp+1), t);
		break;

  		case SYS_WAIT:
      			if(!userAddressValid(sp+1, t))
        			sys_exit(-1, t); 
      			f->eax = sys_wait(*(sp+1), t);
      		break;
    
  		case SYS_CREATE:
      			if(!userAddressValid(sp+4, t) || !userAddressValid(sp+5, t) || !userAddressValid((void *)(sp+4), t))
        			sys_exit(-1, t);
      			f->eax = sys_create ((void*)(sp+4), *(sp+5));
      		break;
    
    		case SYS_REMOVE:
      			if(!userAddressValid(sp+1, t) || !userAddressValid((void *)(sp+1), t))
        			sys_exit(-1, t);
      			f->eax = sys_remove((void *)(sp+1));
      		break;
    
    		case SYS_OPEN:
      			if(!userAddressValid (sp+1, t) || !userAddressValid((void *)(sp+1), t))
        			sys_exit(-1, t);
      			f->eax = sys_open ((void *)(sp+1), t);
      		break;
   
    		case SYS_FILESIZE:
      			if (!userAddressValid(sp+1, t))
        			sys_exit(-1, t);
      			f->eax = sys_filesize(*(sp+1));
      		break;

    		case SYS_READ:
      			if (!userAddressValid(sp+5, t) || !userAddressValid (sp+6, t) || !userAddressValid (sp+7, t) || !userAddressValid ((void *)(sp+6), t))
        			sys_exit(-1, t);
      			f->eax=sys_read(*(sp+5),(void *)(sp+6),*(sp+7));
      		break;
		
    		case SYS_WRITE:
      			if (!userAddressValid(sp+5, t) || !userAddressValid(sp+6, t) || !userAddressValid (sp+7, t) || !userAddressValid((void *)(sp+6), t))
			{
        			sys_exit(-1, t);
			}
      			f->eax = sys_write(*(sp+5),(void *)(sp+6),*(sp+7));
      		break;
    
    		case SYS_SEEK:
      			if(!userAddressValid(sp+4, t) || !userAddressValid(sp+5, t))
        			sys_exit(-1, t);
      			sys_seek(*(sp+4),*(sp+5));
      		break;
    
	
    		case SYS_TELL:
      			if(!userAddressValid(sp+1, t))
        			sys_exit(-1, t);
      			f->eax = sys_tell(*(sp+1));
      		break;
		
    		case SYS_CLOSE:
      			if (!userAddressValid(sp+1, t))
        			sys_exit(-1, t);
      			sys_close(*(sp+1));
      		break;
    
    		default:
      			hex_dump((int)sp, sp, 64, true);
      			printf("Invalid SysCall ID\n");
      			sys_exit(-1, t);   
   
    		break;
	}
}

void sys_halt (void)
{
  shutdown_power_off();
}

void sys_exit (int status, struct thread *t)
{
	t->exit_code = status;
	struct thread * parental = t->parent;
	if(!list_empty(&parental->children))
	{
		struct child * kid = get_child(t->tid, parental);
		if(kid != NULL)
		{
			kid->cookies = status;
			kid->dirty = true;
		}
		if(t->parent->kid_being_waited_on == t->tid)
			sema_up(&t->parent->child_semaphore);
	}
	
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
	return(process_wait(tid, t));
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
		struct file_desc * temp_fd = malloc(sizeof(struct file_desc));
		temp_fd->fd = ++t->fd_count;
		temp_fd->fp = temp;
		list_push_front(&t->file_list, &temp_fd->elem);
		return (temp_fd->fd);
	}
	return (-1);
}

int sys_filesize (int fd)
{
	struct file_desc * getit = get_file_desc(fd, thread_current());
	if(getit != NULL)
	{
		lock_acquire(&file_sys_lock);
		int length = file_length(getit->fp);
		lock_release(&file_sys_lock);
		return (length);
	}
	return (-1);
}

int sys_read (int fd, void * buffer, unsigned size)
{
	int length = 0; 
	
	if(fd == STDIN_FILENO) // = 0 file being keyed in
	{
      		uint8_t* buffy = (uint8_t *) buffer;
      		for (int i = 0; i < size; i++)
	  		buffy[i] = input_getc();
		return(size);
	}
	
	struct file_desc * getit = get_file_desc(fd, thread_current());
	if(getit != NULL)
	{
		lock_acquire(&file_sys_lock);
		length = file_read(getit->fp, buffer, size);
		lock_release(&file_sys_lock);
		return(length);
	}
	return (-1); //shouldn't happen
}

int sys_write (int fd, const void * buffer, unsigned size)
{
	if(fd == STDOUT_FILENO) // = 1 write to screen
	{
		putbuf(buffer, size);
		return (size);
	}
	
	struct file_desc * getit = get_file_desc(fd, thread_current());
	if(getit != NULL)
	{
		lock_acquire(&file_sys_lock);
		int staywrote = file_write(getit->fp, buffer, size);
		lock_release(&file_sys_lock);
		return(staywrote);
	}
	return (-1); //shouldn't happen
}

void sys_seek (int fd, unsigned position)
{
	struct file_desc * getit = get_file_desc(fd, thread_current());
	if(getit != NULL)
	{
		lock_acquire(&file_sys_lock);
		file_seek(getit->fp, position);
		lock_release(&file_sys_lock);
	}
}

unsigned sys_tell (int fd)
{
	struct file_desc * getit = get_file_desc(fd, thread_current());
	if(getit != NULL)
	{
		lock_acquire(&file_sys_lock);
		unsigned showntell = file_tell(getit->fp);
		lock_release(&file_sys_lock);
		return(showntell);
	}
	return (-1);
}

void sys_close (int fd)
{
	if(fd == STDOUT_FILENO || fd == STDIN_FILENO)
		return;
	
	struct file_desc * getit = get_file_desc(fd, thread_current());
	if(getit != NULL)
	{
		lock_acquire(&file_sys_lock);
		file_close(getit->fp);
		lock_release(&file_sys_lock);
		list_remove(&getit->elem);
		free(getit);
	}
}

struct file_desc * get_file_desc(int fd, struct thread * t)
{
	struct list_elem * e = list_begin(&t->file_list);
	while(e!= list_end(&t->file_list))
	{
		struct file_desc * fd_elem = list_entry(e, struct file_desc, elem);
		if (fd_elem->fd == fd)
			return (fd_elem);
		e = list_next(e);
	}
	return (NULL);	
}

struct child *  get_child(tid_t tid, struct thread *t)
{
	struct list_elem * e = list_begin(&t->children);
	while(e!= list_end(&t->children))
	{
		struct child * kid = list_entry(e, struct child, childelem);
		if (kid->id == tid)
			return (kid);
		e = list_next(e);
	}
	return (NULL);	
}



