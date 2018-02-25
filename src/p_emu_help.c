#include "p_emu_libs.h"
#include "p_emu_dbg.h"
#include "p_emu_core.h"
#include "p_emu_help.h"


char* p_emu_dbg_str(p_emu_dbg_t dbg)
{
	switch(dbg)
	{
		case p_emu_dbg_0:
			return "ERROR";
			break;
		case p_emu_dbg_1:
			return "WARN";
			break;
		case p_emu_dbg_2:
			return "INFO";
			break;
		default:
			return "INVALID";
			break;
	}

	return NULL;
}



/* -----------------------------------------------------------------------*
 * This is a basic output, it prints the help menu.
 * --------------------------------------------------------------------- */

void p_emu_usage( void )
{
	P_EMU_INFO("Help :\n"
	     "-v\tVersion.\n"
	     "-h\tHelp.\n"
	     "-f\tRead filter from file.\n"
	     "-l\tList all available interfaces.\n");
	return;
}

/* -----------------------------------------------------------------------*
 * This is a basic output, it prints the version.
 * --------------------------------------------------------------------- */

void p_emu_version( void )
{
	P_EMU_INFO("Version\t: %s\r\n",P_EMU_VERSION);
	return;
}

/* -----------------------------------------------------------------------*
 * This is a basic output, it prints the available interfaces.
 * --------------------------------------------------------------------- */

void p_emu_interfaces( void )
{
	struct ifaddrs *ifap = NULL;
	struct ifaddrs *ptr	 = NULL;

	if( getifaddrs(&ifap) < 0 )
	{
		P_ERROR(DBG_ERROR,"Getting the interfaces failed!");
		return;
	}

	ptr = ifap;
	while(ptr)
	{
		/* Check that the interface is up and AF_PACKET. */
		if( ( ptr->ifa_flags & IFF_UP ) &&
				( ptr->ifa_addr->sa_family == AF_PACKET ) )
		{
			P_EMU_INFO("Interface : [%s]\r\n",ptr->ifa_name);
		}
		ptr = ptr->ifa_next;
	}

	freeifaddrs(ifap);

	return;
}

/* -----------------------------------------------------------------------*
 * This sections reads the configuration files and imports the userdata.
 * --------------------------------------------------------------------- */


/* Configuration delimiters */

#define STREAM_NAME             "Stream="
#define STREAM_DST_MAC          "dst_mac="
#define STREAM_SRC_MAC          "src_mac="
#define STREAM_IN_IFACE         "in_iface="
#define STREAM_OUT_IFACE        "out_iface="
#define STREAM_LOSS_PERCENTAGE  "loss_per="
#define STREAM_STATIC_DELAY     "static_delay:"
#define STREAM_UNIFORM_DELAY    "uniform_delay:"
#define STREAM_NORMAL_DELAY     "normal_delay:"

/* Configuration Callbacks */
#define P_EMU_IMP_FUNC(m)	void(m)(struct p_emu_stream *stream,char* buffer,int len)
P_EMU_IMP_FUNC(import_stream_name);
P_EMU_IMP_FUNC(import_dst_mac);
P_EMU_IMP_FUNC(import_src_mac);
//P_EMU_IMP_FUNC(import_dst_ip);
//P_EMU_IMP_FUNC(import_src_ip);
//P_EMU_IMP_FUNC(import_proto);
/* Import Basic Interface setup. */
P_EMU_IMP_FUNC(import_input_iface);
P_EMU_IMP_FUNC(import_output_iface);
/* Import Distr Attributes. */
P_EMU_IMP_FUNC(import_normal_delay);
//P_EMU_IMP_FUNC(import_exponential);
//P_EMU_IMP_FUNC(import_poisson);
P_EMU_IMP_FUNC(import_uniform_delay);
//P_EMU_IMP_FUNC(import_static);
//P_EMU_IMP_FUNC(import_pareto);
//P_EMU_IMP_FUNC(import_pareto_2);
P_EMU_IMP_FUNC(import_loss);
P_EMU_IMP_FUNC(import_static_delay);


