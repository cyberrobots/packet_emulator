#include "p_emu_libs.h"
#include "p_emu_dbg.h"
#include "p_emu_core.h"
#include "p_emu_help.h"



void p_emu_rx_sock_list_update(void* data, slib_node_t* node)
{
	struct p_emu_socket_list* sock_list = (struct p_emu_socket_list*) data;
	struct p_emu_stream *stream = (struct p_emu_stream *)node->data;

	/* Store the biggest socket descriptor. */
	if(stream->config.rx_iface_fd > sock_list->max_sokcet_fd){
		sock_list->max_sokcet_fd = stream->config.rx_iface_fd;
		P_ERROR(DBG_INFO,"Updating Max Socket [%d]",
			sock_list->max_sokcet_fd);
	}

	P_ERROR(DBG_INFO,"Import Socket (%d)",stream->config.rx_iface_fd);

	/* Import the socket descritor to the fd_set */
	FD_CLR(stream->config.rx_iface_fd,&sock_list->socketfds);
	FD_SET(stream->config.rx_iface_fd,&sock_list->socketfds);

	return;
}


/* Rx callback function, just receive here the pakcet.
   We are going to decide later if we are going to keep the packet or not.
*/

void p_emu_rx_packet(void* data, slib_node_t* node)
{
	struct p_emu_stream *stream = (struct p_emu_stream *)node->data;

	struct p_emu_packet *pack = p_emu_packet_init();

	if(!pack){
		P_ERROR(DBG_ERROR,"Packet Allocation Failed");
		return;
	}

	pack->length = recvfrom(stream->config.rx_iface_fd,pack->payload,
				(size_t)P_EMU_MAX_INPUT_BUFFER_SIZE,0,NULL,NULL);

	if(likely(pack->length > 0))
	{
		clock_gettime(CLOCK_REALTIME,&pack->arrival);

		P_ERROR(DBG_INFO,"S(%d) Len (%d) Time [%lu]s [%lu]ns ___p[%p]",
			stream->config.rx_iface_fd,
			pack->length,
			pack->arrival.tv_sec,
		        pack->arrival.tv_nsec,
		        pack);

		if(slib_list_add_last_node
				(stream->rx_list,&pack->node)!=LIST_OP_SUCCESS){
			P_ERROR(DBG_WARN,"Importing frame failed!!!");

			p_emu_packet_discard(pack);

			return;
		}
#ifdef P_EMU_USE_SEMS
		p_emu_post_rx_signal();
#else
		uint64_t ptr = (uint64_t)stream;

		P_ERROR(DBG_INFO,"___DataSent[%p]_[%lx]__[%lu]___",stream,ptr,
			sizeof(struct p_emu_stream));

		int ret = -1;

		ret = p_emu_rx_msg_queue_send((void*)stream,
					      sizeof(struct p_emu_stream));
		if(ret<0){
			P_ERROR(DBG_ERROR,"Error: p_emu_rx_msg_queue_send() %s",
				strerror(ret));
			assert(0);
		}
#endif
	}

	return;
}


void* p_emu_RxThread(void* params)
{
	struct p_emu_rx_config* cfg = (struct p_emu_rx_config*)params;
	slib_root_t *streams = cfg->streams;
	struct p_emu_socket_list rx_sockets;
	struct timeval 	timeout;

	/* Initialize FD_SET */
	memset(&rx_sockets,0,sizeof(struct p_emu_socket_list));
	FD_ZERO(&rx_sockets.socketfds);
	rx_sockets.max_sokcet_fd = -1;

	/* Update FD_SET */
	slib_func_exec (streams,&rx_sockets,p_emu_rx_sock_list_update);

	timeout.tv_sec	= P_EMU_RX_PATH_RECEIVE_TIMEOUT;
	timeout.tv_usec	= 0;

	while(1)
	{
		if(likely(select(rx_sockets.max_sokcet_fd + 1,
				 &rx_sockets.socketfds,NULL,NULL,&timeout)>=0))
		{
			slib_func_exec (streams,NULL,p_emu_rx_packet);
		}
		else
		{
			P_ERROR(DBG_ERROR,"___SELECT_ERROR___");
		}

		/* Reset select */
		{
			slib_func_exec (streams,
					&rx_sockets,p_emu_rx_sock_list_update);

			timeout.tv_sec	= P_EMU_RX_PATH_RECEIVE_TIMEOUT;
			timeout.tv_usec	= 0;
		}

	}

	return NULL;
}
