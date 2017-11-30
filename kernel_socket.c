
#include "tinyos.h"
#include "kernel_streams.h"
#include "kernel_dev.h"
#include "kernel_proc.h"
#include "kernel_cc.h"
#include "kernel_pipeAndSockets.h"

/* The port table */
SCB* PORT_MAP[MAX_PORT+1];




int socket_read(void* this, char *buf, unsigned int size)
{
	SCB* s = (SCB*)this;
	if(s->type != PEER_SOCKET)
	{
		fprintf(stderr, "Only Peers can Read\n");
		return -1;
	}

	return pipe_read(s->PSocket->pipeReceive,buf,size);
}

int socket_write(void* this, char *buf, unsigned int size)
{
	SCB* s = (SCB*)this;
	if(s->type != PEER_SOCKET)
	{
		fprintf(stderr, "Only Peers can Write\n");
		return -1;
	}

	return pipe_write(s->PSocket->pipeSend,buf,size);
}
int socket_close(void* this)
{
	SCB* s = (SCB*) this;
	if(s->type == LISTENER_SOCKET)
	{
		PORT_MAP[s->port] = 0;
		kernel_broadcast(&s->LSocket->hasRequest);
	
		PORT_MAP[s->port] = 0;
		free(s->LSocket);
		free(s);	
	}

	return 0;
}

static file_ops socket_fops = 
{
  .Open = NULL,
  .Read = socket_read,
  .Write = socket_write,
  .Close = socket_close
};


Fid_t sys_Socket(port_t port )
{	
	

	
	/** Check if it is a valid port*/
	if (port > MAX_PORT || port < 0){  
		//fprintf(stderr, "NOFILE\n" );
		return NOFILE;
	}
	
	FCB** fcb = xmalloc(sizeof(FCB*));
	Fid_t* fid = xmalloc(sizeof(Fid_t));


	if(FCB_reserve(1,fid,fcb) == 0){
		//fprintf(stderr, "NOFILE\n" );
		return NOFILE;
	}


	//fprintf(stderr, "SOcket\n" );
	SCB* s = xmalloc(sizeof(SCB));
	s->ref_count = 1;
	s->fcb = fcb[0];
	

	s->port = port;
	s->type = UNBOUND_SOCKET;
	s->USocket = xmalloc(sizeof(USocket));
	 
	rlnode_init(& s->USocket->node,s);

	fcb[0]->streamobj = s;
	fcb[0]->streamfunc = &socket_fops;
	
	return fid[0];
	
}
int socket_Null()
{
	return -1;
}

int sys_Listen(Fid_t sock)//the socket has already been initialized*
{
	fprintf(stderr, "Listener\n");

	FCB* f = get_fcb(sock);

	if(sock<0 || sock >= MAX_FILEID){
		fprintf(stderr, "Invalid FID\n" );
		return -1;
	}
	/**Checking if socket exists*/
	if(f == NULL){
		fprintf(stderr, "Socket doesn't exist\n" );
		return -1;
	}
	SCB* s= f->streamobj;
	/**Checking if s is a valid object*/
	if(s== NULL){
		fprintf(stderr, "FCB has invalid streamobj\n" );
		return -1;
	}
	/**Checking if s->port is a valid port*/
	if(s->port == NOPORT){
		fprintf(stderr, "Invalid Port\n" );
		return -1;
	}

	/**Checking if the port is bound to another Listener*/
	if(PORT_MAP[s->port] != 0  )
	{	
		if (PORT_MAP[s->port]->type == LISTENER_SOCKET){
			fprintf(stderr, "POrt is bound to another Listener\n" );
			return -1;
		}
	}
	/**Checking if this socket is already initialized*/
	if(s->type == LISTENER_SOCKET || s->type == PEER_SOCKET)
	{
		fprintf(stderr, "Listener already initialized\n" );
		return -1;
	}

	free(s->USocket); /** freeing the old unbound socket*/

	s->LSocket = xmalloc(sizeof(LSocket));
	s->LSocket->hasRequest = COND_INIT;
	rlnode_init(& s->LSocket->ReqQueue,NULL);
	s->type = LISTENER_SOCKET;
	PORT_MAP[s->port] = s;

	fprintf(stderr, "-------------End\n");




	return 0;
}


