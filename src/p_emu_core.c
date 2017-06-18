#include "p_emu_libs.h"
#include "p_emu_dbg.h"
#include "p_emu_core.h"
#include "p_emu_help.h"


p_emu_dbg_t __debug_level =	DBG_INFO;

static struct p_emu_rx_config __p_emu_rx_config;

static struct p_emu_tx_config __p_emu_tx_config;

static struct p_emu_pr_config __p_emu_pr_config;


void* p_emu_RxThread(void* params);
void* p_emu_TxThread(void* params);
void* p_emu_PrThread(void* params);
void* p_emu_TxThread_Delayed(void* params);


/* Create socket descriptor ---------------------------------------------------/
interface : Interface's name
rx_tx : Rx interface 0, Tx interface 1
------------------------------------------------------------------------------*/

int _p_emu_create_socket(const char* interface,uint8_t rx_tx)
{
	int ifindex = 0, sockid = 0, err = -1, set = 0;

	struct sockaddr_ll s_addr;
	struct ifreq ifr;

	if(!interface){
		return -1;
	}

	sockid = socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ALL));

	/* Get settings */
	set = fcntl(sockid,F_GETFL, 0);
	if(set < 0){
		return -1;
	}

	/* Set Settings plus NON Blocking flag */
	err = fcntl(sockid, F_SETFL, set | O_NONBLOCK);
	if(err < 0){
		return -1;
	}

	memset(&ifr,0,sizeof(ifr));

 	strncpy(ifr.ifr_name,(const char*)(interface),strlen(interface));
 	/* Get interface's Flags */
 	err = ioctl(sockid,SIOCGIFFLAGS,&ifr);
 	if(err < 0){
		perror("SIOCGIFFLAGS");
		return -1;
	}

	/* Check if the interface is active */
	if(!(ifr.ifr_flags & IFF_UP)){
		return -1;
	}
 	/* Get Interface's index number */
 	err = ioctl(sockid,SIOCGIFINDEX,&ifr);
	if (err < 0){
		perror("SIOCGIFINDEX");
		return -1;
	}

	ifindex=ifr.ifr_ifindex;

	// Just cleanup
	memset(&s_addr,0,sizeof(s_addr));

	s_addr.sll_family 	= PF_PACKET;
	s_addr.sll_hatype 	= 1;
	s_addr.sll_halen  	= ETH_ALEN;
	s_addr.sll_ifindex 	= ifindex;
	s_addr.sll_pkttype 	= PACKET_OTHERHOST;
	s_addr.sll_protocol	= htons(ETH_P_ALL);		//must check this variables
	s_addr.sll_addr[6]	= 0x00;
	s_addr.sll_addr[7]	= 0x00;

	// Bind socket to the interface.
	bind(sockid,(const struct sockaddr *)&s_addr,sizeof(struct sockaddr_ll));


	memset(&ifr,0,sizeof(ifr));
	strncpy(ifr.ifr_name,(const char*)(interface),strlen(interface));
	err = ioctl(sockid,SIOCGIFHWADDR,&ifr);
	if(err < 0){
		perror("SIOCGIFHWADDR");
		return -1;
	}


	P_ERROR(DBG_INFO,"Socket Creation Success: %d",sockid);

	return sockid;
}

void _p_emu_open_fds(void *data, slib_node_t * node)
{
	int fd = -1, *error = (int*) data;

	struct p_emu_stream *stream = (struct p_emu_stream *)node->data;

	/* Create Rx Socket descriptor */
	fd = _p_emu_create_socket((char*)stream->config.rx_iface,0);
	if(fd < 0){
		P_ERROR(DBG_ERROR,"Socket Creation failed!");
		*error = -1;
		return;
	}

	stream->config.rx_iface_fd = fd;

	/* Create Tx Socket descriptor */
	fd = _p_emu_create_socket((char*)stream->config.tx_iface,1);
	if(fd < 0){
		P_ERROR(DBG_ERROR,"Socket Creation failed!");
		*error = -1;
		return;
	}

	stream->config.tx_iface_fd = fd;

	return;
}

int p_emu_create_interfaces(slib_root_t *streams)
{
	int error = 0;

	slib_func_exec (streams,&error,_p_emu_open_fds);

	return error;
}

int p_emu_stop(void* params)
{
	int err = 0;
	/* TODO : terminate packet emulator */
	err = err;




	return err;
}

