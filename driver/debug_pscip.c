/** \file debug_pscip.c
	Includes functions used while debugging. 
	This functions interprete IP and PCI Carrier registers - make life much easier for the developer 
*/
//interpreting  FPGA status register 
void IP_DEBUG_read_FPGA_stat(volatile struct FPGAstatus_reg *pReg)
{
	printk(KERN_ALERT "\t\t DEBUG: IP_DEBUG_read_FPGA_stat() - beginning\n");	
	printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: Reading and interpreting FPGA status register \n");
	printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: high word(BE-returned): == 0x%x%x  binary  %d%d%d%d %d%d%d%d  %d%d%d%d %d%d%d%d\n",pReg->errorCounter, pReg->highWord,0x01&(pReg->errorCounter >>7),0x01&(pReg->errorCounter >>6),0x01&(pReg->errorCounter >>5),0x01&(pReg->errorCounter >>4),0x01&(pReg->errorCounter >>3),0x01&(pReg->errorCounter >>2),0x01&(pReg->errorCounter>>1),0x01&(pReg->errorCounter >>0),0x01&(pReg->highWord >>7),0x01&(pReg->highWord >>6),0x01&(pReg->highWord >>5),0x01&(pReg->highWord >>4),0x01&(pReg->highWord >>3),0x01&(pReg->highWord >>2),0x01&(pReg->highWord >>1),0x01&pReg->highWord);
	printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: mid  word(BE-returned): == 0x%x  binary  %d%d%d%d %d%d%d%d  %d%d%d%d %d%d%d%d\n",pReg->midWord,0x01&(pReg->midWord >>15),0x01&(pReg->midWord >>14),0x01&(pReg->midWord >>13),0x01&(pReg->midWord >>12),0x01&(pReg->midWord >>11),0x01&(pReg->midWord >>10),0x01&(pReg->midWord>>9),0x01&(pReg->midWord >>8),0x01&(pReg->midWord >>7),0x01&(pReg->midWord >>6),0x01&(pReg->midWord >>5),0x01&(pReg->midWord >>4),0x01&(pReg->midWord >>3),0x01&(pReg->midWord >>2),0x01&(pReg->midWord >>1),0x01&pReg->midWord);
	printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: low  word(BE-returned): == 0x%x  binary  %d%d%d%d %d%d%d%d  %d%d%d%d %d%d%d%d\n",pReg->lowWord,0x01&(pReg->lowWord >>15),0x01&(pReg->lowWord >>14),0x01&(pReg->lowWord >>13),0x01&(pReg->lowWord>>12),0x01&(pReg->lowWord >>11),0x01&(pReg->lowWord >>10),0x01&(pReg->lowWord>>9),0x01&(pReg->lowWord >>8),0x01&(pReg->lowWord >>7),0x01&(pReg->lowWord >>6),0x01&(pReg->lowWord >>5),0x01&(pReg->lowWord >>4),0x01&(pReg->lowWord >>3),0x01&(pReg->lowWord >>2),0x01&(pReg->lowWord >>1),0x01&pReg->lowWord);

//high word
	printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: Erroc counter: %d, highWord: 0x%x\n", pReg->errorCounter,pReg->highWord);
	
	if(pReg->highWord & PSCIP_LINKDOWN_MSK)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) Linkdown - error \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) Linkdown - ok \n");
	if(pReg->highWord & PSCIP_TXERR_MSK)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) Transmition - error \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) Transmition - ok \n");
	if(pReg->highWord & PSCIP_LOOPBACK_MSK)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) loopback mode - on \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) loopback mode - off \n");
	
