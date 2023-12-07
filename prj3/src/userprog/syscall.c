#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/synch.h"
#include "filesys/filesys.h"
#include "filesys/file.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  lock_init(&file_lock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  // printf ("system call!\n");

 /* project_1: System call Handler */
  uint32_t esp4 = *(uint32_t*)(f->esp + 4);
  uint32_t esp8 = *(uint32_t*)(f->esp + 8);
  uint32_t esp12 = *(uint32_t*)(f->esp + 12);
  uint32_t esp16 = *(uint32_t*)(f->esp + 16);

  switch(*(uint32_t*)(f->esp)) {
    case SYS_HALT:
      sys_halt();
      break;
    case SYS_EXIT :
      check_address(esp4);
      sys_exit(esp4);
      break;
    case SYS_EXEC :
      check_address(esp4);
      f->eax = sys_exec((char*)esp4);
      break;
    case SYS_WAIT:
      check_address(esp4);
      f->eax = sys_wait(esp4);
      break;
    case SYS_READ :
      check_address(esp4);
      check_address(esp8);
      check_address(esp12);
      f->eax = sys_read((int)esp4, (void*)esp8, (unsigned)esp12);
      break;
    case SYS_WRITE :
      check_address(esp4);
      check_address(esp8);
      check_address(esp12);
      f->eax = sys_write((int)esp4, (void*)esp8, (unsigned)esp12);
      break;
    /* project_1: Additional Implementation */
    case SYS_FIBO :
      check_address(esp4);
      f->eax = sys_fibo((int)esp4);
      break;
    case SYS_MAX :
      check_address(esp4);
      check_address(esp8);
      check_address(esp12);
      f->eax = sys_max((int)esp4, (int)esp8, (int)esp12, (int)esp16);
      break;
    /* project_2: System calls */
    case SYS_OPEN:
      check_address(esp4);
      f->eax = sys_open((const char*)esp4);
      break;
    case SYS_CLOSE:
      check_address(esp4);
      sys_close((int)esp4);
      break;
    case SYS_CREATE:
      check_address(esp4);
      check_address(esp8);
      f->eax = sys_create((const char*)esp4, (unsigned)esp8);
      break;
    case SYS_REMOVE:
      check_address(esp4);
      f->eax = sys_remove((const char*)esp4);
      break;
    case SYS_FILESIZE:
      check_address(esp4);
      f->eax = sys_filesize((int)esp4);
      break;
    case SYS_SEEK:
      check_address(esp4);
      check_address(esp8);
      sys_seek((int)esp4, (unsigned)esp8);
      break;
    case SYS_TELL:
      check_address(esp4);
      f->eax = sys_tell((int)esp4);
      break;
  }

  //thread_exit();
}


void check_address(const void* vaddr){
  if(!is_user_vaddr(vaddr)) sys_exit(-1);
}

void sys_halt(void) {
  shutdown_power_off();
}

void sys_exit (int status) {
/* project_2: System calls - update */
  printf("%s: exit(%d)\n", thread_name(), status);
  thread_current()->exit_status = status;
  for(int i = 3; i < 128; i++) {
    if (thread_current()->fd[i] != NULL) {
      sys_close(i);
    }
  }
  struct list_elem* e = list_begin(&(thread_current()->child));
  while(e != list_end(&(thread_current()->child))) {
    struct thread* t = list_entry(e, struct thread, child_elem);
    process_wait(t->tid);
    e = list_next(e);
  }
  file_close(thread_current()->cur_file);
  thread_exit();
}

pid_t sys_exec (const char* file) {
  return process_execute(file);
}

int sys_wait (pid_t pid) {
  return process_wait(pid);
}

int sys_read(int fd, void* buffer, unsigned size) {
/* project_2: System calls - update */
  if (!is_user_vaddr(buffer) || !buffer) sys_exit(-1);

  int i;
  lock_acquire(&file_lock);
  if (fd == 0) {
    for (i = 0; i < size; i++) {
      if(input_getc() == '\0') break;
    }
    return i;
  }
  else if (fd >= 3) {
    struct file* file = thread_current()->fd[fd];
    if(!file){
			lock_release(&file_lock);
			sys_exit(-1);
		}
		int result = file_read(file, buffer, size);
		lock_release(&file_lock);
		return result;
  }
	lock_release(&file_lock);
  return -1;
}

int sys_write (int fd, const void* buffer, unsigned size) {
/* project_2: System calls - update */
  lock_acquire(&file_lock);
  if (fd == 1) {
    putbuf((char*)buffer, (size_t)size);
    lock_release(&file_lock);
    return size;
  }
  else if (fd >= 3) {
    struct file* file = thread_current()->fd[fd];
    if (!file) {
      lock_release(&file_lock);
      sys_exit(-1);
    }
    int result;
    result = file_write(file, buffer, size);
    lock_release(&file_lock);
    return result;
  }
  lock_release(&file_lock);
  return -1;
}

int sys_fibo (int n) {
  if (n <= 1) return n;
  return sys_fibo(n - 1) + sys_fibo(n - 2);
}

int sys_max (int a, int b, int c, int d) {
  a = a > b ? a : b;
  a = a > c ? a : c;
  return a > d ? a : d;
}

/* project_2: System calls */
int sys_open(const char* file) {
  if(!is_user_vaddr(file) || !file) sys_exit(-1);

  lock_acquire(&file_lock);
  struct file* fp = filesys_open(file);
  lock_release(&file_lock);

  if (!fp) {
    return -1;
  }
  else {
    for (int i = 3; i < 128; i++) {
      if (thread_current()->fd[i] == NULL) {
        if(strcmp(thread_current()->name, file) == 0) {
          file_deny_write(fp);
        }
        thread_current()->fd[i] = fp;
        return i;
      }
    }
  }
}

void sys_close(int fd) {
  if(thread_current()->fd[fd] == NULL) sys_exit(-1);
  file_close(thread_current()->fd[fd]);
  thread_current()->fd[fd] = NULL;
  return file_close(thread_current()->fd[fd]);
}

bool sys_create(const char* file, unsigned initial_size) {
  if(!file) sys_exit(-1);
  return filesys_create(file, initial_size);
}

bool sys_remove(const char* file) {
  if(!file) sys_exit(-1);
  return filesys_remove(file);
}

int sys_filesize(int fd) {
  if(thread_current()->fd[fd] == NULL) sys_exit(-1);
  return file_length(thread_current()->fd[fd]);
}

void sys_seek(int fd, unsigned position) {
  if(thread_current()->fd[fd] == NULL) sys_exit(-1);
  file_seek(thread_current()->fd[fd], position);
}

unsigned sys_tell(int fd) {
	if(thread_current()->fd[fd] == NULL) sys_exit(-1);
	return file_tell(thread_current()->fd[fd]);
}