#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sysinfo.h"

int
sys_sysinfo(void)
{
  struct sysinfo info;
  uint64 addr;
  argaddr(0, &addr);

  info.freemem = kfreemem();
  info.nproc = nproc();

  struct proc *p = myproc();
  if(copyout(p->pagetable, addr, (char *)&info, sizeof(info)) < 0)
      return -1;

  return 0;
}