#ifndef PACKET_EMU_CORE_H_
#define PACKET_EMU_CORE_H_

#include "p_emu_libs.h"
#include "p_emu_consts.h"

#define P_EMU_MAJOR_V	"0"
#define P_EMU_MINOR_V	"0.1"
#define P_EMU_RELEASE_V	"a"

#define P_EMU_VERSION "v"P_EMU_MAJOR_V"."P_EMU_MINOR_V P_EMU_RELEASE_V

/* Filter ------------------------------------------------------------------- */

struct p_emu_filter_key {
	uint16_t 	length;
#define filter_dst_mac payload[0];/*Size ETHER_ADDR_LEN*/
#define filter_src_mac payload[P_EMU_ETHER_ADDR_LEN];/*Size ETHER_ADDR_LEN*/
#define filter_proto   payload[2 * P_EMU_ETHER_ADDR_LEN];/*Size ETHER_TYPE_LEN*/
	uint8_t 	payload[1600];
}__attribute__ ((__packed__));

struct p_emu_filter_mask {
	uint16_t 	length;
	uint8_t		payload[1600];
}__attribute__ ((__packed__));

enum p_emu_filter_flags {
	FILTERING_IS_ENABLED = 1,
	FILTER_DST_MAC_ENABLED = 1 << 1,
	FILTER_SRC_MAC_ENABLED = 1 << 2,
};

struct p_emu_filter_config {
	uint32_t flags;
	struct p_emu_filter_key 	filter_key;
	struct p_emu_filter_mask 	filter_mask;
};

/* Delay -------------------------------------------------------------------- */

enum p_emu_delay_flags {
	DELAY_IS_ENABLED = 1,
	DELAY_IS_UNIFORM  = 1 << 1,
	DELAY_IS_GAUSSIAN = 1 << 2,
};


struct p_emu_delay_config {
	uint32_t	flags;
};

/* Stream Config ------------------------------------------------------------ */

struct p_emu_stream_config {
	uint8_t		rx_iface[P_EMU_IFACE_NAME_LEN + 1];
	uint16_t	rx_iface_fd;
	uint8_t		tx_iface[P_EMU_IFACE_NAME_LEN + 1];
	uint16_t	tx_iface_fd;
};

/* Stream ---------------------------------------------------------------------/
 Packet emulation stream, basic configuration for each stream regarding the
 datapath, the filtering and the delay options.
 stream_name : Stream's name.
 config : Datapath configuration.
 filter : Filtering configuration.
 delay  : Delay configuration.
 rx_list: Received pakcets list.
 rx_list_max_size : Maximum rx list size allowed.
 tx_list: Ready for tx packets list.
 tx_list_max_size : Maximum rx list size allowed.
/-----------------------------------------------------------------------------*/

struct p_emu_stream {
	char		stream_name[P_EMU_STREAM_NAME_LEN];
	uint16_t	stream_name_len;
	struct		p_emu_stream_config config;
	struct		p_emu_filter_config filter;
	struct		p_emu_delay_config delay;
	slib_root_t	*rx_list;
	uint16_t	rx_list_max_size;
	slib_root_t	*tx_list;
	uint16_t	tx_list_max_size;
};

struct p_emu_socket_list {
	fd_set	socketfds;
	int	max_sokcet_fd;
};


struct p_emu_rx_config {
	slib_root_t *streams;
};

struct p_emu_tx_config {
	slib_root_t *streams;
};

struct p_emu_packet {
	slib_node_t node;
	int length;		/* Payload length. */
	struct timespec tv;	/* Time packet arrived. */
	struct timespec leave;	/* Time packet supposed to leave. */
	uint8_t payload[0];	/* Payload */
};




#endif /* PACKET_EMU_CORE_H_ */
