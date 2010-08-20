/** \file k_pscip.h
	Defines interface with kernel, device structure, IP addresses and masks.
 *
 *
 * 	GG - 18.07.2005: First implementation 
 *
 * 	22/10/2008 - Start of porting the driver :
 *	-	from 2.4.x to 2.6.x
 *	- 	from VME to PCIGG 
 * 	
 */

#ifndef __K_PSCIPDEF_H__
#define __K_PSCIPDEF_H__

#define PSCIP_SIZE  0x100  /* virtual mem size reserved */

/*
 * IP Registers addresses
 */
/* set PS current */ 
#define PSCIP_HPRIOWRITE_REG   	0x0 
#define PSCIP_HPRIOWRITE_STAT   0x1 //ML: changed from 0x0
#define PSCIP_HPRIOWRITE_ADDR   0x0 //ML: changed from 0x0
#define PSCIP_HPRIOWRITE_DATA   0x2 /* 4 bytes */
/* write */
#define PSCIP_WRITE_REG	        0x8 
#define PSCIP_WRITE_STAT        0x9	//ML: changed from 0x8 
#define PSCIP_WRITE_ADDR        0x8 //ML: changed from 0x9
#define PSCIP_WRITE_DATA        0xa /* 4 bytes */
/* read */
#define PSCIP_READ_REG	        0x10  
#define PSCIP_READ_STAT         0x11 //ML: changed from 0x11
#define PSCIP_READ_ADDR         0x10 //ML: changed from 0x11
#define PSCIP_READ_DATA         0x12 /* 4 bytes */
/* write waveform*/
#define PSCIP_WRWAVE_REG		0x18 
#define PSCIP_WRWAVE_STAT      	0x19 //ML: changed from 0x18
#define PSCIP_WRWAVE_ADDR      	0x18 //ML: changed from 0x19
#define PSCIP_WRWAVE_DATA      	0x1a /* 4 bytes */
/* read waveform*/
#define PSCIP_RDWAVE_REG	    0x20 
#define PSCIP_RDWAVE_STAT       0x20 
#define PSCIP_RDWAVE_ADDR       0x21
#define PSCIP_RDWAVE_DATA       0x22 /* 4 bytes */
/* broken transmit */
#define PSCIP_BRK_REG	        0x28 
#define PSCIP_BRK_STAT          0x28 
#define PSCIP_BRK_ADDR          0x29
#define PSCIP_BRK_DATA          0x2a /* 4 bytes */
/* FPGA status */
#define PSCIP_FPGA_REG	        0x30 /* 2 bytes */
#define PSCIP_FPGA_HIGH         0x30 /* 2 bytes */
#define PSCIP_FPGA_MID          0x32 /* 2 bytes */
#define PSCIP_FPGA_LOW          0x34 /* 2 bytes */
/* Registers status */
#define PSCIP_REG_STAT          0x38 /* 2 bytes */
/* Interrupt vector */           
#define PSCIP_IVEC_REG          0x3A /* 1 byte */ //ML: corrected address, it was 3A

/* 
 * IRQ and model registers
 */
 
#define PSCIP_PROM_OFFSET    0x80 - 0x01					// 0x01 - the addresses are shifted by 0x01 regarding to the documentation
#define PSCIP_MODEL_REG      0xB + PSCIP_PROM_OFFSET
#define PSCIP_MANIFAC_REG    0x9 + PSCIP_PROM_OFFSET
#define PSCIP_ILEV_REG       0xC1  /* register on carrier */
#define	PSCIP_RESET_REG      0xC3  /* register on carrier */
#define PSCIP_MANIFAC_ID     0xB
#define PSCIP_MODEL_ID       0x1B 
#define PSCIP_RESET_MASK     0x80

/*
 * proc_fs write codes
 */
#define PSCIP_PROC_HPRIO   1  /* set high priority PS current */
#define PSCIP_PROC_WRITE   2  /* write parameter */
#define	PSCIP_PROC_WRWAVE  3  /* write waveform values */
#define PSCIP_PROC_RESET   100  /* Reset IP */


/*
 * request irq flags
 */
#define PSCIP_IRQ_REQUEST  2
#define PSCIP_IRQ_SERVED   1
#define	PSCIP_IRQ_DONE     0

/* max number of devices supported */
#define PSCIP_MAX_DEV  20 
#define PSCIP_DEFAULT_DEV 8
#define MAX_NUMER_OF_IPs_ON_CARRIER 2
/* channel offset 
 * channel 0: offset=0
 * channel 1: offset=0x40
 *
*/	 
#define PSCIP_CHAN_OFFSET 0x40
#define PSCIP_IP_OFFSET 0x100
/* busy timeout */
#define PSCIP_BUSY_TOUT  150000	/* usec. */
//150

