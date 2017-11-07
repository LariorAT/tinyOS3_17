
#include "tinyos.h"
#include "kernel_sched.h"
#include "kernel_proc.h"

/** 
  @brief Create a new thread in the current process.
  */
Tid_t sys_CreateThread(Task task, int argl, void* args)
{ 
  PCB * curproc = CURPROC;
  PTCB* p = initialize_PTCB(curproc);
  
  p->main_task = task;
  p->args = args;
  p->argl = argl;
  /*creating the new PTCB node*/
  rlnode_init(& p->ptcb_self_node,p);     //check for NULL exception
  rlist_push_back(& curproc->ptcb_list,& p->ptcb_self_node);
  curproc->counter++; //may need mutex_lock TBC

  p->main_thread = spawn_thread(curproc, start_main_thread);
  wakeup(p->main_thread);

  
	/*return the TCB Adress as Thread ID*/
  return (Tid_t)p->main_thread;
}

/**
  @brief Return the Tid of the current thread.
 */
Tid_t sys_ThreadSelf()
{

	return (Tid_t) CURTHREAD;
}

/**
  @brief Join the given thread.
  */
int sys_ThreadJoin(Tid_t tid, int* exitval)
{
  return -1;
}

/**
  @brief Detach the given thread.
  */
int sys_ThreadDetach(Tid_t tid)
{
  TCB* t = (TCB*)tid;
  if(t == NULL || t->state == EXITED || t->owner_ptcb->isExited == 1){
    return -1;
  }
  t->owner_ptcb->isDetached = 1;
  return 0;
	
}

/**
  @brief Terminate the current thread.
  */
void sys_ThreadExit(int exitval)
{
  printf("test4\n");
}

