
#include "tinyos.h"
#include "kernel_streams.h"
#include "kernel_dev.h"
#include "kernel_proc.h"
#include "kernel_cc.h"
#include "kernel_pipeAndSockets.h"

/* The port table */
SCB* PORT_MAP[MAX_PORT+1];

typedef struct Socket_Control_Block SCB;

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
	PPCB* pepeReceive;
}PSocket;


typedef enum{
	UNBOUND_SOCKET,
	LISTENER_SOCKET,
	PEER_SOCKET
}Socket_type;

typedef struct Socket_Control_Block
{
	int ref_count;
	FCB* fcb;
	port_t port;
	Socket_type type;
	union{
		USocket* USocket;
		LSocket* LSocket;
		PSocket* PSocke;
	};
}SCB;
typedef struct Connection_Request
{
	CondVar condition;
	SCB* socket;
	int accepted;
	rlnode node;
}ConReq;

static file_ops socket_fops = 
{
  .Open = NULL,
  .Read = NULL,
  .Write = NULL,
  .Close = NULL
};

Fid_t sys_Socket(port_t port)
{
	return NOFILE;
}

int sys_Listen(Fid_t sock)
{
	return -1;
}


Fid_t sys_Accept(Fid_t lsock)
{
	return NOFILE;
}


int sys_Connect(Fid_t sock, port_t port, timeout_t timeout)
{
	return -1;
}


int sys_ShutDown(Fid_t sock, shutdown_mode how)
{
	return -1;
}

