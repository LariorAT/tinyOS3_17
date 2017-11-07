
#include "tinyos.h"
#include "kernel_sched.h"
#include "kernel_proc.h"




Mutex attached = MUTEX_INIT;

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
  p->main_thread->owner_ptcb = p;
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

  PCB * curproc = CURPROC; /***Local copy for speed*/

  rlnode* tempPTCB = &curproc->ptcb_list;
  int i;
  for (i = 0; i < curproc->counter;i++)
  {
    if((Tid_t)tempPTCB->ptcb->main_thread == tid){
      break;
    }
    else
    tempPTCB = tempPTCB->next;
  }
  if(i==curproc->counter)  /***Incase the the tread was not found*/
  {
    fprintf(stderr, "ERROR:Not thread with this ID in the process\n");
    return -1;
  }

  /*If we reach here we have found the thread we want to join*/

  /***Incase the tread we want to join is the current thread*/
  if(tempPTCB->ptcb->main_thread == CURTHREAD) 
  {
    fprintf(stderr, "ERROR:Can't join current thread\n");
    return -1;
  }
  /***Incase the tread we want to join is not undetached*/
  if(tempPTCB->ptcb->isDetached == 1)
  {
    fprintf(stderr, "ERROR:Thread to join is detached\n");
    return -1;
  }

  if(tempPTCB->ptcb->main_thread->state == EXITED) //If the thread is exited 
    return -1;
  

   /***If we reached here we can now join the thread*/
  Mutex_Lock(&attached);
  Cond_Wait(&attached,&tempPTCB->ptcb->wait_var);
  *exitval = tempPTCB->ptcb->exitval;
	return 0;

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
  
}