/* Filter Table ------------------------------------------------------------- */
static p_emu_import_entry_t p_emu_import_table[]=
{
	//{STREAM_NAME , import_stream_name },
	{STREAM_DST_MAC , import_dst_mac },
	{STREAM_SRC_MAC , import_src_mac },
	{STREAM_IN_IFACE , import_input_iface },
	{STREAM_OUT_IFACE , import_output_iface },
	{STREAM_LOSS_PERCENTAGE,import_loss },
	{STREAM_STATIC_DELAY,import_static_delay },
	{STREAM_UNIFORM_DELAY,import_uniform_delay },
    {STREAM_NORMAL_DELAY,import_normal_delay},
};

#define NUM_OF_IMPORT_PARAMS ( TABLE_SIZE_OF(p_emu_import_table) )
#define NUM_OF_MAX_VAR  NUM_OF_FILTERS * 2


/* Searches for the "param" string and import any float after that till the
end of line or the MAX_FL_VAL or the EOF or space */
double getFloat(const char* buf, const char* param, size_t len) {

#define MAX_FL_VAL (32)

	char    value[MAX_FL_VAL+1];
	char*   pch = strstr(buf,param);
	int     i=0;
	// not found
	if(!pch){
		return -1.0;
	}

	memset(value,0,MAX_FL_VAL);

	//move the p
	pch = pch + len;

	while(pch!=NULL && *pch!='\n' && *pch!='\r' && *pch!=EOF &&
			*pch!=' ' && i<MAX_FL_VAL)
	{
		value[i] = *pch;
		i++;
		pch++;
	}
	value[i]='\0';

        return atof(value);
}

/* Searches for the "param" string and import any integer after that till the
end of line or the MAX_INT_VAL or the EOF or space */
int getInteger(const char* buf,const char* param,size_t len) {

#define MAX_INT_VAL (12)

	char    value[MAX_INT_VAL+1];
	char*   pch = strstr(buf,param);
	int     i=0;
	//not found
	if(!pch){
		return -1;
	}

	memset(value,0,MAX_INT_VAL);

	//move the p
	pch = pch + len;

	while(pch!=NULL && *pch!='\n' && *pch!='\r' && *pch!=EOF &&
			*pch!=' ' && i< MAX_INT_VAL)
	{
		value[i] = *pch;
		i++;
		pch++;
	}

	value[i]='\0';
	return atoi(value);
}


int getChar(const char* buf, const char* param, char* output, size_t outLen) {

	int i = 0;
	char* pch = NULL;

	if( !(pch = strstr(buf, param) ) ) {
		return -1;
	}

	pch = pch + strlen(param);

	while(pch!=NULL && *pch!='\n' && *pch!='\r' && *pch!=EOF &&
			*pch!=' ' && i<(outLen-1))
	{
		output[i]= *pch;
		i++;
		pch++;
	}

	*(output + i )='\0';

	return i;
}

void import_stream_name(struct p_emu_stream *stream,char* buffer,int len){
	P_ERROR(DBG_INFO,"");
	return;
}

void import_input_iface(struct p_emu_stream *stream,char* buffer,int len){

	uint8_t i = 0;

	while(len > 0 && buffer[i]!='\n')
	{
		stream->config.rx_iface[i] = buffer[i];
		i++;
		len--;
	}

	stream->config.rx_iface[i+1] = '\0';

	P_ERROR(DBG_INFO,"in_iface: [%s] ",stream->config.rx_iface);
	return;
}

void import_output_iface(struct p_emu_stream *stream,char* buffer,int len){
	uint8_t i = 0;

	while(len > 0 && buffer[i]!='\n')
	{
		stream->config.tx_iface[i] = buffer[i];
		i++;
		len--;
	}

	stream->config.tx_iface[i+1] = '\0';

	P_ERROR(DBG_INFO,"out_iface: [%s] ",stream->config.tx_iface);
	return;
}

