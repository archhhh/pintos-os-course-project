#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);
void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}
void exit_handler(int status);
static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  int syscall_num = *((int*)f->esp);
  switch(syscall_num)
  {
	case 0:
		shutdown_power_off();
		break;
	case 1:
		valid_address((int*)f->esp+1);
		f->eax = *((int*)f->esp+1);
		exit_handler(f->eax);
		break;
	case 2:
		valid_address((int*)f->esp+1);
		f->eax = process_execute(*((int*)f->esp+1));		
  		break;
	case 4:
		{valid_address((int*)f->esp+4);
		valid_address((int*)f->esp+5);
		enum intr_level old_level;
		old_level = intr_disable();
		f->eax = filesys_create(*((int*)f->esp+4), *((int*)f->esp+5));
		intr_set_level(old_level);
		break;}
/*	case 5:{
		valid_address((int*)f->esp+4);
		valid_address((int*)f->esp+5);
		enum intr_level old_level;
		old_level = intr_disable();
		struct file* t = filesys_open(*((int*)f->esp+1));
		if(t)
		{
			if((t->inode)->open_cnt - 1 >= 0)
			{
				t->removed = true;
				f->eax = true;
			}
			else{
				f->eax = filesys_remove(*((int*)f->esp+1));
		}
		}
		f->eax = false;
		intr_set_level(old_level);
		break;
		}
  }	
 // thread_exit ();*/
 	case 9:
		valid_address((int*)f->esp+5);
		valid_address((int*)f->esp+6);
		valid_address((int*)f->esp+7);
		putbuf(*((int*)f->esp+6), *((int*)f->esp+7));
		f->eax = *((int*)f->esp+7);
		break;
 }
}
void valid_address(void * addr)
{
	if(!pagedir_get_page(thread_current()->pagedir, addr))
		exit_handler(-1);
}
void exit_handler(int status)
{
	printf("%s, exit(%d)\n", thread_current()->name, status);
	process_exit();
	thread_exit();
}
