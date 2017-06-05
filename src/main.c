#include "p_emu.h"

int main (int argc, char *argv[])
{
	extern char *optarg;
	int c = 0;

	if(argc <= 1){
		p_emu_usage();
		return 0;
	}


	slib_root_t *StreamsList = slib_list_init();

	while((c = getopt(argc,argv,"hvlf:"))!=-1)
	{
		switch(c)
		{
			case 'h':
				p_emu_usage();
				break;
			case 'v':
				p_emu_version();
				break;
			case 'l':
				p_emu_interfaces();
				break;
			case 'f':
				if(!p_emu_import_settings(optarg,StreamsList))
					goto RunDeamon;
				else
					p_emu_usage();
				break;
			default:
				p_emu_usage();
				break;
		}
	}

	return 0;

RunDeamon:

	if(geteuid() != 0){
		P_EMU_INFO("root access required!");
		/* TODO: Clean Stream List required */
		return 0;
	}

	if(p_emu_create_interfaces(StreamsList) < 0){
		P_ERROR(DBG_INFO,"Error During Creating interfaces");
		goto cleanup;
	}

	/* All streams have their settings and pakcet emulator is ready to start */

	if(p_emu_start(StreamsList)){
		P_ERROR(DBG_ERROR,"Starting Core failed!");
		goto cleanup;
	}



	while(1)
	{
		P_ERROR(DBG_INFO,"Deamon");
		sleep(100000000);
	}

	return 0;

cleanup:
	return 0;
}

/* END OF ScrambleNet's main ------------------------------------------------ */




