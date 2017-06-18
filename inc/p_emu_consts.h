#ifndef PACKET_EMU_CONSTS_H_
#define PACKET_EMU_CONSTS_H_

#define P_EMU_UNINITIALIZED	(-1)

/* MAC Address size */
#define P_EMU_ETHER_ADDR_LEN	(6)

/* Interface name size */
#define P_EMU_IFACE_NAME_LEN	(16)


#define P_EMU_MAX_LIST_SIZE	(128)


/* Maximum Allowed stream name */
#define P_EMU_STREAM_NAME_LEN	(64)

 /* Maximum receive timeout on the Rx path (seconds) */
#define P_EMU_RX_PATH_RECEIVE_TIMEOUT (10)

#define P_EMU_RX_PATH_TX_TIMER_TIMEOUT (10)


#define P_EMU_JUMBO_ETH_FRAME_SIZE	(65536)

#define P_EMU_ETH_FRAME_SIZE		(1542)

/* Set maximum Frame buffer */
#ifdef P_EMU_JUMBO_FRAME
#define P_EMU_MAX_INPUT_BUFFER_SIZE	(P_EMU_JUMBO_ETH_FRAME_SIZE)
#else
#define P_EMU_MAX_INPUT_BUFFER_SIZE	(P_EMU_ETH_FRAME_SIZE)
#endif



#endif /* PACKET_EMU_CONSTS_H_ */