//mid word
	//reg 0
	printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: -------REGISTER 0 ERRORS----------\n");
	if(pReg->midWord & PSCIP_PAR_ERR_0_MSK)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) Priority erro or high priority write - error \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) Priority erro or high priority write  - ok \n");
	if(pReg->midWord & PSCIP_BREAK_ERR_0_MSK)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) break error - error \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) break error- ok \n");
	if(pReg->midWord & PSCIP_FRAME_ERR_0_MSK)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) framing error- error \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) framing  - ok \n");	
	if(pReg->midWord & PSCIP_COMP_ERR_0_MSK)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) compare - error \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) compare error  - ok \n");
	if(pReg->midWord & PSCIP_TIMEOUT_ERR_0_MSK)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) timeout error - error \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) timeout error - ok \n");	
	//reg 1
	printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: -------REGISTER 1 ERRORS----------\n");
	if(pReg->midWord & PSCIP_PAR_ERR_1_MSK)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) Priority erro or high priority write  - error \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) Priority erro or high priority write  - ok \n");
	if(pReg->midWord & PSCIP_BREAK_ERR_1_MSK)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) break error- error \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) break error- ok \n");
	if(pReg->midWord & PSCIP_FRAME_ERR_1_MSK)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) framing error- error \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) framing error - ok \n");	
	if(pReg->midWord & PSCIP_COMP_ERR_1_MSK)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) compare error - error \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) compare error - ok \n");
	if(pReg->midWord & PSCIP_TIMEOUT_ERR_1_MSK)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) timeout error- error \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) timeout error - ok \n");	
	//reg 2
	printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: ------REGISTER 2 ERRORS----------\n");
	if(pReg->midWord & PSCIP_PAR_ERR_2_MSK)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) Priority erro or high priority write  - error \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) Priority erro or high priority write  - ok \n");
	if(pReg->midWord & PSCIP_BREAK_ERR_2_MSK)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) break error- error \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) break error- ok \n");
	if(pReg->midWord & PSCIP_FRAME_ERR_2_MSK)
		printk(KERN_ALERT "(value==1) framing error- error \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) framing error - ok \n");	
	if(pReg->midWord & PSCIP_COMP_ERR_2_MSK)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) ompare error - error \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) compare error - ok \n");
	if(pReg->midWord & PSCIP_TIMEOUT_ERR_2_MSK)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) timeout error- error \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) timeout error - ok \n");	
//low word
	//reg 3
	printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: -------REGISTER 3 ERRORS----------\n");
	if(pReg->lowWord & PSCIP_PAR_ERR_3_MSK)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) Priority erro or high priority write  - error \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) Priority erro or high priority write  - ok \n");
	if(pReg->lowWord & PSCIP_BREAK_ERR_3_MSK)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) break error- error \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) break error- ok \n");
	if(pReg->lowWord & PSCIP_FRAME_ERR_3_MSK)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) framing error- error \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) framing error - ok \n");	
	if(pReg->lowWord & PSCIP_COMP_ERR_3_MSK)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) compare error - error \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) compare error - ok \n");
	if(pReg->lowWord & PSCIP_TIMEOUT_ERR_3_MSK)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) timeout error- error \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) timeout error - ok \n");		

	//reg 3
	printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: -------REGISTER 4 ERRORS----------\n");
	if(pReg->lowWord & PSCIP_PAR_ERR_4_MSK)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) Priority erro or high priority write  - error \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) Priority erro or high priority write  - ok \n");
	if(pReg->lowWord & PSCIP_BREAK_ERR_4_MSK)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) break error- error \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) break error- ok \n");
	if(pReg->lowWord & PSCIP_FRAME_ERR_4_MSK)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) framing error- error \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) framing error - ok \n");	
	if(pReg->lowWord & PSCIP_COMP_ERR_4_MSK)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) compare error - error \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) compare error - ok \n");
	if(pReg->lowWord & PSCIP_TIMEOUT_ERR_4_MSK)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) timeout error- error \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) timeout error - ok \n");	
	printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: END ->> Reading and interpreting FPGA status register <<- END \n");
	printk(KERN_ALERT "\t\t DEBUG: IP_DEBUG_read_FPGA_stat() - end\n");		
}



