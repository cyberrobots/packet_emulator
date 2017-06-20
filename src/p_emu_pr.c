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

/* Compares two arrays based on the given mask,returns 0 if arrays are identical,
returns the number of different bytes in case of different */

uint32_t p_emu_frame_compare(uint8_t* input,uint8_t* data,uint8_t* mask,size_t len)
{
	uint32_t i=0,f=0;
	uint8_t *c = NULL ,*r = NULL;

	if(!input || !data || !mask || !len){
		goto error;
	}

	c = malloc(len);
	r = malloc(len);

	if(!c || !r){
		goto error;
	}

	for(i=0;i<len;i++)
	{
		c[i] = input[i] ^ data[i];	/* Xor to mark diffs */
		r[i] = c[i] & mask[i];
		f += (r[i]==0)?0:1;
	}

	// clean
	if(c)
		free(c);
	if(r)
		free(r);


	return f;

error:
	if(c)
		free(c);
	if(r)
		free(r);

	return (uint32_t)-1;
}

int p_emu_filter_packet(struct p_emu_stream *stream, struct p_emu_packet *pack)
{
	if(!p_emu_frame_compare(pack->payload,
				stream->filter.filter_key.payload,
				stream->filter.filter_mask.payload,
				stream->filter.filter_mask.length)){
		P_ERROR(DBG_INFO,"P_EMU_KEEP_PACKET___p[%p]",pack);
		return P_EMU_KEEP_PACKET;
	}

	P_ERROR(DBG_INFO,"P_EMU_DISCARD_PACKET___p[%p]",pack);
	return P_EMU_DISCARD_PACKET;
}

/* Calculate if the processed packet should be lost or not,based the
configuration */

int p_emu_loss_check(struct p_emu_stream *stream,struct p_emu_packet *pack)
{
	return P_EMU_KEEP_PACKET;
}

/* Calculate packet's delay based stream's configuration */
void p_emu_delay_calculate(struct p_emu_stream *stream,struct p_emu_packet *pack)
{
	/* ToDo: Add delay Calculation */

	P_ERROR(DBG_INFO,"___Calculate delay___");

	pack->leave.tv_sec = pack->arrival.tv_sec + 100; // static delay
	pack->leave.tv_sec = pack->arrival.tv_nsec;

	return;
}


/*  Timecompare and sectiom, the sc_time_compare_function() decides where the incomming packet
    should be placed at the Tx linked list.
    */
#ifndef TIMESPEC_TO_TIMEVAL
#define	TIMESPEC_TO_TIMEVAL(m, n) do{ \
(m).tv_sec = (n).tv_sec; \
(m).tv_usec = (n).tv_nsec / 1000; \
}while(0);
#endif
//Debug Time compate function
static uint32_t _____packet_comp_line   = 0;

int pack_emu_sort_insert(slib_node_t* new, slib_node_t* tmp,
			     slib_node_t* prev, slib_node_t* next)
{
    struct timeval tv_new;          // Node Inserted now
    struct timeval tv_tmp;          // Node that the list points
    struct timeval tv_prev;         // Node previous of the tmp
    struct timeval tv_next;         // Node next of the next

    if(prev){
        TIMESPEC_TO_TIMEVAL(tv_prev,((struct p_emu_packet *)(prev->data))->leave);
    }

    if(next){
        TIMESPEC_TO_TIMEVAL(tv_next,((struct p_emu_packet *)(next->data))->leave);
    }

    TIMESPEC_TO_TIMEVAL(tv_tmp,((struct p_emu_packet *)(tmp->data))->leave);
    TIMESPEC_TO_TIMEVAL(tv_new,((struct p_emu_packet *)(new->data))->leave);

    if(prev && next)
    {
        if(timercmp(&tv_new,&tv_tmp,<) &&
			(timercmp(&tv_new,&tv_prev,>) ||
			 timercmp(&tv_new,&tv_prev,==)) )
	{
            _____packet_comp_line = __LINE__;
            return INSERT_BEFORE;
        }else if(timercmp(&tv_new,&tv_tmp,>)){//&& timercmp(&tv_new,&tv_prev,<=)
            _____packet_comp_line = __LINE__;
            return INSERT_AFTER;
        }else if(timercmp(&tv_new,&tv_tmp,==)){
            _____packet_comp_line = __LINE__;
            return INSERT_AFTER;
        }else{
            _____packet_comp_line = __LINE__;
            return INSERT_NO;
        }
    }else if(!prev && next){
        if(timercmp(&tv_new,&tv_tmp,>) &&
			(timercmp(&tv_new,&tv_next,<) ||
			 timercmp(&tv_new,&tv_next,=)))
	{ // && timercmp(&tv_new,&tv_next,<=)
            _____packet_comp_line = __LINE__;
            return INSERT_AFTER;
        }else if(timercmp(&tv_new,&tv_tmp,<)) {
            _____packet_comp_line = __LINE__;
            return INSERT_BEFORE;
        }else if(timercmp(&tv_new,&tv_tmp,==)){
            _____packet_comp_line = __LINE__;
            return INSERT_AFTER;
        }else{
            _____packet_comp_line = __LINE__;
            return INSERT_NO;
        }
    }else if(prev  && !next){
        if(timercmp(&tv_new,&tv_tmp,<) &&
			(timercmp(&tv_new,&tv_prev,>) ||
			 timercmp(&tv_new,&tv_prev,==)))
	{ //&& timercmp(&tv_new,&tv_prev,>=)
            _____packet_comp_line = __LINE__;
            return INSERT_BEFORE;
        }else if(timercmp(&tv_new,&tv_tmp,>)) {
            _____packet_comp_line = __LINE__;
            return INSERT_AFTER;
        }else if(timercmp(&tv_new,&tv_tmp,==)){
            _____packet_comp_line = __LINE__;
            return INSERT_AFTER;
        }else{
            _____packet_comp_line = __LINE__;
            return INSERT_NO;
        }
    }else if(!prev && !next){
        if(timercmp(&tv_new,&tv_tmp,<)){
            _____packet_comp_line = __LINE__;
            return INSERT_BEFORE;
        }else if(timercmp(&tv_new,&tv_tmp,>)) {
            _____packet_comp_line = __LINE__;
            return INSERT_AFTER;
        }else if(timercmp(&tv_new,&tv_tmp,==)){
            _____packet_comp_line = __LINE__;
            return INSERT_AFTER;
        }else{
            _____packet_comp_line = __LINE__;
            return INSERT_NO;
        }

    }

    return INSERT_NO;
}

