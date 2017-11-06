#include <bios.h>
#include <stdio.h>
#include <util.h>
#include <kernel_sched.h>

#include <kernel_proc.h>
#include <assert.h>


/*void bootfunc() {
  fprintf(stderr, "Hello from core %u\n", cpu_core_id);
}

int main()
{
  vm_boot(bootfunc, 4, 0);
  return 0;
}
*/


int main(){

PTCB* p = malloc(sizeof(PTCB));
PCB* pc = malloc(sizeof(PCB*));
printf("%d\n",sizeof(PTCB));
pc->pstate = FREE;
printf("1\n");
p->parent = pc;
printf("%x\n",p);

rlnode_init(& pc->ptcb_list, NULL);
pc->ptcb_list.ptcb = p;

printf("%x\n",p->parent);
//assert(p == NULL);



return 0;
}
