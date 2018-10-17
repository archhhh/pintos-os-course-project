#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
static void syscall_handler (struct intr_frame *);

struct file_desc{
	struct list_elem file_elem;
	int fd;
	struct file* file_ptr;
};

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}
void exit_handler(int status, struct intr_frame *f);
void argument_check(struct intr_frame* f, int argc);
static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  valid_address(f->esp, f);
  int syscall_num = *((int*)f->esp);
  switch(syscall_num)
  {
	case 0:
		shutdown_power_off();
		break;
	case 1:
		argument_check(f,1); 
		exit_handler(*((int*)f->esp+1), f);
		break;
	case 2:
		argument_check(f,1);
		valid_address(*((int*)f->esp+1), f);
		f->eax = process_execute(*((int*)f->esp+1));		
  		break;
	case 3:
		argument_check(f, 1);
		f->eax = process_wait(*((int*)f->esp+1));
		break;
	case 4:
		{
		argument_check(f, 2);
		valid_address(*((int*)f->esp+4), f);
		f->eax = filesys_create(*((int*)f->esp+4), *((int*)f->esp+5));
		break;
		}
	case 5:
		{
		argument_check(f, 1);
		valid_address(*((int*)f->esp+1), f);
	/*	struct file* t = filesys_open(*((int*)f->esp+1));
		if(t)
		{
			if(t->inode)->open_cnt - 1 >= 0)
			{
				t->removed = true;
				f->eax = true;
			}
			else{
				f->eax = filesys_remove(*((int*)f->esp+1));
			}
		}else
			f->eax = false; */
		f->eax = filesys_remove(*((int*)f->esp+1));
		break;
		}
	case 6:		
		argument_check(f,1);
		valid_address(*((int*)f->esp+1),f);
		struct file * _f = filesys_open(*((int*)f->esp+1));
		if(_f == NULL)
			f->eax = -1;
		else{
			struct file_desc* f_desc = malloc(sizeof(struct file_desc));
			if(f_desc== NULL)
				f->eax = -1;
			else{
				f_desc->file_ptr = _f;
				f_desc->fd = thread_current()->fd;
				thread_current()->fd += 1;
				list_push_back(&thread_current()->files, &f_desc->file_elem);
				f->eax = f_desc->fd;
			}	
		}
		break;
	case 7:{
		argument_check(f,1);
		struct list_elem * e;
		if(list_empty(&thread_current()->files))
			f->eax = -1;
		else{
		struct file * _f;
		for(e = list_begin(&thread_current()->files); e != list_end(&thread_current()->files); e = list_next(e))
		{
			struct file_desc * f_d = list_entry(e, struct file_desc, file_elem);
			if(f_d -> fd == *((int*)f->esp+1))
			{
				_f = f_d -> file_ptr;
			}
		}		
		if(_f == NULL)
			f->eax = -1;
		else
			f->eax = (int)file_length(_f);
		}
		break;
		}
 	case 9:{
		argument_check(f, 3);	
		valid_address(*((int*)f->esp+6), f);
		if(*((int*)f->esp+5) == 1)
		{
		putbuf(*((int*)f->esp+6), *((int*)f->esp+7));
		f->eax = *((int*)f->esp+7);
		}else
			f->eax = 0;
		break;
		}
 }
}
void valid_address(void * addr, struct intr_frame * f)
{
	if(!is_user_vaddr(addr) || !pagedir_get_page(thread_current()->pagedir, addr))
		exit_handler(-1, f);
}
void exit_handler(int status, struct intr_frame * f)
{
	printf("%s: exit(%d)\n", thread_current()->name, status);
	process_exit();
	thread_exit();
	f->eax = status;
}
void argument_check(struct intr_frame * f, int argc){
	switch(argc)
	{
		case 3:
			valid_address(f->esp, f);
			valid_address((int*)f->esp+5, f);
			valid_address((int*)f->esp+6, f);
			valid_address((int*)f->esp+7, f);
			break;
		case 2:
			valid_address(f->esp, f);
			valid_address((int*)f->esp+4, f);
			valid_address((int*)f->esp+5, f);
			break;
		case 1:
			valid_address(f->esp, f);
			valid_address((int*)f->esp+1, f);
			break;
	}
}