/* max number of readings per device */
#define PSCIP_MAX_READ 100

/* max number of readings per device */
#define PSCIP_MAX_CHAN 2

/* PCI  registration */
#define PCI_VENDOR_ID_TEWS 				0x1498
#define PCI_DEVICE_ID_TCP221_cPCI		0x20D3
#define PCI_DEVICE_ID_TCPI100_PCI		0x3064


/*PCI - IP interface addresses*/
#define IP_REVISION_ID					0x00
#define IP_A_CONTROL					0x02
#define IP_B_CONTROL					0x04
#define IP_RESET						0x0A
#define IP_STATUS						0x0C


/*IP control Register Masks*/
#define IP_INT1_EN						0x0080			//1 - IP interrupt 1 enable, 0 - IP interrupt 1 disabled
#define IP_INT0_EN						0x0040			//1 - IP interrupt 0 enable, 0 - IP interrupt 0 disabled
#define IP_INT1_SENSE					0x0020			//1 - IP interrupt 1 edge sensitive, 0 - IP interrupt 1 level sensitive
#define IP_INT0_SENSE					0x0010			//1 - IP interrupt 0 edge sensitive, 0 - IP interrupt 0 level sensitive
#define IP_ERR_INT_EN					0x0008			//1 - IP error interrupt enable, 0 - IP erro interrupt disabled
#define IP_TIME_INT_EN					0x0004			//1 - IP timeout nterrupt enable, 0 - IP timeout interrupt disabled
#define IP_RECOVER						0x0002			//1 - IP recover time enable, 0 - IP recover time disabled
#define IP_CLKRATE						0x0001			//1 - 32 MHz, 0- 8 MHz

/*IP Reset Register Masks*/
#define IPB_RESET_MASK					0x0002			// 1- IP  B RESET - signal is asserted; 0 - IP B RESET - signal is de-asserted
#define IPA_RESET_MASK					0x0001			// 1- IP  A RESET - signal is asserted; 0 - IP  BRESET - signal is de-asserted

/*IP Status Register Masks*/ 
#define IP_TIME_B						0x2000			//1 - IP_B timeout has occured; 0 - IP_B timeout has not occured
#define IP_TIME_A						0x1000			//1 - IP_A timeout has occured; 0 - IP_A timeout has not occured
#define IP_ERR_B						0x0200			//1 - IP_B ERROR# signal asseted; 0 - IP_B ERROR# signal de-asserted
#define IP_ERR_A						0x0100			//1 - IP_A ERROR# signal asseted; 0 - IP_A ERROR# signal de-asserted
#define IP_INT1_B						0x0008			//1 - Active IP_B interrupt 1 request; 0 - no IP_B intrrupt 1 request
#define IP_INT0_B						0x0004			//1 - Active IP_B interrupt 0 request; 0 - no IP_B intrrupt 0 request
#define IP_INT1_A						0x0002			//1 - Active IP_A interrupt 1 request; 0 - no IP_A intrrupt 1 request
#define IP_INT0_A						0x0001			//1 - Active IP_A interrupt 0 request; 0 - no IP_A intrrupt 0 request			

/*PCI - local space 1 used for IP A-B ID, int and I/O space  - addresses*/		
#define IP_A_IO_SPACE					0x00000000
#define IP_A_ID_SPACE					0x00000080
#define IP_A_INT_SPACE					0x000000C0

#define IP_B_IO_SPACE					0x00000100
#define IP_B_ID_SPACE					0x00000180
#define IP_B_INT_SPACE					0x000001C0
#define IP_INT0_ACK						0x000000C0
#define IP_INT1_ACK						0x000000C2


