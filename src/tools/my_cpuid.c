#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "my_cpuid.h"
#include "../emu/x86emu_private.h"
#include "debug.h"

int get_cpuMhz()
{
	int MHz = 0;
#ifdef __arm__
	FILE *f = fopen("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq", "r");
	if(f) {
		int r;
		if(1==fscanf(f, "%d", &r))
			MHz = r/1000;
		fclose(f);
	}
#endif
	if(!MHz)
		MHz = 1000; // default to 1Ghz...
	return MHz;
}

void my_cpuid(x86emu_t* emu, uint32_t tmp32u)
{
    switch(tmp32u) {
        case 0x0:
            // emulate a P4
            R_EAX = 0x80000004;
            // return GenuineIntel
            R_EBX = 0x756E6547;
            R_EDX = 0x49656E69;
            R_ECX = 0x6C65746E;
            break;
        case 0x1:
            R_EAX = 0x00000601; // familly and all
            R_EBX = 0;          // Brand indexe, CLFlush, Max APIC ID, Local APIC ID
            R_EDX =   1         // fpu 
                    | 1<<8      // cmpxchg8
                    | 1<<11     // sep (sysenter & sysexit)
                    | 1<<15     // cmov
                    | 1<<19     // clflush (seems to be with SSE2)
                    | 1<<23     // mmx
                    //| 1<<24     // fxsr (fxsave, fxrestore)
                    | 1<<25     // SSE
                    | 1<<26     // SSE2
                    ;
            R_ECX =   1<<0      // SSE3
                    //| 1<<9      // SSSE3
                    | 1<<12     // fma
                    | 1<<13     // cx16 (cmpxchg16)
                    ; 
            if(!box86_steam)
                R_ECX |= (1<<9);    // Disabling SSSE3 for steam for now
            break;
        case 0x2:   // TLB and Cache info. Sending 1st gen P4 info...
            R_EAX = 0x665B5001;
            R_EBX = 0x00000000;
            R_ECX = 0x00000000;
            R_EDX = 0x007A7000;
            break;
        
        case 0x4:   // Cache info
            switch (R_ECX) {
                case 0: // L1 data cache
                    R_EAX = (1 | (1<<5) | (1<<8));   //type
                    R_EBX = (63 | (7<<22)); // size
                    R_ECX = 63;
                    R_EDX = 1;
                    break;
                case 1: // L1 inst cache
                    R_EAX = (2 | (1<<5) | (1<<8)); //type
                    R_EBX = (63 | (7<<22)); // size
                    R_ECX = 63;
                    R_EDX = 1;
                    break;
                case 2: // L2 cache
                    R_EAX = (3 | (2<<5) | (1<<8)); //type
                    R_EBX = (63 | (15<<22));    // size
                    R_ECX = 4095;
                    R_EDX = 1;
                    break;

                default:
                    R_EAX = 0x00000000;
                    R_EBX = 0x00000000;
                    R_ECX = 0x00000000;
                    R_EDX = 0x00000000;
                    break;
            }
            break;
        case 0x5:   //mwait info
            R_EAX = 0;
            R_EBX = 0;
            R_ECX = 1 | 2;
            R_EDX = 0;
            break;
        case 0x6:   // thermal
        case 0x9:   // direct cache access
        case 0xA:   // Architecture performance monitor
            R_EAX = 0;
            R_EBX = 0;
            R_ECX = 0;
            R_EDX = 0;
            break;
        case 0x7:   // extended bits...
            /*if(R_ECX==0)    R_EAX = 0;
            else*/ R_EAX = R_ECX = R_EBX = R_EDX = 0;
            break;

        case 0x80000000:        // max extended
            R_EAX = 0x80000005;
            break;
        case 0x80000001:        //Extended Processor Signature and Feature Bits
            R_EAX = 0;  // reserved
            R_EBX = 0;  // reserved
            R_ECX = (1<<5) | (1<<8); // LZCNT | PREFETCHW
            R_EDX = 0;
            break;
        case 0x80000002:    // Brand part 1 (P4 signature)
            R_EAX = 0x20202020;
            R_EBX = 0x20202020;
            R_ECX = 0x20202020;
            R_EDX = 0x6E492020;
            break;
        case 0x80000003:    // Brand part 2
            R_EAX = 0x286C6574;
            R_EBX = 0x50202952;
            R_ECX = 0x69746E65;
            R_EDX = 0x52286D75;
            break;
        case 0x80000004:    // Brand part 3, with frequency
            R_EAX = 0x20342029;
            R_EBX = 0x20555043;
            {
                static int MHz = 0;
                if(!MHz)
					MHz = get_cpuMhz();
                if(MHz>15000) { // swiches to GHz display...
                    char tmp[5];
                    sprintf(tmp, "%1.2f", MHz/1000.);
                    R_ECX = *(uint32_t*)tmp;
                    R_EDX = 0x007A4847; // GHz
                } else {
                    char tmp[5];
                    sprintf(tmp, "%04d", MHz);
                    R_ECX = *(uint32_t*)tmp;
                    R_EDX = 0x007A484D; // MHz
                }
            }
            break;  
        case 0x80000005:
            R_EAX = 0;
            R_EBX = 0;
            R_ECX = 0;
            R_EDX = 0;
            break;
        default:
            printf_log(LOG_INFO, "Warning, CPUID command %X unsupported (ECX=%08x)\n", tmp32u, R_ECX);
            R_EAX = 0;
    }   
}
