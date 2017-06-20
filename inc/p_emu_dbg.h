#ifndef PACKET_EMU_DBG_H_
#define PACKET_EMU_DBG_H_

/* Display Information to user, standar output */
#define P_EMU_INFO(m,...)	fprintf(stdout,m"\r\n",##__VA_ARGS__);


/* Display debug informantion to user, regarding the debug level */

typedef enum{
#define DBG_ERROR 	(0)
#define DBG_WARN 	(1)
#define DBG_INFO 	(2)
    p_emu_dbg_0 = 0,
    p_emu_dbg_1,
    p_emu_dbg_2,
    p_emu_dbg_num,
}p_emu_dbg_t;

/* Return Debug Level */

char* p_emu_dbg_str(p_emu_dbg_t dbg);

#ifndef NDEBUG
#ifdef TIME_LOG
#define P_ERROR(l,m,...) \
	extern struct timeval p_emu_time_start; \
	extern struct timeval p_emu_time_current; \
	extern p_emu_dbg_t __debug_level; \
	do{	\
        if(p_emu_time_start.tv_sec==0 && p_emu_time_start.tv_usec==0) \
        {gettimeofday(&p_emu_time_start,NULL); } \
        gettimeofday(&p_emu_time_current,NULL); \
	if ( l <= __debug_level )	\
	{	\
	fprintf(stdout,"[%.0lld][%s][%d][%s]"m"\n", \
	((p_emu_time_current.tv_sec-p_emu_time_start.tv_sec)*1000000LL+ \
		p_emu_time_current.tv_usec-p_emu_time_start.tv_usec), \
	p_emu_dbg_str(__debug_level), \
	__LINE__, \
	__func__, \
	##__VA_ARGS__); \
	}	\
}while(0);

#else
#define P_ERROR(l,m,...) \
    extern p_emu_dbg_t __debug_level; \
    do{	\
	if ( l <= __debug_level )	\
	{	\
		fprintf(stdout,"[%s][%d][%s]"m"\n", \
		p_emu_dbg_str(__debug_level), \
		__LINE__, \
		__func__, \
		##__VA_ARGS__); \
	}	\
    }while(0);
#endif
#else
#define P_ERROR(l,m,...)  ;
#endif

#endif /* PACKET_EMU_DBG_H_ */

