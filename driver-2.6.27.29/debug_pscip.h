/** \file debug_pscip.h
	Header defining structures used when debugging. 
*/
//structures used by debugging functions - simpler access than ioread/iowrite, 
//however ioread/write are recommended so for "real work" they are used
struct FPGAstatus_reg
	{
		u8	highWord;
		u8 errorCounter;
		u16 midWord;
		u16 lowWord;
	};		
//structure reflecting IP interface - used in IP_DEBUG_read_IP_interface_reg(volatile struct IPLocalSpace0 *pReg);
struct IPLocalSpace0
	{
		u16 revisionID;
		u16 IPAcontrol;
		u16 IPBcontrol;
		u16 NOTused1;
		u16 NOTused2;
		u16 RESET;
		u16 STATUS;
	};
//declaration of functions used for debuggin purposes
void IP_DEBUG_read_FPGA_stat(volatile struct FPGAstatus_reg *pReg);
void IP_DEBUG_read_IP_interface_reg(volatile struct IPLocalSpace0 *pReg);
static unsigned char pscip_get_PCI_info(struct pci_dev *dev);
