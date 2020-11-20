#include "Timer.h"

module PatmosAppP
{
	uses interface Boot;

	uses interface GeneralIO as Led0;
	uses interface GeneralIO as Led1;
	uses interface GeneralIO as Led2;
	uses interface GeneralIO as Led3;
	uses interface GeneralIO as Led4;
	uses interface GeneralIO as Led5;
	uses interface GeneralIO as Led6;
	uses interface GeneralIO as Led7;
	uses interface GeneralIO as Led8;

	uses interface GeneralIO as Key0;
	uses interface GeneralIO as Key1;
	uses interface GeneralIO as Key2;
	uses interface GeneralIO as Key3;

	uses interface UartByte;
	uses interface UartStream;
	uses interface Deadline;
	uses interface CpuInfo;
	uses interface MemoryManagementUnit;

	uses interface Exception as TrapOP;
	uses interface Exception as TrapTest;

	uses interface Timer<TMicro> as Timer0;
	uses interface Timer<TMicro> as Timer1;
	uses interface McuSleep;
}
implementation
{
	task void sendCPUInfo();
	task void printIllOP();
	task void printTrap4();

	uint8_t* itoa(int n, char* s)
	{
		int i, j;
		char c;

		i = 0;
		do 
		{       /* generate digits in reverse order */
			s[i++] = n % 10 + '0';   /* get next digit */
		} while ((n /= 10) > 0);     /* delete it */
		s[i] = '\0';

		for (i = 0, j = strlen(s)-1; i<j; i++, j--) 
		{
			c = s[i];
			s[i] = s[j];
			s[j] = c;
		}
		return (uint8_t*)s;
	} 

	void gotoSleep()
	{
		call McuSleep.sleep();
	}

	event void Boot.booted()
	{
		call Timer0.startPeriodic( 100000 );
		call Timer1.startPeriodic( 500000 );  
	}

	async event void UartStream.sendDone( uint8_t* buf, uint16_t len, error_t error ){}

	async event void UartStream.receiveDone( uint8_t* buf, uint16_t len, error_t error ){}

	async event void UartStream.receivedByte( uint8_t byte ){}

	async event void Deadline.done(uint32_t count)
	{
		call Led8.toggle();
	}

	async event void TrapOP.fired()
	{
		post printIllOP();  
	}

	async event void TrapTest.fired()
	{
		post printTrap4();   
	}

	event void Timer1.fired()
	{
		call Led4.toggle();
		call Led5.toggle();
		call Led6.toggle();
		call Led7.toggle(); 
	}

	event void Timer0.fired()
	{
		static bool led0 = FALSE, led1 = FALSE, led2 = FALSE, led3 = FALSE;
		static bool oldKey0 = FALSE, oldKey1 = FALSE, oldKey2 = FALSE, oldKey3 = FALSE;
		bool key0Val, key1Val, key2Val, key3Val;

		key0Val = call Key0.get();
		key1Val = call Key1.get();
		key2Val = call Key2.get();
		key3Val = call Key3.get();

		if(!key0Val && oldKey0)
		{
			if(led0)
			{
				call Led0.clr();
				led0 = FALSE; 
			}
			else
			{
				call Led0.set();
				led0 = TRUE;
			}

			__nesc_enable_interrupt();
			call TrapTest.enable();

			trap(4);
			//asm volatile(".word 0xffffffff");
			//(*((volatile _IODEV unsigned *)0xffffffff)) = 0;
		}

		if(!key1Val && oldKey1)
		{
			char buffer[30+1];
			call UartStream.send((uint8_t*)"Enter some text (max 30 chars+\\0): ", strlen("Enter some text (max 30 chars+\\0): "));

			call UartStream.receive((uint8_t*)buffer, strlen(buffer));
			call UartStream.send((uint8_t*)buffer, strlen(buffer));


			if(led1)
			{
				call Led1.clr();
				led1 = FALSE; 
			}
			else
			{
				call Led1.set();
				led1 = TRUE;
			}
		}

		if(!key2Val && oldKey2)
		{
			call Deadline.start(call CpuInfo.getClkFreq()*5);

			if(led2)
			{
				call Led2.clr();
				led2 = FALSE; 
			}
			else
			{
				call Led2.set();
				led2 = TRUE;
			}
		}

		if(!key3Val && oldKey3)
		{
			post sendCPUInfo();

			if(led3)
			{
				call Led3.clr();
				led3 = FALSE; 
			}
			else
			{
				call Led3.set();
				led3 = TRUE;
			}
		}

		oldKey0 = key0Val;
		oldKey1 = key1Val;
		oldKey2 = key2Val;
		oldKey3 = key3Val;  
	}

	task void printIllOP()
	{
		call UartStream.send((uint8_t*)"\nIllegal-Operation-ISR!\n", (uint16_t)strlen("\nIllegal-Operation-ISR!\n"));
	}

	task void printTrap4()
	{
		call UartStream.send((uint8_t*)"\nTrap4-Test-ISR!\n", (uint16_t)strlen("\nTrap4-Test-ISR!\n"));
	}

	task void sendCPUInfo()
	{
		extMemConf_t extMemConf;
		chacheConf_t cacheConf;
		char buffer[30+1];

		call UartStream.send((uint8_t*)"\nCore ID: ", (uint16_t)strlen("\nCore ID: "));
		call UartStream.send(itoa(call CpuInfo.getCoreID(), buffer), strlen(buffer));
		call UartStream.send((uint8_t*)"\nClk-Freq: ", (uint16_t)strlen("\nClk-Freq: "));
		call UartStream.send(itoa(call CpuInfo.getClkFreq(), buffer), strlen(buffer));
		call UartStream.send((uint8_t*)"\nCore-Cnt: ", (uint16_t)strlen("\nCore-Cnt: "));
		call UartStream.send(itoa(call CpuInfo.getCoreCount(), buffer), strlen(buffer));
		call UartStream.send((uint8_t*)"\nPipelines: ", (uint16_t)strlen("\nPipelines: "));
		call UartStream.send(itoa(call CpuInfo.getFeatures(), buffer), strlen(buffer));
		call UartStream.send((uint8_t*)"\nExtMemSize: ", (uint16_t)strlen("\nExtMemSize: "));
		call UartStream.send(itoa(call CpuInfo.getExtMemSize(), buffer), strlen(buffer));

		call UartStream.send((uint8_t*)"\nExtMemConf: ", (uint16_t)strlen("\nExtMemConf: "));
		call UartStream.send(itoa(call CpuInfo.getExtMemConfVal(), buffer), strlen(buffer));
		extMemConf = call CpuInfo.getExtMemConf();
		call UartStream.send((uint8_t*)"\n\tExtMem Burst Length: ", (uint16_t)strlen("\n\tExtMem Burst Length: "));
		call UartStream.send(itoa(extMemConf.burstLength, buffer), strlen(buffer));
		call UartStream.send((uint8_t*)"\n\tExtMem CombinedWritesToExtMem: ", (uint16_t)strlen("\n\tExtMem CombinedWritesToExtMem: "));
		call UartStream.send(itoa(extMemConf.combinedWritesToExtMem, buffer), strlen(buffer));

		call UartStream.send((uint8_t*)"\nICacheSize: ", (uint16_t)strlen("\nICacheSize: "));
		call UartStream.send(itoa(call CpuInfo.getICacheSize(), buffer), strlen(buffer));
		call UartStream.send((uint8_t*)"\nICacheConf: ", (uint16_t)strlen("\nICacheConf: "));
		call UartStream.send(itoa(call CpuInfo.getICacheConfVal(), buffer), strlen(buffer));
		cacheConf = call CpuInfo.getICacheConf();
		call UartStream.send((uint8_t*)"\n\tICache Type: ", (uint16_t)strlen("\n\tICache Type: "));
		call UartStream.send(itoa(cacheConf.cacheType, buffer), strlen(buffer));
		call UartStream.send((uint8_t*)"\n\tICache Policy: ", (uint16_t)strlen("\n\tICache Policy: "));
		call UartStream.send(itoa(cacheConf.replacementPolicy, buffer), strlen(buffer));
		call UartStream.send((uint8_t*)"\n\tICache Associativity: ", (uint16_t)strlen("\n\tICache Associativity: "));
		call UartStream.send(itoa(cacheConf.associativity, buffer), strlen(buffer));

		call UartStream.send((uint8_t*)"\nDCacheSize: ", (uint16_t)strlen("\nDCacheSize: "));
		call UartStream.send(itoa(call CpuInfo.getDCacheSize(), buffer), strlen(buffer));
		call UartStream.send((uint8_t*)"\nDCacheConf: ", (uint16_t)strlen("\nDCacheConf: "));
		call UartStream.send(itoa(call CpuInfo.getDCacheConfVal(), buffer), strlen(buffer));
		cacheConf = call CpuInfo.getDCacheConf();
		call UartStream.send((uint8_t*)"\n\tDCache Type: ", (uint16_t)strlen("\n\tDCache Type: "));
		call UartStream.send(itoa(cacheConf.cacheType, buffer), strlen(buffer));
		call UartStream.send((uint8_t*)"\n\tDCache Policy: ", (uint16_t)strlen("\n\tDCache Policy: "));
		call UartStream.send(itoa(cacheConf.replacementPolicy, buffer), strlen(buffer));
		call UartStream.send((uint8_t*)"\n\tDCache Associativity: ", (uint16_t)strlen("\n\tDCache Associativity: "));
		call UartStream.send(itoa(cacheConf.associativity, buffer), strlen(buffer));

		call UartStream.send((uint8_t*)"\nSCacheSize: ", (uint16_t)strlen("\nSCacheSize: "));
		call UartStream.send(itoa(call CpuInfo.getSCacheSize(), buffer), strlen(buffer));
		call UartStream.send((uint8_t*)"\nSCacheConf: ", (uint16_t)strlen("\nSCacheConf: "));
		call UartStream.send(itoa(call CpuInfo.getSCacheConf(), buffer), strlen(buffer));
		call UartStream.send((uint8_t*)"\nI-SPM-Size: ", (uint16_t)strlen("\nI-SPM-Size: "));
		call UartStream.send(itoa(call CpuInfo.getISPMSize(), buffer), strlen(buffer));
		call UartStream.send((uint8_t*)"\nD-SPM-Size: ", (uint16_t)strlen("\nD-SPM-Size: "));
		call UartStream.send(itoa(call CpuInfo.getDSPMSize(), buffer), strlen(buffer));

		call UartStream.send((uint8_t*)"\nBootSrcStart: ", (uint16_t)strlen("\nBootSrcStart: "));
		call UartStream.send(itoa(call CpuInfo.getBootSrcStart(), buffer), strlen(buffer));
		call UartStream.send((uint8_t*)"\nBootSrcSize: ", (uint16_t)strlen("\nBootSrcSize: "));
		call UartStream.send(itoa(call CpuInfo.getBootSrcSize(), buffer), strlen(buffer));
		call UartStream.send((uint8_t*)"\nBootDstStart: ", (uint16_t)strlen("\nBootDstStart: "));
		call UartStream.send(itoa(call CpuInfo.getBootDstStart(), buffer), strlen(buffer));
		call UartStream.send((uint8_t*)"\nBootDstSize: ", (uint16_t)strlen("\nBootDstSize: "));
		call UartStream.send(itoa(call CpuInfo.getBootDstSize(), buffer), strlen(buffer));
		call UartByte.send('\n');
	}
}