#include "p_emu_libs.h"
#include "p_emu_dbg.h"
#include "p_emu_core.h"
#include "p_emu_help.h"


#if 0
/* Arm Timer */

if(p_emu_timer_start(stream,pack))
{
	P_ERROR(DBG_WARN,"Failed starting timer!");
	p_emu_packet_discard(pack);
	return;
}
#endif

void p_emu_update_tx_lists(void* data, slib_node_t* node)
{
	slib_node_t *pack_node = NULL;
	struct p_emu_stream *stream = (struct p_emu_stream *)node->data;

	P_ERROR(DBG_INFO,"Packets for Tx_____________[%d]",
		slib_get_list_cont(stream->tx_list));


	pack_node = slib_return_first(stream->tx_list);
	if(!pack_node){
		/*Since we are waiting for rx signal each time,
		this case should never happen */
		P_ERROR(DBG_WARN,"Error, No packets!!!");
		assert(pack_node);
		return;
	}

	struct p_emu_packet *packet = (struct p_emu_packet *)pack_node->data;


	p_emu_packet_discard(packet);

	return;
}


void p_emu_update_tx_timers(void* data, slib_node_t* node)
{

	return;
}


void p_emu_tx_non_delayed_packet(void* data, slib_node_t* node)
{
	struct p_emu_stream *stream = (struct p_emu_stream *)node->data;
	slib_node_t *pack_node = NULL;
	struct p_emu_packet *packet = NULL;
	int len = -1;

	if(stream->delay.flags & DELAY_IS_ENABLED){
		return;
	}

	pack_node = slib_return_first(stream->tx_list);
	if(!pack_node){
		/* this stream has no packets ready for tx */
		return;
	}

	packet = (struct p_emu_packet *)pack_node->data;

	P_ERROR(DBG_INFO,"Sending packet [%d]",packet->length);

	len = sendto(stream->config.tx_iface_fd, ( const void *)packet->payload,
		     (size_t)packet->length,0,NULL, 0);

	if(len!=packet->length)
	{
		P_ERROR(DBG_WARN,"Failed Sending packet [%d]",len);
		assert(0);
	}

	p_emu_packet_discard(packet);

	return;
}

void* p_emu_TxThread(void* params)
{

	struct p_emu_tx_config* cfg = (struct p_emu_tx_config*)params;
	slib_root_t *streams = cfg->streams;

	streams = streams;

	while(1)
	{
		/* TODO : Add a queue here in order to send packet directly */
		p_emu_wait_tx_signal();

		slib_func_exec (streams,NULL,p_emu_tx_non_delayed_packet);

	}


	return NULL;
}