void IP_DEBUG_read_IP_interface_reg(volatile struct IPLocalSpace0 *pReg)
{

	
	printk(KERN_ALERT "\t\t DEBUG: IP_DEBUG_read_IP_interface_reg() - beginnig\n");	
	printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: Reading and interpreting IP inteface register (not sure whether it can be read) \n");
	printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: Revision ID: 0x%x\n",pReg->revisionID);
//IPAcontrol
	printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: IP A control reg:\n");
	printk(KERN_ALERT "\t\t\t\t\t\t DEBUG:  word (binarry):  %d%d%d%d %d%d%d%d  %d%d%d%d %d%d%d%d\n",0x01&(pReg->IPAcontrol >>15),0x01&(pReg->IPAcontrol >>14),0x01&(pReg->IPAcontrol >>13),0x01&(pReg->IPAcontrol>>12),0x01&(pReg->IPAcontrol >>11),0x01&(pReg->IPAcontrol >>10),0x01&(pReg->IPAcontrol>>9),0x01&(pReg->IPAcontrol >>8),0x01&(pReg->IPAcontrol >>7),0x01&(pReg->IPAcontrol >>6),0x01&(pReg->IPAcontrol >>5),0x01&(pReg->IPAcontrol >>4),0x01&(pReg->IPAcontrol >>3),0x01&(pReg->IPAcontrol >>2),0x01&(pReg->IPAcontrol >>1),0x01&pReg->IPAcontrol);
//	printk(KERN_ALERT "IP_INT1_EN:  %d%d%d%d %d%d%d%d  %d%d%d%d %d%d%d%d\n",0x01&(pReg->IP_INT1_EN >>15),0x01&(pReg->IP_INT1_EN >>14),0x01&(pReg->IP_INT1_EN >>13),0x01&(pReg->IP_INT1_EN>>12),0x01&(pReg->IP_INT1_EN >>11),0x01&(pReg->IP_INT1_EN >>10),0x01&(pReg->IP_INT1_EN>>9),0x01&(pReg->IP_INT1_EN >>8),0x01&(pReg->IP_INT1_EN >>7),0x01&(pReg->IP_INT1_EN >>6),0x01&(pReg->IP_INT1_EN >>5),0x01&(pReg->IP_INT1_EN >>4),0x01&(pReg->IP_INT1_EN >>3),0x01&(pReg->IP_INT1_EN >>2),0x01&(pReg->IP_INT1_EN >>1),0x01&pReg->IP_INT1_EN);

	if(pReg->IPAcontrol & IP_INT1_EN)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) IP interrupt 1 enable \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) IP interrupt 1 disabled\n");
	
	if(pReg->IPAcontrol & IP_INT0_EN)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) IP interrupt 0 enable \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) IP interrupt 0 disabled\n");
	if(pReg->IPAcontrol & IP_INT1_SENSE)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) IP interrupt 1 edge sensitive \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) IP interrupt 1 level sensitive\n");
	if(pReg->IPAcontrol & IP_INT0_SENSE)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) IP interrupt 0 edge sensitive \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) IP interrupt 0 level sensitive\n");
	if(pReg->IPAcontrol & IP_ERR_INT_EN)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) IP timeout nterrupt enable \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) IP timeout interrupt disabled \n");
	if(pReg->IPAcontrol & IP_RECOVER)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) IP recover time enable \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) IP recover time disabled\n");
	if(pReg->IPAcontrol & IP_CLKRATE)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) 32 MHz,\n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) 8 MHz\n");
//IP B control
	printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: IP B control reg:\n");
	if(pReg->IPBcontrol & IP_INT1_EN)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) IP interrupt 1 enable \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) IP interrupt 1 disabled\n");
	
	if(pReg->IPBcontrol & IP_INT0_EN)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) IP interrupt 0 enable \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) IP interrupt 0 disabled\n");
	if(pReg->IPBcontrol & IP_INT1_SENSE)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) IP interrupt 1 level sensitive \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) IP interrupt 1 edge sensitive\n");
	if(pReg->IPBcontrol & IP_INT0_SENSE)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) IP interrupt 0 level sensitive \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) IP interrupt 0 edge sensitive\n");
	if(pReg->IPBcontrol & IP_ERR_INT_EN)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) IP timeout nterrupt enable \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) IP timeout interrupt disabled\n");
	if(pReg->IPBcontrol & IP_RECOVER)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) IP recover time enable \n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) IP recover time disabled\n");
	if(pReg->IPBcontrol & IP_CLKRATE)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) 32 MHz,\n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) 8 MHz\n");
//reset register		
	printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: IP Reset register:\n");
	if(pReg->RESET & IPB_RESET_MASK)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) IP  B RESET - signal is asserted,\n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) IP B RESET - signal is de-asserted\n");
	if(pReg->RESET & IPA_RESET_MASK)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) IP A RESET - signal is asserted,\n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) IP A RESET - signal is de-asserted\n");