void import_src_mac(struct p_emu_stream *stream,char* buffer,int len){

	uint8_t *ptr = &stream->filter.filter_key.payload[P_EMU_ETHER_ADDR_LEN];

	unsigned int p[P_EMU_ETHER_ADDR_LEN];

	/* Get the mac */

	sscanf(buffer,"%2X:%2X:%2X:%2X:%2X:%2X",(unsigned int*)p,
	       (unsigned int*)p+1,
	       (unsigned int*)p+2,
	       (unsigned int*)p+3,
	       (unsigned int*)p+4,
	       (unsigned int*)p+5);

	for (int i = 0; i < 6; i++ ){
		ptr[i] = (uint8_t) p[i];
	}

	P_ERROR(DBG_INFO,"%.2X:%.2X:%.2X:%.2X:%.2X:%.2X",
		(unsigned int)ptr[0],
		(unsigned int)ptr[1],
		(unsigned int)ptr[2],
		(unsigned int)ptr[3],
		(unsigned int)ptr[4],
		(unsigned int)ptr[5]);

	stream->filter.filter_key.length  += P_EMU_ETHER_ADDR_LEN;

	/* Set the mask */

	memset(&stream->filter.filter_mask.payload[P_EMU_ETHER_ADDR_LEN],
	       0xFF,P_EMU_ETHER_ADDR_LEN);

	stream->filter.filter_mask.length += P_EMU_ETHER_ADDR_LEN;

	/* Set the flags */
	stream->filter.flags |= (FILTERING_IS_ENABLED | FILTER_SRC_MAC_ENABLED);

	return;
}

void import_dst_mac(struct p_emu_stream *stream,char* buffer,int len){

	unsigned int p[P_EMU_ETHER_ADDR_LEN];

	/* Get the mac */

	sscanf(buffer,"%2X:%2X:%2X:%2X:%2X:%2X",(unsigned int*)p,
	       (unsigned int*)p+1,
	       (unsigned int*)p+2,
	       (unsigned int*)p+3,
	       (unsigned int*)p+4,
	       (unsigned int*)p+5);

	for (int i = 0; i < 6; i++ ){
		stream->filter.filter_key.payload[i+6] = (uint8_t) p[i];
	}

	P_ERROR(DBG_INFO,"%.2X:%.2X:%.2X:%.2X:%.2X:%.2X",
	   (unsigned int)stream->filter.filter_key.payload[6],
	   (unsigned int)stream->filter.filter_key.payload[7],
	   (unsigned int)stream->filter.filter_key.payload[8],
	   (unsigned int)stream->filter.filter_key.payload[9],
	   (unsigned int)stream->filter.filter_key.payload[10],
	   (unsigned int)stream->filter.filter_key.payload[11]);

	stream->filter.filter_key.length = 2 * P_EMU_ETHER_ADDR_LEN;

	/* Set the mask */

	memset(&stream->filter.filter_mask.payload[P_EMU_ETHER_ADDR_LEN],
	       0xFF,P_EMU_ETHER_ADDR_LEN);

	stream->filter.filter_mask.length = 2 * P_EMU_ETHER_ADDR_LEN;

	/* Set the flags */
	stream->filter.flags |= (FILTERING_IS_ENABLED | FILTER_DST_MAC_ENABLED);

	return;
}

void import_loss(struct p_emu_stream *stream,char* buffer,int len){

	stream->loss.percentage = getFloat(buffer,"",0);

	P_ERROR(DBG_INFO,"Loss percentage %f",stream->loss.percentage);

	stream->loss.flags |= LOSS_IS_ENABLED;
}

