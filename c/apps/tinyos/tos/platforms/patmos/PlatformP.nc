#include "hardware.h"

module PlatformP @safe() 
{
	provides interface Init;
    uses interface Init as LedsInit;
    uses interface Init as MMUInit;
    uses interface MemoryManagementUnit as MMU;

    uses interface GeneralIO as Led0;
	uses interface GeneralIO as Led1;
	uses interface GeneralIO as Led2;
	uses interface GeneralIO as Led3;
	uses interface GeneralIO as Led4;
	uses interface GeneralIO as Led5;
	uses interface GeneralIO as Led6;
	uses interface GeneralIO as Led7;
	uses interface GeneralIO as Led8;
}

implementation 
{
	command error_t Init.init() 
	{
		//reset all the
		EXCEPTION_PEND = 0;

        call LedsInit.init();
        //call MMUInit.init();
        return SUCCESS;
	}

	default command error_t LedsInit.init() 
	{
		call Led0.clr();
		call Led1.clr();
		call Led2.clr();
		call Led3.clr();
		call Led4.clr();
		call Led5.clr();
		call Led6.clr();
		call Led7.clr();
		call Led8.clr();

		return SUCCESS;
	}

	default command error_t MMUInit.init() 
	{
		uint8_t i;
		mmuConf_t config;
		memset(&config, 0, sizeof(mmuConf_t));

		for(i = 0; i < MMU_NUM_SEGMENTS; i++)
			call MMU.setConfiguration(i, config);

		return SUCCESS;
	}
}