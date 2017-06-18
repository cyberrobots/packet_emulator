#ifndef PACKET_EMU_HELP_H_
#define PACKET_EMU_HELP_H_

/* Compiler definitions for branching */

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)


#define TABLE_SIZE_OF(m)	(sizeof(m)/sizeof(m[0]))


/* Import Callback ---------------------------------------------------------- */
typedef void (*p_emu_import_cb_t)(struct p_emu_stream *stream,char* buffer,int len);

/* Import callback structure ------------------------------------------------ */
typedef struct
{
#define IMPORT_CMD_SIZE 	(128)
	char command[IMPORT_CMD_SIZE];
	p_emu_import_cb_t	func;
}p_emu_import_entry_t;

/* Prototype declaration */

struct p_emu_packet * p_emu_packet_init();

void p_emu_packet_discard(struct p_emu_packet *packet);

int p_emu_import_settings(const char *filename, slib_root_t *streams);

int p_emu_create_interfaces(slib_root_t *streams);

void p_emu_init_rx_path(void);
void p_emu_post_rx_signal(void);
void p_emu_wait_rx_signal(void);

void p_emu_init_tx_path(void);
void p_emu_post_tx_signal(void);
void p_emu_wait_tx_signal(void);

int p_emu_timer_start(struct p_emu_stream *stream,struct p_emu_packet *pack);

#endif /* PACKET_EMU_HELP_H_ */
