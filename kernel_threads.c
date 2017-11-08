
#include "tinyos.h"
#include "kernel_sched.h"
#include "kernel_proc.h"
#include "kernel_cc.h"






/** 
  @brief Create a new thread in the current process.
  */
Tid_t sys_CreateThread(Task task, int argl, void* args)
{ 
  PCB * curproc = CURPROC;
  PTCB* p = initialize_PTCB();
  
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

  printf("test1\n");
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
  TCB* t = (TCB*)tid;

  /***if the thread we want to join is not exited*/
  if(t->owner_ptcb->isExited==0) 
  {
    int i;
    /**Search for it in the ptcb list*/
   for (i = 0; i < curproc->counter;i++)
    {
      if((Tid_t)tempPTCB->ptcb->main_thread == tid)
        break;
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

    /***If we reached here we can now join the thread*/

    tempPTCB->ptcb->refCounter++; //Update the reference counter of the the thread we join in

    kernel_wait(&tempPTCB->ptcb->wait_var,SCHED_MUTEX); //Waiting

    
    tempPTCB->ptcb->refCounter--; //Update the reference counter
    }

  if(exitval!=NULL)
    *exitval = tempPTCB->ptcb->exitval; //Save the exit value

  if(tempPTCB->ptcb->isExited == 1) /*If the thread is exited*/
    { /*If none is waiting we release the ptcb */
      if(tempPTCB->ptcb->refCounter==0) 
        {
          rlist_remove(tempPTCB);
          free(tempPTCB->ptcb);
          curproc->counter--;
        }
    }

	return 0;
}

/**
  @brief Detach the given thread.
  */
int sys_ThreadDetach(Tid_t tid)
{
  TCB* t = (TCB*)tid;

  /***Checks if the thread can be declared detached ,else returns -1*/
  if(t == NULL || t->state == EXITED || t->owner_ptcb->isExited == 1) 
  {
   return -1; 
  }
  /*We signal all joined threads to cut loose(:p) and declare it detached*/
  kernel_broadcast(&t->owner_ptcb->wait_var);
  t->owner_ptcb->isDetached = 1;
  return 0;
}

/**
  @brief Terminate the current thread.
  */
void sys_ThreadExit(int exitval)
{
  /***Local copies for speed*/
  TCB* current = CURTHREAD; 
  PCB * curproc = CURPROC; 

  /***Incase of Main thread in process we end all the other threads in process*/
  if(current->owner_ptcb==current->owner_pcb->ptcb_list.ptcb ) 
  {
    rlnode* next = &curproc->ptcb_list;
    if(curproc->counter>1)
      next = next->next;
    else
      return;

    /*Join the next thread in ptcb list*/
    sys_ThreadJoin((Tid_t) next->ptcb->main_thread, &exitval);
    sys_ThreadExit(exitval);
  }
  /*for all threads*/
    
  current->owner_ptcb->isExited = 1;
  current->owner_ptcb->refCounter--;
  current->owner_ptcb->exitval = exitval;
  kernel_broadcast(&current->owner_ptcb->wait_var);
  
}

