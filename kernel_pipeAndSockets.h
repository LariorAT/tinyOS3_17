#ifndef __KERNEL_PIPEANDSOCKETS_H
#define __KERNEL_PIPEANDSOCKETS_H


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

PPCB* initialize_Pipe(pipe_t* pipe,Fid_t* fid,FCB** fcb);

typedef struct Socket_Control_Block SCB;

int socket_Null();
int socket_close(void* this);
int socket_read(void* this, char *buf, unsigned int size);
int socket_write(void* this, char *buf, unsigned int size);
int findFID(FCB* f);
int ShutdownFCB(FCB* fcb,shutdown_mode how);

typedef struct Unbound_Socket
{
	rlnode node;
}USocket;

typedef struct Listener_Socket
{
	CondVar hasRequest;
	rlnode ReqQueue;
}LSocket;

typedef struct PEER_SOCKET{
	SCB* PeerSocket;
	PPCB* pipeSend;
	PPCB* pipeReceive;
}PSocket;


typedef enum{
	UNBOUND_SOCKET,
	LISTENER_SOCKET,
	PEER_SOCKET
}Socket_type;

typedef struct Socket_Control_Block
{
	int fid;
	FCB* fcb;
	port_t port;
	Socket_type type;
	union{
		USocket* USocket;
		LSocket* LSocket;
		PSocket* PSocket;
	};
}SCB;

typedef struct Connection_Request
{
	CondVar condition;
	SCB* socket;
	int accepted;
	rlnode node;
}ConReq;


#endif
