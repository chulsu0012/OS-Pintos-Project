#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include "userprog/process.h"

typedef int pid_t;
#define PID_ERROR ((pid_t)-1)

void syscall_init (void);

void check_address(const void* vaddr);
void sys_halt(void);
void sys_exit(int status);
pid_t sys_exec(const char* file);
int sys_wait(pid_t);
int sys_read(int fd, void* buffer, unsigned size);
int sys_write(int fd, const void* buffer, unsigned size);
int sys_fibo(int n);
int sys_max(int a, int b, int c, int d);

#endif /* userprog/syscall.h */
