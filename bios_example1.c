#include <bios.h>
#include <stdio.h>

#include <util.h>
#include <kernel_sched.h>

#include <kernel_proc.h>
#include <assert.h>



void bootfunc() {
  fprintf(stderr, "Hello from core %u\n", cpu_core_id);
}

int main()
{
  vm_boot(bootfunc, 4, 0);
  return 0;
}