void import_static_delay(struct p_emu_stream *stream,char* buffer,int len){

	int sec = getInteger(buffer,"sec=",strlen("sec="));

	int nsec = getInteger(buffer,"nsec=",strlen("nsec="));

	if(sec>0)
		stream->delay.st_delay.d.tv_sec = sec;

	if(nsec>0)
		stream->delay.st_delay.d.tv_nsec = nsec;





	/* Make sure that nsec is less than 10^9 */

	while(stream->delay.st_delay.d.tv_nsec>1000000000){
		stream->delay.st_delay.d.tv_nsec-=1000000000;
		stream->delay.st_delay.d.tv_sec++;
	}

	stream->delay.flags = (DELAY_IS_ENABLED | DELAY_IS_STATIC);

	return;
}


void import_uniform_delay(struct p_emu_stream *stream,char* buffer,int len){

    P_ERROR(DBG_INFO,"___import_uniform_delay___");

	int min = getInteger(buffer,"min=",strlen("min="));

	int max = getInteger(buffer,"max=",strlen("max="));


    if(min>0)
        stream->delay.un_delay.min = min;

	if(max>0)
		stream->delay.un_delay.max = max;

	stream->delay.flags = (DELAY_IS_ENABLED | DELAY_IS_UNIFORM);

	return;
}

void import_normal_delay(struct p_emu_stream *stream,char* buffer,int len){

    int mean = getInteger(buffer,"mean=",strlen("mean="));

	int stddev = getInteger(buffer,"stddev=",strlen("stddev="));

    int shift = getInteger(buffer,"shift=",strlen("shift="));


    if(mean>0)
        stream->delay.ga_delay.mean = mean;

	if(stddev>0)
		stream->delay.ga_delay.stddev = stddev;

    if(shift>0)
		stream->delay.ga_delay.shift = shift;

	stream->delay.flags = (DELAY_IS_ENABLED | DELAY_IS_GAUSSIAN);

	return;
}


int p_emu_CheckStreamName(void* data,  slib_node_t * node)
{
	struct p_emu_stream *Stream = (struct p_emu_stream *)node->data;

	char* pch = (char*) data;

	if(!memcmp(pch,Stream->stream_name,Stream->stream_name_len))
		return 1;

	return 0;
}

void p_emu_StreamDestroy (struct p_emu_stream *stream)
{
	if(!stream)
		return;

	/* Close sockets */
	if(stream->config.rx_iface_fd!=P_EMU_UNINITIALIZED){
		shutdown(stream->config.rx_iface_fd,SHUT_RDWR);
		close(stream->config.rx_iface_fd);
	}

	if(stream->config.tx_iface_fd!=P_EMU_UNINITIALIZED){
		shutdown(stream->config.tx_iface_fd,SHUT_RDWR);
		close(stream->config.tx_iface_fd);
	}

	/* TODO: Destroy Rx-Tx lists */


	/* Free stream */
	free(stream);
}

struct p_emu_stream * p_emu_StreamInit (char* stream_name)
{
	struct p_emu_stream *Stream = NULL;

	if((!(Stream = malloc(sizeof(struct p_emu_stream)))) || !stream_name){
		return NULL;
	}

	memset(Stream,0,sizeof(struct p_emu_stream));

	Stream->stream_name_len = strlen(stream_name);

	memcpy(Stream->stream_name,stream_name,Stream->stream_name_len);

	/* Remove non-printable characters */

	if(Stream->stream_name[Stream->stream_name_len-1] == '\n'
			|| Stream->stream_name[Stream->stream_name_len-1] == '\r'){

		Stream->stream_name[Stream->stream_name_len-1] = '\0';
		Stream->stream_name_len--;
	}

	P_ERROR(DBG_INFO,"New Stream: [%s] Len[%d]",
		Stream->stream_name, Stream->stream_name_len);

	Stream->config.rx_iface_fd = P_EMU_UNINITIALIZED;
	Stream->config.tx_iface_fd = P_EMU_UNINITIALIZED;

