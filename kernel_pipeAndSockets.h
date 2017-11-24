#ifndef __KERNEL_PIPE_H
#define __KERNEL_PIPE_H


#define SIZE_OF_BUFFER 8192

int pipe_read(void* this, char *buf, unsigned int size);
int pipe_reader_close(void* this);
int pipe_write(void* this, const char* buf, unsigned int size);
int pipe_writer_close(void* this);
int pipe_Null();

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


#endif