Fid_t sys_Accept(Fid_t lsock)
{
	fprintf(stderr, "Accept\n");

	FCB* f = get_fcb(lsock);

	if(lsock<0 || lsock >= MAX_FILEID){
		fprintf(stderr, "Invalid FID\n" );
		return NOFILE;
	}

	/**Checking if socket exists*/
	if(f == NULL){
		fprintf(stderr, "Socket doesn't exist\n" );
		return NOFILE;
	}
	SCB* ls= f->streamobj;
	/**Checking if ls is a valid object*/
	if(ls == NULL){
		fprintf(stderr, "FCB has invalid streamobj\n" );
		return NOFILE;
	}
	/**Checking if lsock is initialized*/
	if(ls->type != LISTENER_SOCKET)
	{
		fprintf(stderr, "Unitialized Listener\n" );
		return NOFILE;
	}

	FCB** fcb = xmalloc(sizeof(FCB*));
	Fid_t* fid = xmalloc(sizeof(Fid_t));


	if(FCB_reserve(1,fid,fcb) == 0){
		fprintf(stderr, "Maximum Fid, reached\n" );
		return NOFILE;
	}
	
	SCB* u = xmalloc(sizeof(SCB));
	u->ref_count = 1;
	u->fcb = fcb[0];
	

	u->port = ls->port;
	u->type = UNBOUND_SOCKET;
	u->USocket = xmalloc(sizeof(USocket));
	 
	rlnode_init(& u->USocket->node,u);

	fcb[0]->streamobj = u;
	fcb[0]->streamfunc = &socket_fops;

	fprintf(stderr, "Accept Waiting\n");
	kernel_wait(& ls->LSocket->hasRequest,SCHED_IO);
	fprintf(stderr, "Accept WAKING UP\n");
	if (ls->port == 0){ //////////////////////////////////////////////////////////////
		fprintf(stderr, "Listener Closed///////////////////////////////////////////////\n" );
		return NOFILE;
	}

	ConReq* connect_req = (ConReq*)rlist_pop_front(& ls->LSocket->ReqQueue)->obj;
	
	
	if(connect_req == NULL){
		return NOFILE;

	}
	fprintf(stderr, "2\n");
	connect_req->accepted = 1;
	SCB* connect_socket = connect_req->socket;

	

	 
	fprintf(stderr, "\n");
	
	connect_socket->type = PEER_SOCKET;
	
	free(connect_socket->USocket);
	
	connect_socket->PSocket = xmalloc(sizeof(PSocket));

	u->type = PEER_SOCKET;
	free(u->USocket); /** freeing the old unbound socket*/
	u->PSocket = xmalloc(sizeof(PSocket));

	u->PSocket->PeerSocket = connect_socket;
	connect_socket->PSocket->PeerSocket  = u;

	

	pipe_t* pipe = xmalloc(2*sizeof(pipe_t));

	Fid_t fid1[2];
	fid1[0] = fid[0];
	fid1[1] = findFID(connect_socket->fcb);

	FCB* fcb1[2];
	fcb1[0] = acquire_FCB();
	fcb1[1] = acquire_FCB();

	if (fcb1[0] == NULL || fcb1[1] == NULL){
		return NOFILE;
	}
	fprintf(stderr, "2.4\n");
	PPCB* p1 = initialize_Pipe(&pipe[0],fid1,fcb1);	

	u->PSocket->pipeSend = p1;
	connect_socket->PSocket->pipeReceive = p1;

	fid1[0] = findFID(connect_socket->fcb);
	fid1[1] = fid[0];

	fcb1[0] = acquire_FCB();
	fcb1[1] = acquire_FCB();

	if (fcb1[0] == NULL || fcb1[1] == NULL){
		return NOFILE;
	}


	PPCB* p2 = initialize_Pipe(&pipe[1],fid1,fcb1);

	connect_socket->PSocket->pipeSend = p2;
	u->PSocket->pipeReceive = p2;


	return fid[0];
}


int sys_Connect(Fid_t sock, port_t port, timeout_t timeout)
{
	fprintf(stderr, "Connect\n");

	FCB* f = get_fcb(sock);

	if(sock<0 || sock >= MAX_FILEID){
		fprintf(stderr, "Invalid FID\n" );
		return -1;
	}

	/**Checking if socket exists*/
	if(f == NULL){
		fprintf(stderr, "Socket doesn't exist\n" );
		return -1;
	}
	SCB* s= f->streamobj;
	/**Checking if s is a valid object*/
	if(s == NULL){
		fprintf(stderr, "FCB has invalid streamobj\n" );
		return -1;
	}

	if(PORT_MAP[port] == 0){
		fprintf(stderr, "Unitialized Listener\n" );
		return -1;
	}
	/**Checking if lsock is initialized*/
	if(PORT_MAP[port]->type != LISTENER_SOCKET)
	{
		fprintf(stderr, "Unitialized Listener\n" );
		return -1;
	}
	SCB* ls = PORT_MAP[port];

	ConReq* req = xmalloc(sizeof(ConReq));
	req->condition = COND_INIT;
	req->socket = s;
	req->accepted = 0;
	rlnode_init(& req->node,req);
	rlist_push_back(& ls->LSocket->ReqQueue,& req->node);

	fprintf(stderr, "1, \n");

	kernel_signal(& ls->LSocket->hasRequest);
	kernel_timedwait(& req->condition,SCHED_IO,timeout);

	fprintf(stderr, "1.2\n");
	if(req->accepted == 0){
		fprintf(stderr, "Timeout, Connect Failed\n" );
		return -1;
	}

	return 0;
}


int sys_ShutDown(Fid_t sock, shutdown_mode how)
{
	fprintf(stderr, "HIIIIIIIIIIIIIII\n");
	return -1;
}

int findFID(FCB* f)
{
	PCB* p = CURPROC;
	FCB* fcb = NULL;

	for(int i=0;i<MAX_FILEID;i++) {
		fcb = p->FIDT[i];

    	if(fcb != NULL) 
    	{
      		if(fcb == f)
      		{
      			return i;
      		}
      		
    	}
  	}
  	fprintf(stderr, "ERRROR ON FIND FID\n");
  	assert(0);
  	return -1;
}