int p_emu_start(slib_root_t* streams)
{
	int err = 0;

	pthread_t      p_emu_rx_ThreadId;
	pthread_t      p_emu_pr_ThreadId;
	pthread_t      p_emu_tx_ThreadId;
	pthread_t      p_emu_tx_ThreadId_Delay;

	p_emu_init_rx_path();

	p_emu_init_tx_path();

	memset(&__p_emu_rx_config,0,sizeof(struct p_emu_rx_config));
	memset(&__p_emu_tx_config,0,sizeof(struct p_emu_tx_config));
	memset(&__p_emu_pr_config,0,sizeof(struct p_emu_pr_config));

	__p_emu_rx_config.streams = streams;

	__p_emu_tx_config.streams = streams;

	__p_emu_pr_config.streams = streams;

	void 			**RxstatusID = NULL;	// Thread's Return status
	err = pthread_create(&p_emu_rx_ThreadId,
					NULL,
					p_emu_RxThread,
					(void*)&__p_emu_rx_config);

	if(err)
	{
		P_ERROR(DBG_ERROR,"Thread Init failed [%d]",err);
		goto clean;
	}

	void 			**PrstatusID = NULL;	// Thread's Return status
	err = pthread_create(&p_emu_pr_ThreadId,
			     NULL,
			     p_emu_PrThread,
			     (void*)&__p_emu_pr_config);

	if(err)
	{
		P_ERROR(DBG_ERROR,"Thread Init failed [%d]",err);
		goto clean;
	}

	void 			**TxstatusID = NULL;	// Thread's Return status
	err = pthread_create(&p_emu_tx_ThreadId,
					NULL,
					p_emu_TxThread,
					(void*)&__p_emu_tx_config);

	if(err)
	{
		P_ERROR(DBG_ERROR,"Thread Init failed [%d]",err);
		goto clean;
	}


	void 		**TxstatusID_Delay = NULL;	// Thread's Return status
	err = pthread_create(&p_emu_tx_ThreadId_Delay,
			     NULL,
			     p_emu_TxThread_Delayed,
			     (void*)&__p_emu_tx_config);

	if(err)
	{
		P_ERROR(DBG_ERROR,"Thread Init failed [%d]",err);
		goto clean;
	}

clean:
	pthread_join(p_emu_rx_ThreadId,RxstatusID);
	pthread_join(p_emu_pr_ThreadId,PrstatusID);
	pthread_join(p_emu_tx_ThreadId,TxstatusID);
	pthread_join(p_emu_tx_ThreadId_Delay,TxstatusID_Delay);





	return err;
}

struct p_emu_packet * p_emu_packet_init()
{
	struct p_emu_packet *pack = malloc(sizeof(struct p_emu_packet) +
					   P_EMU_MAX_INPUT_BUFFER_SIZE);
	if(!pack)
		return NULL;

	memset(pack,0,sizeof(struct p_emu_packet));

	P_ERROR(DBG_INFO,"New packet______(%p)",pack);
	pack->node.data = pack;
	P_ERROR(DBG_INFO,"New packet______(%p)",pack->node.data);

	return pack;
}

void p_emu_packet_discard(struct p_emu_packet *packet)
{
	if(!packet)
	{
		P_ERROR(DBG_ERROR,"Invalid Pakcet");
		return;
	}
	free(packet);
	packet = (struct p_emu_packet *)NULL;
}

/* Send Signal to process thread -------------------------------------------- */
static sem_t __p_emu_rx_sem;

void p_emu_init_rx_path(void)
{
	memset(&__p_emu_rx_sem,0,sizeof(sem_t));

	if(sem_init(&__p_emu_rx_sem,0,0)<0)
	{
		P_ERROR(DBG_ERROR,"Error: sem_init() %s",strerror(errno));
	}
	return;
}
void p_emu_post_rx_signal(void)
{
	if(sem_post(&__p_emu_rx_sem)<0)
	{
		P_ERROR(DBG_ERROR,"Error: sem_post() %s",strerror(errno));
	}
	return;
}
void p_emu_wait_rx_signal(void)
{
	if(sem_wait(&__p_emu_rx_sem)<0)
	{
		P_ERROR(DBG_ERROR,"Error: sem_wait() %s",strerror(errno));
	}
	return;
}

/* Send Signal to transmit thread ------------------------------------------- */
static sem_t __p_emu_tx_sem;

void p_emu_init_tx_path(void)
{
	memset(&__p_emu_tx_sem,0,sizeof(sem_t));

	if(sem_init(&__p_emu_tx_sem,0,0)<0)
	{
		P_ERROR(DBG_ERROR,"Error: sem_init() %s",strerror(errno));
	}
	return;
}
void p_emu_post_tx_signal(void)
{
	if(sem_post(&__p_emu_tx_sem)<0)
	{
		P_ERROR(DBG_ERROR,"Error: sem_post() %s",strerror(errno));
	}
	return;
}
void p_emu_wait_tx_signal(void)
{
	if(sem_wait(&__p_emu_tx_sem)<0)
	{
		P_ERROR(DBG_ERROR,"Error: sem_wait() %s",strerror(errno));
	}
	return;
}


/*----------------------------------------------------------------------------*/
