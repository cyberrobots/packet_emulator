#include "p_emu_libs.h"
#include "p_emu_dbg.h"
#include "p_emu_core.h"
#include "p_emu_help.h"
#include "p_emu_math.h"

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

    if(!(stream->filter.flags & FILTERING_IS_ENABLED)) {
            return P_EMU_KEEP_PACKET;
    }

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
#define LOSS_ACCURACY (100000) // three decimal digits
	
	int result = P_EMU_KEEP_PACKET;
	int tempRand = 0;
	
	if(stream->loss.flags & LOSS_IS_ENABLED){
		P_ERROR(DBG_INFO,"Packet loss is enabled for this stream {%s} -> %f",stream->stream_name,stream->loss.percentage);
		tempRand = rand() % (LOSS_ACCURACY + 1);
		if(tempRand < ((stream->loss.percentage / 100) * LOSS_ACCURACY )){
			result = P_EMU_DISCARD_PACKET;
		}
	}
	
	return result;
}

/* Calculate packet's delay based stream's configuration */
void p_emu_delay_calculate(struct p_emu_stream *stream,struct p_emu_packet *pack)
{
	/* ToDo: Add delay Calculation */

	P_ERROR(DBG_INFO,"___Calculate delay___");

	uint32_t delay_type = (stream->delay.flags & ~DELAY_IS_ENABLED);

	switch(delay_type)
	{
		case DELAY_IS_STATIC:
		{
			P_ERROR(DBG_INFO,"___STATIC_DELAY___");
			pack->leave.tv_sec = pack->arrival.tv_sec +
					     stream->delay.st_delay.d.tv_sec;
			pack->leave.tv_nsec = pack->arrival.tv_nsec +
					      stream->delay.st_delay.d.tv_nsec;
			break;
		}
        case DELAY_IS_UNIFORM:
        {

            P_ERROR(DBG_INFO,"___UNIFORM_DELAY___");
            long delay = uniform((void *)&stream->delay.un_delay);

			pack->leave.tv_sec = pack->arrival.tv_sec;

            pack->leave.tv_nsec = pack->arrival.tv_nsec + delay;


            break;
        }
        case DELAY_IS_GAUSSIAN:
        {
            P_ERROR(DBG_INFO,"___DELAY_IS_GAUSSIAN___");

            long delay = rand_normal((void *)&stream->delay.ga_delay);

			pack->leave.tv_sec = pack->arrival.tv_sec;

            pack->leave.tv_nsec = pack->arrival.tv_nsec + delay;


            break;
        }

		default:
			break;
	}


	/* Make sure that nsec is less than 10^9 */

	while(pack->leave.tv_nsec > 1000000000){
		pack->leave.tv_sec++;
		pack->leave.tv_nsec-=1000000000;
	}

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

int pack_emu_sort_insert(void* newNode, void* currNode,
			     void* prevNode, void* nextNode)
{
#define LESS(m,n)       timercmp(m,n,<)
#define GREATER(m,n)    timercmp(m,n,>)
#define EQUAL(m,n)      timercmp(m,n,==)

    struct timeval nVal;          // Node Inserted now
    struct timeval cVal;          // Node that the list points
    //struct timeval pVal;         // Node previous of the tmp
    //struct timeval nxVal;         // Node next of the next
    int result = INSERT_INVALID;

    //P_EMU_UNUSED(nVal);
    //P_EMU_UNUSED(cVal);
    //P_EMU_UNUSED(pVal);
    //P_EMU_UNUSED(nxVal);

	uint8_t nextExists = 0;
	uint8_t prevExists = 0;
    struct timespec nleave = ((struct p_emu_packet *)(((slib_node_t*)newNode)->data))->leave;
    struct timespec cleave = ((struct p_emu_packet *)(((slib_node_t*)currNode)->data))->leave;
    //struct timespec pleave = ((struct p_emu_packet *)(((slib_node_t*)prevNode)->data))->leave;
    //struct timespec nxleave = ((struct p_emu_packet *)(((slib_node_t*)nextNode)->data))->leave;

    if(prevNode){
        //TIMESPEC_TO_TIMEVAL(pVal,pleave);
		prevExists=1;
    }

    if(nextNode){
        //TIMESPEC_TO_TIMEVAL(nxVal,nxleave);
		nextExists =1;
    }

    TIMESPEC_TO_TIMEVAL(cVal,cleave);
    TIMESPEC_TO_TIMEVAL(nVal,nleave);

    if((LESS(&nVal,&cVal)) && (prevExists==0)){
        result = INSERT_BEFORE;
    }else
    if((GREATER(&nVal,&cVal) || EQUAL(&nVal,&cVal)) && (nextExists==0)){
        result = INSERT_AFTER;
    }else
    if((GREATER(&nVal,&cVal) || EQUAL(&nVal,&cVal)) && (nextExists==1)){
        result = INSERT_NO;
    }else
    if((LESS(&nVal,&cVal)) && (prevExists==1))
	{
        result = INSERT_BEFORE;
    }else{
        assert(0);
    }


    return result;

#undef LESS
#undef GREATER
#undef EQUAL

}

int p_emu_timer_start(struct p_emu_stream *stream,struct p_emu_packet *pack)
{
	struct itimerspec interval;
	int stat = -1;

	if(!stream || !pack){
		P_ERROR(DBG_ERROR,"Parameters");
		return -1;
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
		P_ERROR(DBG_ERROR,"Failed to start timer [%d][%s]",
			stat,strerror(errno));
		return -1;
	}

	return 0;
}

void p_emu_process_received(void* data, void* node)
{
	struct p_emu_packet *pack = NULL;
	slib_node_t *pack_node = NULL;
	struct p_emu_stream *stream = (struct p_emu_stream *)((slib_node_t *)node)->data;


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
#ifdef P_EMU_USE_SEMS
				p_emu_post_tx_signal();
#else

                /* Push packet to queue */

                int ret = -1;
                uint64_t ptr = (uint64_t)stream;

                P_ERROR(DBG_INFO,"TxQueue_______Sent[%p]_[%lx]",
                    stream,ptr);

                ret = p_emu_tx_msg_queue_send((void*)&ptr,
                                  sizeof(uint64_t));
                if(unlikely(ret<0)){
                    P_ERROR(DBG_ERROR,"Error: \
                        p_emu_tx_msg_queue_send() %s",
                        strerror(ret));
                    assert(0);
                }
#endif
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
#ifndef P_EMU_USE_SEMS
	slib_node_t node;
	uint64_t ptr  = 0;
#endif
	P_EMU_UNUSED(streams);

	while(1)
	{
#ifdef P_EMU_USE_SEMS
		/* wait a generic rx signal */
		p_emu_wait_rx_signal();

		/* Check every stream for packets,
		TODO :  Change that for efficiency */
		slib_func_exec (streams,NULL,p_emu_process_received);
#else
		ptr = 0;

		ssize_t ret = p_emu_rx_msg_queue_wait((void*)&ptr,
						      sizeof(uint64_t));
		if(unlikely(ret<0)){
			P_ERROR(DBG_ERROR,"Error: p_emu_rx_msg_queue_wait() %s",
				strerror(ret));
			assert(0);
		}


		node.data = (struct p_emu_stream *)ptr;

		p_emu_process_received(NULL,(slib_node_t*)&node);
#endif



	}

	return NULL;
}


























#if 0
#error Temp code

    P_EMU_UNUSED(_____packet_comp_line);

    struct timeval tv_new;          // Node Inserted now
    struct timeval tv_tmp;          // Node that the list points
    struct timeval tv_prev;         // Node previous of the tmp
    struct timeval tv_next;         // Node next of the next

    P_EMU_UNUSED(tv_new);
    P_EMU_UNUSED(tv_tmp);
    P_EMU_UNUSED(tv_prev);
    P_EMU_UNUSED(tv_next);



    if(prev){
        TIMESPEC_TO_TIMEVAL(tv_prev,((struct p_emu_packet *)(prev->data))->leave);
    }

    if(next){
        TIMESPEC_TO_TIMEVAL(tv_next,((struct p_emu_packet *)(next->data))->leave);
    }

    TIMESPEC_TO_TIMEVAL(tv_tmp,((struct p_emu_packet *)(tmp->data))->leave);
    TIMESPEC_TO_TIMEVAL(tv_new,((struct p_emu_packet *)(new->data))->leave);

#if 0
//new
static unsigned long invalid=0;
static unsigned long invalid1=0;
static unsigned long before=0;
static unsigned long before1=0;
static unsigned long after=0;
#define LESS(m,n)       timercmp(m,n,<)
#define GREATER(m,n)    timercmp(m,n,>)
#define EQUAL(m,n)      timercmp(m,n,==)


    if (!prev){
        if(GREATER(&tv_new,&tv_tmp) || EQUAL(&tv_new,&tv_tmp)) {
            printf("INSERT_AFTER__[%ld]\r\n",after++);
            return INSERT_NO;
        }else{
            printf("INSERT_BEFORE1__[%ld]\r\n",before1++);
            return INSERT_BEFORE;
        }
    }else{
    }




    if (!next){
        if(GREATER(&tv_new,&tv_tmp) || EQUAL(&tv_new,&tv_tmp)) {
            printf("INSERT_AFTER__[%ld]\r\n",after++);
            return INSERT_AFTER;
        }else{
            printf("INSERT_BEFORE1__[%ld]\r\n",before1++);
            return INSERT_BEFORE;
        }
    }else{
        if(GREATER(&tv_new,&tv_tmp) || EQUAL(&tv_new,&tv_tmp)) {
            printf("INSERT_NO___1___[%ld]\r\n",invalid1++);
            return INSERT_NO;
        }else{
            printf("INSERT_BEFORE__[%ld]\r\n",before++);

            return INSERT_BEFORE;
        }
    }

    printf("No_ValidOperation___[%ld]\r\n",invalid++);
#undef LESS
#undef GREATER
#undef EQUAL


#else
//old
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
#endif
#endif