//IP Status Register
	printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: IP Status register:\n");
	printk(KERN_ALERT "\t\t\t\t\t\t DEBUG:  word (binarry):  %d%d%d%d %d%d%d%d  %d%d%d%d %d%d%d%d\n",0x01&(pReg->STATUS >>15),0x01&(pReg->STATUS >>14),0x01&(pReg->STATUS >>13),0x01&(pReg->STATUS>>12),0x01&(pReg->STATUS >>11),0x01&(pReg->STATUS >>10),0x01&(pReg->STATUS>>9),0x01&(pReg->STATUS >>8),0x01&(pReg->STATUS >>7),0x01&(pReg->STATUS >>6),0x01&(pReg->STATUS >>5),0x01&(pReg->STATUS >>4),0x01&(pReg->STATUS >>3),0x01&(pReg->STATUS >>2),0x01&(pReg->STATUS >>1),0x01&pReg->STATUS);
//	printk(KERN_ALERT "IP_TIME_B:  %d%d%d%d %d%d%d%d  %d%d%d%d %d%d%d%d\n",0x01&(pReg->IP_TIME_B >>15),0x01&(pReg->IP_TIME_B >>14),0x01&(pReg->IP_TIME_B >>13),0x01&(pReg->IP_TIME_B>>12),0x01&(pReg->IP_TIME_B>>11),0x01&(pReg->IP_TIME_B >>10),0x01&(pReg->IP_TIME_B>>9),0x01&(pReg->IP_TIME_B >>8),0x01&(pReg->IP_TIME_B >>7),0x01&(pReg->IP_TIME_B >>6),0x01&(pReg->IP_TIME_B >>5),0x01&(pReg->IP_TIME_B >>4),0x01&(pReg->IP_TIME_B >>3),0x01&(pReg->IP_TIME_B >>2),0x01&(pReg->IP_TIME_B >>1),0x01&pReg->IP_TIME_B);

	if(pReg->STATUS & IP_TIME_B)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1)  IP_B timeout has occured\n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) IP_B timeout has not occured\n");
	if(pReg->STATUS & IP_TIME_A)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1)  IP_A timeout has occured\n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) IP_A timeout has not occured\n");

	if(pReg->STATUS & IP_ERR_B)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1)  IP_B ERROR# signal asseted\n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) IP_B ERROR# signal de-asserted\n");

	if(pReg->STATUS & IP_ERR_A)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) IP_A ERROR# signal asseted\n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) IP_A ERROR# signal de-asserted\n");

	if(pReg->STATUS & IP_INT1_B)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) Active IP_B interrupt 1 request\n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) no IP_B intrrupt 1 request\n");

	if(pReg->STATUS & IP_INT0_B)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) Active IP_B interrupt 0 request\n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) no IP_B intrrupt 0 request\n");

	if(pReg->STATUS & IP_INT1_A)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1)  Active IP_A interrupt 1 request\n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) no IP_A intrrupt 1 request\n");

	if(pReg->STATUS & IP_INT0_A)
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==1) Active IP_A interrupt 0 request\n");
	else
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: (value==0) no IP_A intrrupt 0 request\n");

		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: END ->> Reading and interpreting IP inteface register <<- END \n");
	printk(KERN_ALERT "\t\t DEBUG: IP_DEBUG_read_IP_interface_reg() - end\n");		
}