int p_emu_timer_start(struct p_emu_stream *stream,struct p_emu_packet *pack)
{
	struct itimerspec interval;
	int stat = -1;

	if(!stream || !pack){
		P_ERROR(DBG_ERROR,"Parameters");
		return -1;
	}

	/* Stop timer */
	stat = timerfd_settime(stream->timers.tx_timer,0,NULL,NULL);
	if(stat < 0)
	{
		P_ERROR(DBG_WARN,"Failed to reset [%d]",stat);
	}

	/* Set and Arm timer */
	interval.it_interval.tv_sec  = 0;
	interval.it_interval.tv_nsec = 0;
	interval.it_value.tv_sec  = pack->leave.tv_sec;
	interval.it_value.tv_nsec = pack->leave.tv_nsec;

	stat = timerfd_settime(stream->timers.tx_timer,
				   TFD_TIMER_ABSTIME,&interval,NULL);
	if(stat < 0)
	{
		P_ERROR(DBG_WARN,"Failed to start timer [%d]",stat);
		return -1;
	}

	P_ERROR(DBG_INFO,"Timer __[%d]__ started!",stream->timers.tx_timer);


	return 0;
}

void p_emu_process_received(void* data, slib_node_t* node)
{
	struct p_emu_packet *pack = NULL;
	slib_node_t *pack_node = NULL;
	struct p_emu_stream *stream = (struct p_emu_stream *)node->data;


	if(!slib_get_list_cont(stream->rx_list)){
		P_ERROR(DBG_WARN,"No more packets");
		return;
	}

	pack_node = slib_return_first(stream->rx_list);

	if(!pack_node){
		/*Since we are waiting for rx signal each time,
		this case should never happen */
		P_ERROR(DBG_WARN,"Error, No packets!!!");
		assert(pack_node);
		return;
	}

	pack = (struct p_emu_packet *)pack_node->data;

	{
		/* Packet process */

		if(p_emu_filter_packet(stream,pack)==P_EMU_KEEP_PACKET)
		{
			/* Check if packet loss is enabled and the packet
			should be discarded*/
			if((stream->loss.flags & LOSS_IS_ENABLED) &&
				(p_emu_loss_check(stream,pack)==P_EMU_DISCARD_PACKET))
			{
				P_ERROR(DBG_INFO,"___LOSS_IS_ENABLED_DISCARD__");
				{
					/* packet should be discarded */
					p_emu_packet_discard(pack);
					return;
				}
			}

			/* We insert the packets based the delay they carry,
			if Delay_is_not_enabled we just create a FIFO queue
			*/

			if(stream->delay.flags & DELAY_IS_ENABLED)
			{

				/* Calculate delay time */

				p_emu_delay_calculate(stream,pack);

				/* Insert the packet based it's delay. */

				if(slib_insert_and_sort(stream->tx_list,
							pack_node,NULL,
							pack_emu_sort_insert)
						!=LIST_OP_SUCCESS)
				{
					P_ERROR(DBG_WARN,
						"__Importing frame failed!__");
					p_emu_packet_discard(pack);
					return;
				}

				pack_node = NULL; pack = NULL;

				pack_node = slib_show_first(stream->tx_list);
				if(!pack_node){
					assert(pack_node);
				}

				pack = (struct p_emu_packet *)pack_node->data;

				/* Start or just update timer. */

				if(p_emu_timer_start(stream,pack)){
					P_ERROR(DBG_ERROR,"Timer Failed!");
				}

			}else{
				/* Prepare the packet for TX. */
				if(slib_list_add_last_node(stream->tx_list,
							   pack_node)
						!=LIST_OP_SUCCESS)
				{
					P_ERROR(DBG_WARN,
						"__Importing frame failed__");
					p_emu_packet_discard(pack);
					return;
				}

				p_emu_post_tx_signal();
			}

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

		/* Check every stream for packets,
		TODO :  Change that for efficiency */
		slib_func_exec (streams,NULL,p_emu_process_received);

	}

	return NULL;
}