/* PCI interface register Masks -stolen from Ramon driver:)*/
#define PLX9030_INTCSR_LINTI1_ENA  0x0001 /* LINTi1 enable */
#define PLX9030_INTCSR_LINTI1_POL  0x0002 /* LINTi1 polarity, 1 = active high */
#define PLX9030_INTCSR_LINTI1_STAT 0x0004 /* LINTi1 status, 1 = interrupt active */
#define PLX9030_INTCSR_LINTI2_ENA  0x0008 /* LINTi2 enable */
#define PLX9030_INTCSR_LINTI2_POL  0x0010 /* LINTi2 polarity, 1 = active high */
#define PLX9030_INTCSR_LINTI2_STAT 0x0020 /* LINTi2 status, 1 = interrupt active */
#define PLX9030_INTCSR_PCI_IRQENA  0x0040 /* PCI interrupt enable, 1 = enabled */
#define PLX9030_INTCSR_SWINT       0x0080 /* Software interrupt, 1 = generate PCI IRQ */
#define PLX9030_INTCSR_LINTI1_SENA 0x0100 /* LINTi1 select enable, 0 = level, 1 = edge triggerable */
#define PLX9030_INTCSR_LINTI2_SENA 0x0200 /* LINTi1 select enable, 0 = level, 1 = edge triggerable */
#define PLX9030_INTCSR_LINTI1_ICLR 0x0400 /* LINTi1 edge triggerable IRQ clear, writing 1 clears irq */
#define PLX9030_INTCSR_LINTI2_ICLR 0x0800 /* LINTi2 edge triggerable IRQ clear, writing 1 clears irq */

#define FALSE 0
#define TRUE 1

/** 	Structure to mantain internal device driver variables.
	It embeds pci_dev, and cdev structures used for kernel interfacing
 */
typedef struct {	
	uint32_t 				pciaddr;  						/**< Phisical address of Base Address 0 PCI Carrier. Used in probe to check IP device number */		
	struct semaphore 		sem; 				 			/**< Semaphore struct. */
	pscip_stats_t 			stats[PSCIP_MAX_CHAN];			/**< Structure used for storing operatin statistics and flags */
	wait_queue_head_t 		queue[PSCIP_MAX_CHAN];			/**< Structure used in waitqueue mechanism. */
	spinlock_t 				lock[PSCIP_MAX_CHAN];			/**< Structure used in spinlock mechanism. */
//	spinlock_t 				*lock[PSCIP_MAX_CHAN];			
	int 					irqflag[PSCIP_MAX_CHAN];		/**< Interrupt flag. 0 - PSCIP_IRQ_DONE1; 1 - PSCIP_IRQ_SERVED; 2 - PSCIP_IRQ_REQUEST */
	int 					irqdata[PSCIP_MAX_CHAN];		/**< Interrupt data. Seems to be not used */
	struct tasklet_struct 	pscip_tsk[PSCIP_MAX_CHAN];		/**< Tasklet struct. */
//	struct tasklet_struct 	*pscip_tsk[PSCIP_MAX_CHAN];		/* tasklet struct */
	struct timeval 			tv[PSCIP_MAX_CHAN];				/**< To avoid ip lock during dsp reboot. */
	struct cdev 			cdev;	  						/**< Character device structure embedded in device structure. */
	unsigned long			PCIinterfaceAddr,				/**< Beginning of PCI interface registers.  PCI Carrier physical address of Base Address 0. */
							PCIinterfaceLen,				/**< Length of the memory area hodling PCI interface registers.  */
							PCIinterfaceFlags,				/**< PCI Carrier physical address of Base Address 0 flags. */
							IPinterfaceAddr,  				/**< Beginning of IP interface registers.  PCI Carrier physical address of Base Address 2. */
							IPinterfaceLen,					/**< Length of the memory area hodling IP interface registers.  */	
							IPinterfaceFlags,				/**< PCI Carrier physical address of Base Address 2 flags. */
							IP_IOspaceAddr,					/**< Beginning of I/O memory space. PCI Carrier physical address of Base Address 3. */
							IP_IOspaceLen,					/**< Length of the memory area hodling I/O memory space.  */
							IP_IOspaceFlags;				/**< PCI Carrier physical address of Base Address 3 flags. */
	void					*pPCIinterfaceReg;				/**< Virtual address of IP interface registers.  */
	void					*pIPinterfaceReg;				/**< Virtual address of PCI interface registers.  */
	void					*pIP_IOspace[PSCIP_MAX_CHAN];	/**< Virtual address of I/O memory space.  */

	int						IPnumber;						/** Number of IP on the PCI Carrier. There are max two IPs on one carrier: 0 , 1  */
	int						refcount;						/**Keeps the number of opened references to the device (opend files/nodes in /dev folder). */
	int						irq;							/** Interrupt line number. */
	struct pci_dev 			*pcidev;						/** PCI device struct embedded in device structure. Only the device structure of IP 0 (with IPnumber = 0) embeds this structure. */
	char                	device_name[10];				/** Name of the device representing it in the user space (both /dev and /proc/driver).  ex pscip0, pscip1,.... */
	int						pscip_number;					/** Number of the IP represented by the device structure connected to PCI bus. */
	int 					carrier_number;					/** Number of the carrier represented by the device structure connected to PCI bus. Should be: carrier_number = pscip_number % 2 */
	char					IP_in_slot;						/** Indicates whether the IP is in the Carrier slot. true = IP detected in the slot; false = checkmem failed */
} npscip_Dev, *pscip_Dev;


