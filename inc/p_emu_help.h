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

void p_emu_usage(void);
void p_emu_version( void );
void p_emu_interfaces( void );

int p_emu_import_settings(const char *filename, slib_root_t *streams);

int p_emu_create_interfaces(slib_root_t *streams);

int p_emu_start(slib_root_t* streams);

#endif /* PACKET_EMU_HELP_H_ */
