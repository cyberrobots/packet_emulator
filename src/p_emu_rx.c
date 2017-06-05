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
		P_ERROR(DBG_INFO,"MAX__SOCKET [%d]__",sock_list->max_sokcet_fd);
	}

	P_ERROR(DBG_INFO,"___SOCKET [%d]__",stream->config.rx_iface_fd);

	/* Import the socket descritor to the fd_set */
	FD_CLR(stream->config.rx_iface_fd,&sock_list->socketfds);
	FD_SET(stream->config.rx_iface_fd,&sock_list->socketfds);

	return;
}

static uint8_t buff[4096];
void p_emu_rx_packet(void* data, slib_node_t* node)
{
	struct p_emu_stream *stream = (struct p_emu_stream *)node->data;


	int len = 0;

	memset(buff,0, 4096);

	len = recvfrom(stream->config.rx_iface_fd,buff,(size_t)4096,0,NULL,NULL);

	if(len > 0){
		P_ERROR(DBG_INFO,"___SOCKET [%d] RECEIVED DATA [%d]___",
			stream->config.rx_iface_fd,len);
	}

	return;
}


void* p_emu_RxThread(void* params)
{
	struct p_emu_rx_config* cfg = (struct p_emu_rx_config*)params;
	slib_root_t *streams = cfg->streams;
	struct p_emu_socket_list rx_sockets;
	struct timespec tv;
	struct timeval 	timeout;



	/* Initialize FD_SET */
	memset(&rx_sockets,0,sizeof(struct p_emu_socket_list));
	FD_ZERO(&rx_sockets.socketfds);
	rx_sockets.max_sokcet_fd = -1;


	/* Update FD_SET */
	slib_func_exec (streams,&rx_sockets,p_emu_rx_sock_list_update);



	//while(1){sleep(1000);}

	timeout.tv_sec	= 10;
	timeout.tv_usec	= 0;

	while(1)
	{
		if(likely(select(streams->index+1,&rx_sockets.socketfds,NULL,NULL,&timeout) >= 0))
		{
			P_ERROR(DBG_INFO,"___SELECT_EVENT_RECEIVED___");
			slib_func_exec (streams,NULL,p_emu_rx_packet);
		}
		else
		{
			P_ERROR(DBG_ERROR,"___SELECT_ERROR___");
		}


		P_ERROR(DBG_INFO,"___RESET___");

		slib_func_exec (streams,&rx_sockets,p_emu_rx_sock_list_update);

		timeout.tv_sec	= 10;
		timeout.tv_usec	= 0;

	}


	return NULL;
}