	Stream->timers.tx_timer = timerfd_create(CLOCK_REALTIME,TFD_NONBLOCK);
	if(Stream->timers.tx_timer < 0){
		P_ERROR(DBG_ERROR,"Timer failed [%d]",Stream->timers.tx_timer);
		return NULL;
	}

	/* Initialize stream's rx_list and tx_list */

	Stream->rx_list = slib_list_init();
	Stream->rx_list_max_size = P_EMU_MAX_LIST_SIZE;

	Stream->tx_list = slib_list_init();
	Stream->tx_list_max_size = P_EMU_MAX_LIST_SIZE;

	if(!Stream->rx_list || !Stream->tx_list){
		P_ERROR(DBG_ERROR,"List init failed");
		memset(Stream,0,sizeof(struct p_emu_stream));
		free(Stream);
	}

	return Stream;
}

int p_emu_import_settings(const char *filename, slib_root_t *streams)
{
	FILE *fp = NULL;
	char *line = NULL;
	int read = 0;
	size_t len = 0;
	uint32_t i = 0, cmd_len = 0;

	slib_node_t *StreamNode = NULL;
	struct p_emu_stream *Stream = NULL;


	if(!filename || !streams) {
		P_ERROR(DBG_ERROR,"Wrong params");
		return -1;
	}

	P_ERROR(DBG_INFO,"Openning file __%s__",filename);

	fp = fopen(filename,"r");
	if(!fp) {
		P_ERROR(DBG_ERROR,"File open Failed!");
		goto termination;
	}

	while ((read = getline(&line, &len, fp)) != -1)
	{
		if(memcmp(line,STREAM_NAME,strlen(STREAM_NAME))==0)
		{
			P_ERROR(DBG_INFO,"New Stream!");

			StreamNode = slib_find_in_list(streams,
						   &line[strlen(STREAM_NAME)],
						   p_emu_CheckStreamName);

			if(StreamNode)
			{
				/* Stream exists, just update it */
				Stream = (struct p_emu_stream *)StreamNode->data;
			}else{
				/* Create a new stream */
				P_ERROR(DBG_INFO,"Create Stream!");

				Stream = p_emu_StreamInit(&line[strlen(STREAM_NAME)]);

				/* Add stream in the list. */
				if(slib_list_add_last(streams,Stream)
						!= LIST_OP_SUCCESS)
				{
					P_ERROR(DBG_ERROR,
						"Adding [%p] to list failed!",
						Stream);

					goto termination;
				}
			}

			continue;
		}

		if(!Stream)
		{
			P_ERROR(DBG_ERROR,"Stream ptr NULL, terminating!!!");
			goto termination;
		}

		for(i=0; i < NUM_OF_IMPORT_PARAMS; i++)
		{
			cmd_len = strlen(p_emu_import_table[i].command);

			if(memcmp(line,p_emu_import_table[i].command,
					cmd_len) ==0 && (len > cmd_len))
			{
				p_emu_import_table[i].func(Stream,
							   &line[cmd_len],len);
			}
		}

	}

	if (line) {
		free(line);
	}

	fclose(fp);

	return 0;

termination:
	if (line) {
		free(line);
	}

	if (fp) {
		fclose(fp);
	}

	return -1;
}

/* End of p_emu_help.c ------------------------------------------------- */



/* Test func ------------------ */

#if 0
void string2mac(char* buffer, uint8_t* mac){
	char dictionary [] = "0123456789abcdef:";
	int dicSize = strlen(dictionary);
	int j = 0 ;
	while(*buffer){
		for(int i = 0; i < (dicSize-2);i++){
			if(dictionary[i] == *buffer){
				if(j%2)
				{
					*mac= *mac | (uint8_t)i;
					mac++;
				}else{
					*mac = (i<<4);
				}
				j++;
			}else if (*buffer==dictionary[dicSize-1]){
				mac++;
				break;
			}
		}
		buffer++;
	}
}
#endif
