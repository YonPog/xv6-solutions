#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  backtrace();
  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_sigalarm(void) {
  int interval;
  if (argint(0, &interval) < 0) {
    return -1;
  }
  if (interval < 0) {
    return -1;
  }
  uint64 user_handler;
  if (argaddr(1, &user_handler) < 0) {
    return -1;
  }
  myproc()->alarm_interval = interval;
  myproc()->alarm_handler = user_handler;
  myproc()->passed_ticks = 0;
  if (interval == 0 && user_handler == 0) {
    myproc()->is_alarm_on = 0;
  } else {
    myproc()->is_alarm_on = 1;
  }
  return 0;
}

uint64 sys_sigreturn(void) {
  struct proc *p = myproc();
  // restore user registers
  memmove(p->trapframe, &(p->alarm_regs), sizeof(struct trapframe));
  // allow for more alarm calls
  p->is_in_alarm_handler = 0;
  return 0;
}
