#include "p_emu_libs.h"
#include "p_emu_dbg.h"
#include "p_emu_core.h"
#include "p_emu_help.h"

#define MAX_TX_RETRIES (30)

void p_emu_tx_sock_list_update(void* data, void* node)
{
	struct p_emu_socket_list* sock_list = (struct p_emu_socket_list*) data;
	struct p_emu_stream *stream = (struct p_emu_stream *)((slib_node_t*)node)->data;

	/* Store the biggest socket descriptor. */
	if(stream->config.tx_iface_fd > sock_list->max_sokcet_fd){
		sock_list->max_sokcet_fd = stream->config.tx_iface_fd;
		P_ERROR(DBG_INFO,"Updating Max Socket [%d]",
				sock_list->max_sokcet_fd);
	}

	P_ERROR(DBG_INFO,"Import Socket (%d)",stream->config.tx_iface_fd);

	/* Import the socket descritor to the fd_set */
	FD_CLR(stream->config.tx_iface_fd,&sock_list->socketfds);
	FD_SET(stream->config.tx_iface_fd,&sock_list->socketfds);

	return;
}

void p_emu_rx_reverse_packet(void* data, void* node)
{
	struct p_emu_stream *stream = (struct p_emu_stream *)((slib_node_t*)node)->data;
	struct p_emu_packet *pack = p_emu_packet_init();
	int len = -1;
	
	if(!pack){
		P_ERROR(DBG_ERROR,"Packet Allocation Failed");
		return;
	}

	/* Receive packet from Tx interface and send it instantly via RX interface */
	pack->length = recvfrom(stream->config.tx_iface_fd,pack->payload,
							(size_t)P_EMU_MAX_INPUT_BUFFER_SIZE,0,NULL,NULL);
	
	if(pack->length > 0){
	
		int tries = MAX_TX_RETRIES;
		do
		{
			P_ERROR(DBG_INFO,"Sending packet [%d]",pack->length);
			
			len = sendto(stream->config.rx_iface_fd, 
						 ( const void *)pack->payload,(size_t)pack->length,MSG_DONTROUTE,NULL, 0);
			
			P_ERROR(DBG_INFO,"Data Sent [%d]",len);
			
			if(len == pack->length){
				break;
			}

			if(len!= pack->length && errno==EAGAIN){
				tries--;
			}else{
				
				P_ERROR(DBG_WARN,"Failed Sending packet [%d]__[%s][%d]",len,strerror(errno),errno);
				//assert(0);
				break;
			}


		}while(tries);
	}
	
	p_emu_packet_discard(pack);
	
	
	/*
	if(likely(pack->length > 0))
	{
		clock_gettime(CLOCK_REALTIME,&pack->arrival);

		P_ERROR(DBG_INFO,"S(%d) Len (%d) Time [%lu]s [%lu]ns ___p[%p] __packNo[%u]",
				stream->config.rx_iface_fd,
				pack->length,
				pack->arrival.tv_sec,
		        pack->arrival.tv_nsec,
		        pack,rxCounter);
		rxCounter++;
		
		if(slib_list_add_last_node
				(stream->rx_list,&pack->node)!=LIST_OP_SUCCESS){
			P_ERROR(DBG_WARN,"Importing frame failed!!!");

			p_emu_packet_discard(pack);

			return;
		}
#ifdef P_EMU_USE_SEMS
		p_emu_post_rx_signal();
#else
		int ret = -1;
		uint64_t ptr = (uint64_t)stream;

		P_ERROR(DBG_INFO,"___DataSent[%p]_[%lx]__[%lu]___",stream,ptr,
				sizeof(struct p_emu_stream));

		ret = p_emu_rx_msg_queue_send((void*)&ptr,
									  sizeof(uint64_t));
		if(unlikely(ret<0)){
			P_ERROR(DBG_ERROR,"Error: p_emu_rx_msg_queue_send() %s",
					strerror(ret));
			assert(0);
		}
#endif
	}
	*/

	return;
}

void* p_emu_ReversePathThread(void* params)
{
	struct p_emu_rev_config* cfg = (struct p_emu_rev_config*)params;
	slib_root_t *streams = cfg->streams;
	
	struct p_emu_socket_list tx_sockets;
	struct timeval 	timeout;

	/* Initialize FD_SET */
	memset(&tx_sockets,0,sizeof(struct p_emu_socket_list));
	FD_ZERO(&tx_sockets.socketfds);
	tx_sockets.max_sokcet_fd = -1;

	/* Update FD_SET */
	slib_func_exec (streams,&tx_sockets,p_emu_tx_sock_list_update);

	timeout.tv_sec	= P_EMU_RX_PATH_RECEIVE_TIMEOUT;
	timeout.tv_usec	= 0;

	while(1)
	{
		if(likely(select(tx_sockets.max_sokcet_fd + 1,
						 &tx_sockets.socketfds,NULL,NULL,&timeout)>=0))
		{
			slib_func_exec (streams,NULL,p_emu_rx_reverse_packet);
		}
		else
		{
			P_ERROR(DBG_ERROR,"___SELECT_ERROR___");
		}

		/* Reset select */
		{
			slib_func_exec (streams,&tx_sockets,p_emu_tx_sock_list_update);

			timeout.tv_sec	= P_EMU_RX_PATH_RECEIVE_TIMEOUT;
			timeout.tv_usec	= 0;
		}

	}
	
	
	
	return NULL;
}

#undef MAX_TX_RETRIES