int IP_DEBUG_read_errors(pscip_Dev dev)
{

	int i ;
	pscip_fpga_t fpga_val;	
	
	printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: *** PSCIP Driver - reading errors ***\n");
	
	for (i = 0; i < 1; i++) {

		printk(KERN_ALERT  "\t\t\t\t\t\t DEBUG: ----- Channel %d -----\n",i);
		printk(KERN_ALERT  "\t\t\t\t\t\t DEBUG: Link status: ");

		fpga_val.channel = i;
		if (k_pscip_rdfpgastat(&fpga_val, 0, dev) < 0)
			goto out;
		if (fpga_val.high_data & PSCIP_LINKDOWN_MSK)
			printk(KERN_ALERT "\t\t\t\t\t\t DEBUG: link down\n");
		else if (fpga_val.high_data & PSCIP_TXERR_MSK)
			printk(KERN_ALERT  "\t\t\t\t\t\t DEBUG: tx error\n");				
		else {
			printk(KERN_ALERT  "\t\t\t\t\t\t DEBUG: ok\n");
			printk(KERN_ALERT  "\t\t\t\t\t\t DEBUG: Controller status:\n");
			if (dev->stats[i].cntrl_stat & PSCIP_INPUTBUF_MSK)
				printk(KERN_ALERT  " \t\t\t\t\t\t DEBUG:    - Buffer full\n");
			else
				printk(KERN_ALERT  "\t\t\t\t\t\t DEBUG:     - Buffer not full\n");			
			if (dev->stats[i].cntrl_stat & PSCIP_DSP_MSK)
				printk(KERN_ALERT " \t\t\t\t\t\t DEBUG:    - DSP stopped\n");
			else
				printk(KERN_ALERT "\t\t\t\t\t\t DEBUG:     - DSP running\n");	
			if (dev->stats[i].cntrl_stat & PSCIP_REMLOC_MSK)
				printk(KERN_ALERT  "\t\t\t\t\t\t DEBUG:     - Local control\n");
			else
				printk(KERN_ALERT  "\t\t\t\t\t\t DEBUG:     - Remote control\n");							
		}
			
		printk(KERN_ALERT  " \t\t\t\t\t\t DEBUG:  Rx packets: %d\n",dev->stats[i].rx);
		printk(KERN_ALERT  "\t\t\t\t\t\t DEBUG:   Tx packets: %d\n",dev->stats[i].tx);
		printk(KERN_ALERT  "\t\t\t\t\t\t DEBUG:   Tx errors:  %d\n",dev->stats[i].txerr_cnt);	
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG:   Link errors: %d\n",dev->stats[i].linkdown_cnt);
		printk(KERN_ALERT "\t\t\t\t\t\t DEBUG:   IP number: %d\n",dev->IPnumber);
	}	

	
	return 1;

	//IP error 
	out:
	printk(KERN_ERR "PSCIP error\n");	
	



	return 0;		//added for 2.6 - temporary to avoid warnings
}

//functions prints all it can now and can be usefull about the PCI carrier, basicaly issuing command "lspci -v should 
// provide similar information 
static unsigned char pscip_get_PCI_info(struct pci_dev *dev)
{
	u16 word;
	u8 revision;

	printk(KERN_ALERT "\t\tDEBUG: pscip_get_PCI_info() - beginning \n");	

	
	printk(KERN_ALERT "\t\t  ************ PSCI driver ***********\n");
	printk(KERN_ALERT "\t\t  *	Reading information about	  *\n");
	printk(KERN_ALERT "\t\t  *   (compact)PCI carrier from reg. *\n");
	printk(KERN_ALERT "\t\t  ************************************\n");

	
	pci_read_config_word(dev, PCI_VENDOR_ID, &word);
	printk(KERN_ALERT "\t\t DEBUG: vendor id (should be 1498) 0x%x\n", word);
	
	pci_read_config_word(dev, PCI_DEVICE_ID, &word);
	printk(KERN_ALERT "\t\tt DEBUG: device id(should be 0x3064): 0x%x\n", word);
	
	pci_read_config_word(dev, PCI_STATUS, &word);
	printk(KERN_ALERT "\t\t DEBUG: status (should be 0x0280): 0x%x\n", word);
	
	pci_read_config_byte(dev, PCI_REVISION_ID, &revision);
	printk(KERN_ALERT "\t\t DEBUG: revision id (should be 0x00): 0x%x\n", revision);
	
	pci_read_config_word(dev, PCI_SUBSYSTEM_VENDOR_ID, &word);
	printk(KERN_ALERT "\t\t DEBUG: subsystem vendor ID (0x0B ??): 0x%x\n", word);
	
	pci_read_config_word(dev, PCI_SUBSYSTEM_ID, &word);
	printk(KERN_ALERT "\t\t DEBUG: subsystem ID (0x1B??): 0x%x\n", word);

	printk(KERN_ALERT "\t\t DEBUG: interrupt line number: %d\n", dev->irq);	

	printk(KERN_ALERT "\t\t DEBUG: pscip_get_PCI_info() - end \n");	
	return 0;
}
