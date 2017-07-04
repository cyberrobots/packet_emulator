#include "p_emu_libs.h"
#include "p_emu_dbg.h"
#include "p_emu_core.h"
#include "p_emu_help.h"


void p_emu_update_tx_timers(void* data, slib_node_t* node)
{
	struct p_emu_timer_list* TimList = (struct p_emu_timer_list*) data;
	struct p_emu_stream *stream = (struct p_emu_stream *)node->data;

	if(!(stream->delay.flags & DELAY_IS_ENABLED)){
		return;
	}

	/* Store the biggest socket descriptor. */
	if(stream->timers.tx_timer > TimList->max_timer_fd){
		TimList->max_timer_fd = stream->timers.tx_timer;
		P_ERROR(DBG_INFO,"Updating Max Timer descriptor [%d]",
			stream->timers.tx_timer);
	}

	P_ERROR(DBG_INFO,"Import Socket (%d)",stream->timers.tx_timer);

	/* Import the socket descritor to the fd_set */
	FD_CLR(stream->timers.tx_timer,&TimList->timerfds);
	FD_SET(stream->timers.tx_timer,&TimList->timerfds);

	return;
}

void p_emu_tx_delayed_packet(struct p_emu_stream *stream)
{

	slib_node_t *pack_node = NULL;
	struct p_emu_packet *packet = NULL;
	int len = -1;

	pack_node = slib_return_first(stream->tx_list);
	if(!pack_node){
		/* this stream has no packets ready for tx */
		return;
	}

	packet = (struct p_emu_packet *)pack_node->data;

	len = sendto(stream->config.tx_iface_fd, ( const void *)packet->payload,
		     (size_t)packet->length,0,NULL, 0);

	if(len!=packet->length)
	{
		P_ERROR(DBG_WARN,"Failed Sending packet [%d]",len);
		assert(0);
	}

#ifdef PACK_EMU_INTERVAL_SHOW
	struct timespec leaving;
	memset(&leaving,0,sizeof(struct timespec));
	clock_gettime(CLOCK_REALTIME,&leaving);

	P_ERROR(DBG_INFO,"Timer[%d]___Sending__Tx_S(%d) Len (%d) Time [%lu]s [%lu]ns ___p[%p]",
		stream->timers.tx_timer,
		stream->config.tx_iface_fd,
		packet->length,
		leaving.tv_sec,
		leaving.tv_nsec,
		packet);
	printf("Interval[%lu]<<<<<<<<<<<<<<<<<\r\n",
		(leaving.tv_sec-packet->arrival.tv_sec));
#endif

	p_emu_packet_discard(packet);


	pack_node = NULL; packet = NULL;
	pack_node = slib_show_first(stream->tx_list);
	/* If no first node exists, stop timer otherwise update */
	if(!pack_node)
	{
		//timerfd_settime(stream->timers.tx_timer,0,NULL,NULL);

		P_ERROR(DBG_ERROR,"______NO____PACKET_____");
	}else{

		P_ERROR(DBG_ERROR,"______Re-Arm Timer_____");

		/* Update timer. */
		packet = (struct p_emu_packet *)pack_node->data;
		if(p_emu_timer_start(stream,packet)){
			P_ERROR(DBG_ERROR,"Timer Failed!");
		}
	}

	return;
}

void p_emu_tx_timers(void* data, slib_node_t* node)
{
	struct p_emu_stream *stream = (struct p_emu_stream *)node->data;
	uint64_t exp = 0; int s = 0;

	if(!(stream->delay.flags & DELAY_IS_ENABLED)){
		P_ERROR(DBG_INFO,"No Delays Here....");
		return;
	}

	s = read(stream->timers.tx_timer, &exp, sizeof(uint64_t));
	if (s != sizeof(uint64_t))
	{
		P_ERROR(DBG_ERROR,"Descriptor![%d]",stream->timers.tx_timer);
		P_ERROR(DBG_ERROR,"Timer read Failed! [%d]",s);
		P_ERROR(DBG_ERROR,"Timer read Failed! [%lu]",exp);
		//assert(0);
	}
	else
	{
		/* Tx packet */
		p_emu_tx_delayed_packet(stream);
	}
	return;
}


