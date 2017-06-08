#include "p_emu_libs.h"
#include "p_emu_dbg.h"
#include "p_emu_core.h"
#include "p_emu_help.h"

#ifdef P_EMU_KEEP_PACKET
#undef P_EMU_KEEP_PACKET
#define P_EMU_KEEP_PACKET (0)
#else
#define P_EMU_KEEP_PACKET (0)
#endif

#ifdef P_EMU_DISCARD_PACKET
#undef P_EMU_DISCARD_PACKET
#define P_EMU_DISCARD_PACKET (1)
#else
#define P_EMU_DISCARD_PACKET (1)
#endif


int p_emu_filter_packet(struct p_emu_stream *stream, struct p_emu_packet *pack)
{

	return P_EMU_DISCARD_PACKET;
}


void p_emu_process_received(void* data, slib_node_t* node)
{
	struct p_emu_stream *stream = (struct p_emu_stream *)node->data;


	if(!slib_get_list_cont(stream->rx_list)){
		P_ERROR(DBG_WARN,"No more packets");
		return;
	}

	P_ERROR(DBG_WARN,"Stream [%s]__Sock[%d] Packets ready (%d)",
		stream->stream_name,
		stream->config.rx_iface_fd,
		slib_get_list_cont(stream->rx_list));

	slib_node_t *pack_node = slib_return_first(stream->rx_list);

	if(!pack_node){
		/*Since we are waiting for rx signal each time,
		this case should never happen */
		P_ERROR(DBG_WARN,"Error, No packets!!!");
		assert(pack_node);
		return;
	}

	struct p_emu_packet *pack = (struct p_emu_packet *)pack_node->data;

	{
		/* Packet process */

		if(p_emu_filter_packet(stream,pack)==P_EMU_KEEP_PACKET){
		}else{
			/* Not interested on that, discard them */
			p_emu_packet_discard(pack);
		}


	}

	return;
}


void* p_emu_PrThread(void* params)
{

	struct p_emu_pr_config* cfg = (struct p_emu_pr_config*)params;
	slib_root_t *streams = cfg->streams;

	while(1)
	{
		/* wait a generic rx signal */
		p_emu_wait_rx_signal();

		P_ERROR(DBG_WARN,"_____PrThread_spin____");

		/* Check every stream for packets,
		TODO :  Change that for efficiency */
		slib_func_exec (streams,NULL,p_emu_process_received);

	}

	return NULL;
}
