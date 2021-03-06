#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return proc->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(proc->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
//********our code*********
typedef void (*sighandler_t)(int);
int
sys_signal(void)
{
  sighandler_t handler=0;
  int signum=0;

  if(argint(0, &signum) < 0)
    return -1;

  if(argptr(1, (void *)&handler,sizeof(handler)) < 0)
    return -1;

  return (int)(signal(signum,(sighandler_t)handler));
}

int
sys_sigsend(void)
{
  int pid=0;
  int signum=0;

  if(argint(0, &pid) < 0)
    return -1;
  if(argint(1, &signum) < 0)
    return -1;
  
  return sigsend(pid,signum);
}

int sys_sigreturn(void){
  return sigreturn();
}

int sys_alarm(void){
  int alarm_ticks;

  if(argint(0, &alarm_ticks) < 0)
    return -1;
  if (alarm_ticks==0)
    return alarm(0);
  
  return alarm(alarm_ticks+ticks);

}
