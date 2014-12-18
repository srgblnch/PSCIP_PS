/** \file pscip.h
	Header file used in the driver and device server. 
	Includes structures, ioctl commands and masks useful in the driver and DS
*
* 	GG - 18.07.205: First implementation 
*
* 	22/10/2008 - Start of porting the driver :
*	-	from 2.4.x to 2.6.x
*	- 	from VME to PCIGG 
* 
 */
 
#ifndef __PSCIPDEF_H__
#define __PSCIPDEF_H__

#ifdef __cplusplus
extern "C" {
#endif


/* ioctl data struct */
typedef struct {
	unsigned	int 	chan;  /* channel 0/1 */
	unsigned 	char 	stat,   /* status register */ 
						address;  /* address register */
				int 	data;							
} pscip_t;



/* ioctl fpga data struct */
typedef struct {
  unsigned int channel;  /* channel 0/1 */  
  unsigned short high_data,
	               mid_data,
	               low_data;
} pscip_fpga_t;

/* ioctl stats data struct */
typedef struct {  
  unsigned int channel;  /* channel 0/1 */ 
  unsigned int rx, /* rx successful counter */
	             tx, /* tx successful counter */
	             err_flag, /* global status error flag */
							 link_err_flag[5], /* link error flag (status) */
							 cntrl_stat, /* controller status */							 
							 linkdown_cnt,
							 txerr_cnt;
							 
} pscip_stats_t;


/*
 * Channel Status Register BitMasks (base+0,base+8,base+16...) (read/write)
 */
#define PSCIP_WRITEREAD_MODE_MSK  0x80 /* 1=write cycle; 0=read cycle (w/r)*/
#define PSCIP_ECHO_MSK            0x40 /* 1=write echo/send of requested data; 0=write cycle/read request (w/r) */
#define PSCIP_INPUTBUF_MSK        0x20 /* 1=buffer full; 0=buffer not full (r) */
#define PSCIP_DSP_MSK             0x10 /* 1=dsp stopped; 0=dsp running (r) */
#define PSCIP_REMLOC_MSK          0x8  /* 1=local control; 0=remote control (r) */
#define PSCIP_LINK_MSK            0x4  /* 1=link down; 0=link up (r) */

/*
 * Status Register BitMasks (base+56) (read only)
 */
#define PSCIP_ERROR_MSK        0x8000  /* 1=error; 0=ok */
#define PSCIP_RDWAVE_BUSY_MSK  0x10    /* 1=ready; 0=busy */
#define PSCIP_WRWAVE_BUSY_MSK  0x8     /* 1=ready; 0=busy */
#define PSCIP_READ_BUSY_MSK    0x4     /* 1=ready; 0=busy */
#define PSCIP_WRITE_BUSY_MSK   0x2     /* 1=ready; 0=busy */
#define PSCIP_SETCUR_BUSY_MSK  0x1     /* 1=ready; 0=busy */

/*
 * FPGA Status Register BitMasks (base+48) (write only)
 */
#define PSCIP_ERRCLEAR_MSK    0x8000   /* 1=clear counter; 0=don't care */
#define PSCIP_SETLOOP_MSK     0x1      /* 1=set loopback mode on; 0=set loopback mode off */

/*
 * FPGA Status Register BitMasks (base+48)=high (read only)
 */
/* bits 15..8 */ 
#define PSCIP_ERRCNT_MSK      0x8000   /* error counter */
#define PSCIP_LINKDOWN_MSK    0x80     /* 1=link error; 0=link ok */
#define PSCIP_TXERR_MSK       0x40     /* 1=tx error; 0=tx ok */
#define PSCIP_LOOPBACK_MSK    0x1      /* 1=loopback mode on; 0=loopback mode off */

/*
 * FPGA Status Register BitMasks (base+50)=mid (read only)
 */ 
#define PSCIP_PAR_ERR_0_MSK      0x4000  /* 1=parity error on register 0; 0=ok */
#define PSCIP_BREAK_ERR_0_MSK    0x2000  /* 1=break error on register 0; 0=ok */
#define PSCIP_FRAME_ERR_0_MSK    0x1000  /* 1=frame error on register 0; 0=ok */
#define PSCIP_COMP_ERR_0_MSK     0x800   /* 1=compare error on register 0; 0=ok */
#define PSCIP_TIMEOUT_ERR_0_MSK  0x400   /* 1=timeout error on register 0; 0=ok */
#define PSCIP_PAR_ERR_1_MSK      0x200   /* 1=parity error on register 1; 0=ok */
#define PSCIP_BREAK_ERR_1_MSK    0x100   /* 1=break error on register 1; 0=ok */
#define PSCIP_FRAME_ERR_1_MSK    0x80    /* 1=frame error on register 1; 0=ok */
#define PSCIP_COMP_ERR_1_MSK     0x40    /* 1=compare error on register 1; 0=ok */
#define PSCIP_TIMEOUT_ERR_1_MSK  0x20    /* 1=timeout error on register 1; 0=ok */
#define PSCIP_PAR_ERR_2_MSK      0x10    /* 1=parity error on register 2; 0=ok */
#define PSCIP_BREAK_ERR_2_MSK    0x8     /* 1=break error on register 2; 0=ok */
#define PSCIP_FRAME_ERR_2_MSK    0x4     /* 1=frame error on register 2; 0=ok */
#define PSCIP_COMP_ERR_2_MSK     0x2     /* 1=compare error on register 2; 0=ok */
#define PSCIP_TIMEOUT_ERR_2_MSK  0x1     /* 1=timeout error on register 2; 0=ok */

/*
 * FPGA Status Register BitMasks (base+52)=low (read only)
 */ 
#define PSCIP_PAR_ERR_3_MSK      0x200   /* 1=parity error on register 3; 0=ok */
#define PSCIP_BREAK_ERR_3_MSK    0x100   /* 1=break error on register 3; 0=ok */
#define PSCIP_FRAME_ERR_3_MSK    0x80    /* 1=frame error on register 3; 0=ok */
#define PSCIP_COMP_ERR_3_MSK     0x40    /* 1=compare error on register 3; 0=ok */
#define PSCIP_TIMEOUT_ERR_3_MSK  0x20    /* 1=timeout error on register 3; 0=ok */
#define PSCIP_PAR_ERR_4_MSK      0x10    /* 1=parity error on register 4; 0=ok */
#define PSCIP_BREAK_ERR_4_MSK    0x8     /* 1=break error on register 4; 0=ok */
#define PSCIP_FRAME_ERR_4_MSK    0x4     /* 1=frame error on register 4; 0=ok */
#define PSCIP_COMP_ERR_4_MSK     0x2     /* 1=compare error on register 4; 0=ok */
#define PSCIP_TIMEOUT_ERR_4_MSK  0x1     /* 1=timeout error on register 4; 0=ok */

/* FPGA link error bitmask */
#define PSCIP_PAR_ERR_MSK      0x10    /* 1=parity error; 0=ok */
#define PSCIP_BREAK_ERR_MSK    0x8     /* 1=break error; 0=ok */
#define PSCIP_FRAME_ERR_MSK    0x4     /* 1=frame error; 0=ok */
#define PSCIP_COMP_ERR_MSK     0x2     /* 1=compare error; 0=ok */
#define PSCIP_TIMEOUT_ERR_MSK  0x1     /* 1=timeout error; 0=ok */


/*
 * Link identification
 */
#define PSCIP_WRHPRIO_IDX    0    /* high priority write */ 
#define PSCIP_WRITE_IDX      1    /* write */ 
#define PSCIP_READ_IDX       2    /* read */ 
#define PSCIP_WRWAVE_IDX     3    /* write waveform */
#define PSCIP_RDWAVE_IDX     4    /* read waveform */

#define PSCIP_LINKDOWN_TOUT  4    /* wait 4 second to give data after the link up again (previously was down) */
/* 
 * Real-time thread status BitMask
 */
#define PSCIP_THREAD_RUN_MSK      0x1   /* 1=thread running; 0=thread stopped */  
#define PSCIP_THREAD_ERR_MSK      0x2   /* 1=thread errors; 0=thread ok */ 

/*
 * Ioctl commands
 */
#define PSCIP_IOCTL_MAGIC  0xB1B  
#define PSCIP_WRHIPRIO     _IOW  (PSCIP_IOCTL_MAGIC, 1, pscip_t) /* high priority write */
#define PSCIP_WRITE        _IOW  (PSCIP_IOCTL_MAGIC, 2, pscip_t) /* write */
#define PSCIP_READ         _IOWR (PSCIP_IOCTL_MAGIC, 3, pscip_t) /* read */
#define PSCIP_WRWAVE       _IOW  (PSCIP_IOCTL_MAGIC, 4, pscip_t) /* write waveform */
#define PSCIP_RDWAVE       _IOWR (PSCIP_IOCTL_MAGIC, 5, pscip_t) /* read waveform */
#define PSCIP_RDLINKSTAT   _IOWR  (PSCIP_IOCTL_MAGIC,6, pscip_t) /* read link status register */
#define PSCIP_RDFPGASTAT   _IOR  (PSCIP_IOCTL_MAGIC, 7, pscip_fpga_t) /* read fpga registers */
#define PSCIP_RDSTATISTICS _IOR  (PSCIP_IOCTL_MAGIC, 8, pscip_stats_t) /* read/write/errors statistics */
//#define PSCIP_VMEADDR      _IOR  (PSCIP_IOCTL_MAGIC, 9, unsigned long) /* get pscip vmeaddr */
#define PSCIP_PCIADDR      _IOR  (PSCIP_IOCTL_MAGIC, 9, unsigned long) /* get pscip vmeaddr */
#define PSCIP_RDTEST         _IOWR (PSCIP_IOCTL_MAGIC, 12, pscip_t) /* read(test version) */
#ifdef __FOFB__
#define PSCIP_WRHIPRIO_UNCHECKED    _IOWR (PSCIP_IOCTL_MAGIC, 13, pscip_t)
#define PSCIP_WRITE_UNCHECKED       _IOWR (PSCIP_IOCTL_MAGIC, 14, pscip_t)
#define PSCIP_READ_UNCHECKED        _IOWR (PSCIP_IOCTL_MAGIC, 15, pscip_t)
#define PSCIP_ENABLE_INTERRUPTIONS  _IOWR (PSCIP_IOCTL_MAGIC, 16, pscip_t)
#define PSCIP_DISABLE_INTERRUPTIONS _IOWR (PSCIP_IOCTL_MAGIC, 17, pscip_t)
#endif

#define PSCIP_RESET        _IO  (PSCIP_IOCTL_MAGIC, 17)           /* reset IP module */

//added functionality
#define PSCIP_CLRCOUNTER        _IOW  (PSCIP_IOCTL_MAGIC, 18, pscip_t) 
#define PSCIP_DUMPFPGAREGS        _IOW  (PSCIP_IOCTL_MAGIC, 19, pscip_t)           /* clear IP counter*/
#define PSCIP_WRITEIPREG        _IOW  (PSCIP_IOCTL_MAGIC, 10, pscip_t) /* read ip register */
#define PSCIP_READIPREG         _IOWR (PSCIP_IOCTL_MAGIC, 11, pscip_t) /* write IP register */
/* Error codes */
#define PSCIP_ERR_OFFSET   PSCIP_IOCTL_MAGIC
#define PSCIP_MISMATCH_ERR (PSCIP_ERR_OFFSET + 0)  /* Echo value doesn't match */
#define PSCIP_REMLOC_ERR   (PSCIP_ERR_OFFSET + 1)  /* Remote control disabled */
#define PSCIP_READ_ERR     (PSCIP_ERR_OFFSET + 2)  /* Generic reading error */
#define PSCIP_TOUT_ERR     (PSCIP_ERR_OFFSET + 3)  /* Reading timeout error */
#define PSCIP_BUFFER_ERR   (PSCIP_ERR_OFFSET + 4)  /* Input buffer full */
#define PSCIP_DSP_ERR      (PSCIP_ERR_OFFSET + 5)  /* Dsp stopped */
#define PSCIP_LINK_ERR     (PSCIP_ERR_OFFSET + 6)  /* Link is down */
#define PSCIP_TX_ERR       (PSCIP_ERR_OFFSET + 7)  /* Transmission error */
#define PSCIP_PARAM_ERR    (PSCIP_ERR_OFFSET + 8)  /* Parameter error */
#define PSCIP_BUS_ERR      (PSCIP_ERR_OFFSET + 9) /* VME bus error */

#define PSCIP_SIZE_MSG  80




#define PSCIP_MINOR_NUMBER  0;		//initial minor number
#define PSCIP_MAJOR_NUMBER  0;		//initial minor number
#ifdef __cplusplus
}
#endif

#endif

/* EOF */
