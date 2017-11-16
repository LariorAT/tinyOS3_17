
#include "tinyos.h"
<<<<<<< HEAD
#include "kernel_streams.h"
#include "kernel_dev.h"
#include "kernel_proc.h"
#include "kernel_cc.h"

#define SIZE_OF_BUFFER 8192

int pipe_read(void* this, char *buf, unsigned int size);
int pipe_reader_close(void* this);
int pipe_write(void* this, const char* buf, unsigned int size);
int pipe_writer_close(void* this);
int pipe_Null();

static file_ops reader_fops = {
  .Open = NULL,
  .Read = pipe_read,
  .Write = pipe_Null,
  .Close = pipe_reader_close
};


static file_ops writer_fops = 
{
  .Open = NULL,
  .Read = pipe_Null,
  .Write = pipe_write,
  .Close = pipe_writer_close
};


typedef struct Pipe_control_block{
	char* buffer ;
	CondVar noSpace;
	CondVar hasNoData;
	FCB * reader;
	FCB * writer;
	int wP; /**reader pointer position*/
	int rP; /**writer pointer position*/


}PPCB;

PPCB* initialize_Pipe(pipe_t* pipe)	
{
	PPCB* p = xmalloc(sizeof(PPCB));
	p->buffer = xmalloc(SIZE_OF_BUFFER);
	p->noSpace = COND_INIT;
	p->hasNoData = COND_INIT;

	FCB** fcb = xmalloc(2*sizeof(FCB*));
	Fid_t *fid = xmalloc(2*sizeof(Fid_t*));

	if(FCB_reserve(2,fid,fcb)==0){
		//fprintf(stderr, "Could not reserve Fcbs \n");
		return NULL;
	}
	
	p->reader = fcb[0];
	p->writer = fcb[1];

	p->reader->streamfunc = &reader_fops;
	p->writer->streamfunc = &writer_fops;

	p->reader->streamobj = p;
	p->writer->streamobj = p;

	pipe->read = fid[0];
	pipe->write = fid[1];

	p->wP =0;
	p->rP = -1;
	return p;

}

int pipe_read(void* this, char *buf, unsigned int size)
{
	PPCB* p = (PPCB*) CURPROC->FIDT[(int)this]->streamobj;

	if (size>SIZE_OF_BUFFER)
		return -1;

	if(p->rP ==-1){
		return -1;
	}
	int i =0;

	while (i<size){

		if(p->buffer[p->rP] == 0){ /*0 indicates the end of stream*/  ///may need to check if i need to put 0 manually at pipe_write
			return i;
		}
		
		if (p->rP != p->wP){
			buf[i] = p->buffer[p->rP];
			p->rP ++;
			i++;
		}else if (p->rP == p->wP)
		{
			kernel_wait(& p->hasNoData,SCHED_IO);
		}

		if(p->rP == SIZE_OF_BUFFER){
			p->rP =0;
		}
	}

	return i;
}
int pipe_write(void* this, const char* buf, unsigned int size)
{
	//TO check if *this doesnt exist and return -1
	if (size>SIZE_OF_BUFFER)
		return -1;
	
	PPCB* p = (PPCB*) CURPROC->FIDT[(int)this]->streamobj; //MAY need to check (int)this
	//int freeSpace = find_bufferSpace(p->rP,p->wP);
	int i =0;
	while(i<size){
		
		if (p->rP != p->wP){
			p->buffer[p->wP] = buf[i];
			p->wP ++;
			i++;
		}else if (p->rP == p->wP)
		{
			kernel_wait(& p->noSpace,SCHED_IO);
		}

		if(p->wP == SIZE_OF_BUFFER){
			p->wP =0;
		}
		
	}
	return i;
}

int pipe_reader_close(void* this)
{
	return 0;
}
int pipe_writer_close(void* this)
{
	return 0;
}
/*Returns -1*/
int pipe_Null()
{
	return -1;
}
/**Returns the free space of the pipe
int find_bufferSpace(int rP,int wP)
{
	if (rP > wP)
		return rP-wP;
	else
		return SIZE_OF_BUFFER - (wP-rP);
}*/

int sys_Pipe(pipe_t* pipe)
{
	PPCB* p;

	p = initialize_Pipe(pipe);
	if (p == NULL){
		return -1;
	}

	return 0;

=======


int sys_Pipe(pipe_t* pipe)
{
	return -1;
>>>>>>> e65222d39cf9f36dff561b81edd0b91bab8cd4f6
}

