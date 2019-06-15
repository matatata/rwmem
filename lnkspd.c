/** \file
 * Adapted rdpci.c to read and try to set a PCIe device's link speed.
 * My NVMe PCIe Carrier card does not negotiate 5 GT/s but stays at 2.5 GT/s in my MacPro3,1. With this program I can set link speed to 2 and I have my 5 GT/s
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include "DirectHW.h"

int
main(
	int argc,
	char ** argv
)
{
	if (argc != 4 && argc != 5)
	{
		fprintf(stderr, "Usage: %s bus slot func [link_speed 1,2 or 3]\n", argv[0]);
		return EXIT_FAILURE;
	}
    
    unsigned target_speed = 0x0;
    if(argc == 5){
        target_speed = strtoul(argv[4], NULL, 0);
        switch (target_speed) {
            case 1: // PCI 1.0 2.5 GT/s
            case 2: // PCI 2.0 5 GT/s
            case 3: // PCI 3.0 ?
                break;
                
            default:
                printf("illegal target link speed %d\n",target_speed);
                return EXIT_FAILURE;
        }
    }

	const uint32_t bus = strtoul(argv[1], NULL, 0);
	const uint32_t slot = strtoul(argv[2], NULL, 0);
	const uint32_t func = strtoul(argv[3], NULL, 0);
	const uint32_t reg = 0;

	const uint32_t addr = 0xe0000000
		| ((bus  & 0xFF) << 20) // 8 bits
		| ((slot & 0x1F) << 15) // 5 bits
		| ((func & 0x07) << 12) // 3 bits
		| ((reg  & 0xFFC) << 0) // 12 bits, minus bottom 2
		;

	if (iopl(0) < 0)
	{
		perror("iopl");
		return EXIT_FAILURE;
	}

	const uintptr_t page_mask = 0xFFF;
	const uintptr_t page_offset = addr & page_mask;
	const uintptr_t map_addr = addr & ~page_mask;
	const size_t map_len = (page_offset + 256 + page_mask) & ~page_mask;

	const uint8_t * const pcibuf = map_physical(map_addr, map_len);
	if (pcibuf == NULL)
	{
		perror("map");
		return EXIT_FAILURE;
	}

    
    
    
#define REG(i) *(uint32_t*)(pcibuf + page_offset + (i))

    
    unsigned vendor_device = REG(0x0);
    unsigned VENDOR = vendor_device & 0xFFFF;
    unsigned DEVICE = (vendor_device >> 16) & 0xFFFF;
    
    if(VENDOR == 0xFFFF || DEVICE==0xFFFF){
        printf("bad device %08x\n",vendor_device);
        return EXIT_FAILURE;
    }
    
    printf("Vendor %04x Device %04x ",VENDOR,DEVICE);
    
    unsigned CAP_PTR = REG(0x34) & 0xFF;
    while(CAP_PTR) {
        unsigned CAP_X = REG(CAP_PTR); // Cap list register
        unsigned CAP_ID = CAP_X & 0xFF;
        
        if(CAP_ID & 0x10) {
            unsigned lnk_stat = REG(CAP_PTR+0x10) >> 16; // offset 0x12
            unsigned cur_speed = lnk_stat & 0xF;
            unsigned neg_width = lnk_stat >> 4 & 0x3F;
            unsigned lnk_capa = REG(CAP_PTR+0x0C);
            unsigned max_speed = lnk_capa & 0xF;
            unsigned max_width = lnk_capa >> 4 & 0x3F;
            
            printf("link speed %d (max %d) x%d (max x%d)\n",cur_speed,max_speed,neg_width,max_width);
            
            if(target_speed !=0){
                if(target_speed != cur_speed) {
                    printf("trying to set target link speed to %d and set retrain link bit, will then sleep 1 sec and check link speed again\n",target_speed);
                    // set target speed to max value
                    unsigned link_control_reg2 = REG(CAP_PTR+0x30);
                    
                    link_control_reg2 &= ~0xf; // Clear lower 4 bits.
                    link_control_reg2 |= target_speed & 0xf; // set target speed
                    REG(CAP_PTR+0x30)=link_control_reg2;
                    
                    // Set Retrain Link bit 5 of offset 10
                    unsigned link_ctrl_reg = REG(CAP_PTR+0x10);
                    link_ctrl_reg |= 0x20;
                    REG(CAP_PTR+0x10)=link_ctrl_reg;
                    
                    sleep(1);
                    
                    // check new link speed
                    lnk_stat = REG(CAP_PTR+0x10) >> 16; // offset 0x12
                    unsigned new_cur_speed = lnk_stat & 0xF;
                    
                    
                    printf("link speed now %d\n",new_cur_speed);
                }
                else {
                    printf("nothing to do\n");
                }
            }
            
            return EXIT_SUCCESS;
        }
        else {
            // must look at next capability record
            CAP_PTR = (CAP_X & 0xFF00) >> 8;
            
        }
    }
    printf("Could not find PCI Express Capability Structure\n");
	return EXIT_FAILURE;
}