/*PCI 9030 chip interface register -stolen from Ramon driver:) */
struct Pci9030LocalConf
{
  unsigned int LAS0RR;    /* 0x00 Local Address Space 0 Range */
  unsigned int LAS1RR;    /* 0x04 Local Address Space 1 Range */
  unsigned int LAS2RR;    /* 0x08 Local Address Space 2 Range */
  unsigned int LAS3RR;    /* 0x0C Local Address Space 3 Range */
  unsigned int EROMRR;    /* 0x10 Expansion ROM Range */
  unsigned int LAS0BA;    /* 0x14 Local Address Space 0 Local Base Address */
  unsigned int LAS1BA;    /* 0x18 Local Address Space 1 Local Base Address */
  unsigned int LAS2BA;    /* 0x1C Local Address Space 2 Local Base Address */
  unsigned int LAS3BA;    /* 0x20 Local Address Space 3 Local Base Address */
  unsigned int EROMBA;    /* 0x24 Expansion ROM Local Base Address */
  unsigned int LAS0BRD;   /* 0x28 Local Address Space 0 Bus Region Descriptor */
  unsigned int LAS1BRD;   /* 0x2C Local Address Space 1 Bus Region Descriptor */
  unsigned int LAS2BRD;   /* 0x30 Local Address Space 2 Bus Region Descriptor */
  unsigned int LAS3BRD;   /* 0x34 Local Address Space 3 Bus Region Descriptor */
  unsigned int EROMBRD;   /* 0x38 Expansion ROM Bus Region Descriptor */
  unsigned int CS0BASE;   /* 0x3C Chip Select 0 Base Address */
  unsigned int CS1BASE;   /* 0x40 Chip Select 1 Base Address */
  unsigned int CS2BASE;   /* 0x44 Chip Select 2 Base Address */
  unsigned int CS3BASE;   /* 0x48 Chip Select 3 Base Address */
  unsigned short INTCSR;    /* 0x4C Interrupt Control/Status */
  unsigned short PROT_AREA; /* 0x4E Serial EEPROM Write-Protected Address Boundary */
  unsigned int CNTRL;     /* 0x50 PCI Target Response, Serial EEPROM, and Initialization Control */
  unsigned int GPIOC;     /* 0x54 General Purpose I/O Control */
  unsigned int reserved1; /* 0x58 */
  unsigned int reserved2; /* 0x5C */
  unsigned int reserved3; /* 0x60 */
  unsigned int reserved4; /* 0x64 */
  unsigned int reserved5; /* 0x68 */
  unsigned int reserved6; /* 0x6C */
  unsigned int PMDATASEL; /* 0x70 Hidden 1 Power Management Data Select */
  unsigned int PMDATASCALE; /* 0x74 Hidden 2 Power Management Data Scale */
};

/* proc filesystem device structure */
typedef struct {
	 int count;  /* index of devices in proc fs */
} npscip_Proc, *pscip_Proc;





/* kernel level API's */
int k_pscip_wait_linkdown(pscip_t *, int, pscip_Dev);
int k_pscip_wrhiprio(pscip_t *,int , pscip_Dev);					
int k_pscip_write(pscip_t *,int , pscip_Dev);
int k_pscip_read(pscip_t *,int , pscip_Dev);
int k_pscip_read_test(pscip_t *val, int id_pscip, pscip_Dev dev);
int k_pscip_wrwave(pscip_t *,int , pscip_Dev);
int k_pscip_rdwave(pscip_t *,int , pscip_Dev);
int k_pscip_wrstat(pscip_t *,int , pscip_Dev);
int k_pscip_rdlinkstat(pscip_t *,int , pscip_Dev);
int k_pscip_rdfpgastat(pscip_fpga_t *,int , pscip_Dev);
int k_pscip_rdstatistics(pscip_stats_t *,int , pscip_Dev);
int k_pscip_reset(int, pscip_Dev);
int k_pscip_clear_err_counter(pscip_t *val, int id_pscip, pscip_Dev dev);
int k_pscip_dump_all_registers( pscip_t *val, int id_pscip, pscip_Dev dev);
/* Interfacing IP */
void IP_read_byte(void *address,  uint8_t *val);
void IP_read_word(void *address, uint16_t *val);
void IP_write_byte(void *address,  uint8_t val);
void IP_write_word(void *address,  uint16_t val);
#endif

/* EOF */
