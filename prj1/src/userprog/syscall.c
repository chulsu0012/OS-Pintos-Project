#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
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
      f->eax = sys_read((int)esp4, (void*)esp8, (unsigned)esp12);
      break;
    case SYS_WRITE :
      f->eax = sys_write((int)esp4, (void*)esp8, (unsigned)esp12);
      break;
    /* project_1: Additional Implementation */
    case SYS_FIBO :
      f->eax = sys_fibo((int)esp4);
      break;
    case SYS_MAX :
      f->eax = sys_max((int)esp4, (int)esp8, (int)esp12, (int)esp16);
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
  printf("%s: exit(%d)\n", thread_name(), status);
  thread_current()->exit_status = status;
  thread_exit();
}

pid_t sys_exec (const char* file) {
  return process_execute(file);
}

int sys_wait (pid_t pid) {
  return process_wait(pid);
}

int sys_read(int fd, void* buffer, unsigned size) {
  int i; uint8_t c;
  if (fd == 0) {
    for (i = 0; i < (int)size; i++) {
      uint8_t c = input_getc();
      if(c == '\0') break;
    }
    return i;
  }
  return -1;
}

int sys_write (int fd, const void* buffer, unsigned size) {
  if (fd == 1) {
    putbuf((char*)buffer, (size_t)size);
    return size;
  }
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