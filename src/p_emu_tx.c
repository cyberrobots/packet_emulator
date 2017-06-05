#include "p_emu_libs.h"
#include "p_emu_dbg.h"
#include "p_emu_core.h"





void* p_emu_TxThread(void* params)
{

	struct p_emu_tx_config* cfg = (struct p_emu_tx_config*)params;
	slib_root_t *streams = cfg->streams;

	streams = streams;

	while(1)
	{
		P_ERROR(DBG_INFO,"TxThread");

		sleep(12);
	}


	return NULL;
}
