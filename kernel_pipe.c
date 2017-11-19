
#include "tinyos.h"
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
	pipe_t* pipe;
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
	Fid_t *fid = xmalloc(2*sizeof(Fid_t));

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

	p->pipe = pipe;
	p->wP =0;
	p->rP = 0;
	return p;

}

int pipe_read(void* this, char *buf, unsigned int size)
{
	//fprintf(stderr, "TRead\n");
	PPCB* p = (PPCB*) this;
	
	
	
	int i =0;

	while (i<size){

		/*When we reach 0 , we end the streaming*/ 
		if(p->buffer[p->rP] == 0 && (i+1)!=size){  
			buf[i] = p->buffer[p->rP];
			//fprintf(stderr, "Reading from:%d word : %c\n",p->rP,p->buffer[p->rP] );
			p->rP ++;
			
			return 0;
		}
		
		if (p->rP != p->wP){
			buf[i] = p->buffer[p->rP];
			//fprintf(stderr, "Reading from:%d word : %c\n",p->rP,p->buffer[p->rP] );
			p->rP ++;
			i++;
		}else if (p->rP == p->wP)
		{
			fprintf(stderr, "Blocked : Reader\n" );
			kernel_wait(& p->hasNoData,SCHED_IO);
			fprintf(stderr, "Waking : Reader\n" );
		}

		if(p->rP == SIZE_OF_BUFFER){
			p->rP = 0;
		}

		kernel_broadcast(& p->noSpace);
	}

	return i;
}
int pipe_write(void* this, const char* buf, unsigned int size) // TO WAKE UP CV
{
	
	PPCB* p = (PPCB*) this;
	if(p == NULL)
		return -1;
	/*It means the Reader end of the pipe is closed*/
	if(p->reader == NULL){
		return -1;
	}
	
	int i =0;
	while(i<size){

		/*In this case we have no space to write data, so we are blocking the thread*/
		if((p->wP + 1)== p->rP || (p->wP == SIZE_OF_BUFFER &&p->rP == 0)){ 
			fprintf(stderr, "Blocked : WRITER\n" );
			kernel_wait(& p->noSpace,SCHED_IO);
			fprintf(stderr, "Waking : WRITER\n" );
		}

		
		p->buffer[p->wP] = buf[i];
		//fprintf(stderr, "Writing to:%d word : %c\n",p->wP,p->buffer[p->wP] );
		p->wP ++;
		i++;
		/*When we reach the end of the array, we reset the pointer to 0*/
		if(p->wP == SIZE_OF_BUFFER){
			p->wP =0;
		}
		
		kernel_broadcast(& p->hasNoData);
	}
	return i;
}

int pipe_reader_close(void* this)
{
	PPCB* p = (PPCB*) this;
	//fprintf(stderr, "TO CLose reader\n" );
	p->reader = NULL;
	free(p->buffer);

	return 0;
}
int pipe_writer_close(void* this)
{
	PPCB* p = (PPCB*) this;
	//fprintf(stderr, "TO CLose writer\n");
	kernel_broadcast(& p->hasNoData);

	
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

}