void* p_emu_TxThread_Delayed(void* params)
{
	struct p_emu_tx_config* cfg = (struct p_emu_tx_config*)params;
	slib_root_t *streams = cfg->streams;

	struct p_emu_timer_list TimList;
	struct timeval SelectTimeout;


	memset(&TimList,0,sizeof(struct p_emu_timer_list));
	FD_ZERO(&TimList.timerfds);
	TimList.max_timer_fd = -1;

	/* Update FD_SET */
	slib_func_exec (streams,&TimList,p_emu_update_tx_timers);

	SelectTimeout.tv_sec	= P_EMU_RX_PATH_TX_TIMER_TIMEOUT;
	SelectTimeout.tv_usec	= 0;

	while(1)
	{
		if(select(TimList.max_timer_fd + 1,&TimList.timerfds,
				NULL,NULL,&SelectTimeout)>=0)
		{

			P_ERROR(DBG_ERROR,"___Timer_Select___");

			slib_func_exec(streams,NULL,p_emu_tx_timers);
		}
		else
		{
			P_ERROR(DBG_ERROR,"___SELECT_ERROR___");
		}

		{
			/* Reset Select */
			slib_func_exec (streams,
					&TimList,p_emu_update_tx_timers);

			SelectTimeout.tv_sec	= P_EMU_RX_PATH_TX_TIMER_TIMEOUT;
			SelectTimeout.tv_usec	= 0;
		}
	}


	return NULL;
}

/* Non Delayed Streams ------------------------------------------------------ */

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
		P_ERROR(DBG_WARN,"Failed Sending packet [%d]__[%s]",
			len,strerror(errno));
		assert(0);
	}


#ifdef PACK_EMU_INTERVAL_SHOW
	struct timespec leaving;
	memset(&leaving,0,sizeof(struct timespec));
	clock_gettime(CLOCK_REALTIME,&leaving);

	P_ERROR(DBG_INFO,"Sending__Tx_S(%d) Len (%d) Time [%lu]s [%lu]ns ___p[%p]",
		stream->config.tx_iface_fd,
		packet->length,
		leaving.tv_sec,
		leaving.tv_nsec,
		packet);
	printf("Interval[%lu][%lu]<<<<<<<<<<<<<<<<<\r\n",
	       (leaving.tv_sec-packet->arrival.tv_sec),
	       (leaving.tv_nsec-packet->arrival.tv_nsec));
#endif

	p_emu_packet_discard(packet);

	return;
}

void* p_emu_TxThread(void* params)
{

	struct p_emu_tx_config* cfg = (struct p_emu_tx_config*)params;
	slib_root_t *streams = cfg->streams;
#ifndef P_EMU_USE_SEMS
	slib_node_t node = {0};
	uint64_t ptr  = 0;
#endif

	P_EMU_UNUSED(streams);

	while(1)
	{
#ifdef P_EMU_USE_SEMS
		/* TODO : Add a queue here in order to send packet directly */
		p_emu_wait_tx_signal();

		slib_func_exec (streams,NULL,p_emu_tx_non_delayed_packet);
#else
		ptr = 0;
		ssize_t ret = p_emu_tx_msg_queue_wait((void*)&ptr,
						      sizeof(uint64_t));
		if(unlikely(ret<0)){
			P_ERROR(DBG_ERROR,"Error: p_emu_rx_msg_queue_wait() %s",
				strerror(ret));
			assert(0);
		}

		node.data = (struct p_emu_stream *)ptr;

		p_emu_tx_non_delayed_packet(NULL,(slib_node_t*)&node);
#endif

	}


	return NULL;
}
