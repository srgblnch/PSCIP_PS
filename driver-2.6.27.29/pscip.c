/** \file pscip.c
* 	PSCIP Driver for Linux Kernel 2.6.x-development version - main file.
 *
 * 	GG - 19.07.2005: First implementation 
 *
 * 	22/10/2008 - Start of porting the driver :
 *	-	from 2.4.x to 2.6.x
 *	- 	from VME to PCIGG 
 * 	12/20008 - finalizing first release
 */
 
#include <linux/init.h>				
#include <linux/module.h>
#include <linux/moduleparam.h>		/* Defines parameter interface. */
#include <linux/fs.h>				/* Defines character device interface. 
										* for dynamic allocation of device numbers
										* defines register_chrdev_region()  & alloc_chrdev_)region() & etc
										* defines struct file_operation
										* defines struct file
									*/
#include <linux/types.h>			/* Defines devices tructures & types. 
										* device structure which holds device numbers (dev_t)
									*/
#include <linux/kdev_t.h>			/* Defines interface with dev_t structure.
											* set of macros to obtain major & minor parts of a dev_t (MINOR, MAJOR,MKDEV)	
									*/
#include <linux/cdev.h>				/* Defines cdev structure and interface.
										* struct cdev and its associated helper functions
									*/
#include <linux/kernel.h>			/* Defines kernel related macros/fuctions.
										* container_of() macro
										* defines barrier()
									*/
#include <linux/pci.h>				/*   Defines PCI interface.
										* provides function to access configuration space of PCI
										* defines symbolic names for config reg addresses
									*/
#include <linux/ioport.h>			/* Defines IO interface	
										* defines all resource flags  used to define some features of the individual resource in PCI
										* included in <linux/pci.h> so probably no need to include her
									*/
#include <linux/interrupt.h>		/* Defines interrupt interface
										* neede for interupbs
										* need for tasklets
									*/
/*#include <asm/spinlock.h>*/			/* Defines spinlock interface. 
										* added for 2.6 - strange because spinlock mechanism was used before but this was not included, maybe other header included spinlock
										* needed for spinlock mechanims
									*/

#include <asm/system.h>				/* architecture specific memory barriers */

#include <asm/io.h>					/* Defines IO platform specific functions. 
										* ioread8/ioread16
										* iowrite8/iowrite16		
									*/
#include <linux/proc_fs.h>			/* Defines proc_fs interface -essential for implementing proc file system */
									
#include <linux/errno.h>
//#include <linux/tqueue.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>
//#include <linux/byteorder.h>					/* Defines macros to convert between Big Endian and Little Endian */

//#define __W_R_TIME__					//if defined, the time of hardware operation (read/write) is measured
//#define __DEBUG__					
									/**< If defined, enables debug printk to logfile.
										Can be very useful when trying to determine bug location
										Every function prints information when beginning and when finishing
										Functions whose code is in pscip.c file print informatin starting with DEBUG_M and are not intended
										Fundtions whose code is in k_pscip.c file print information starting with DEBUG_K and are intended
										Funnctions whose code is in debug_pscip. file  print information starting with DEBUG and are even more inteded
									*/


	
/** Determines the driver's licencing. If missing the kernel shouts about taining it*/
MODULE_LICENSE("GPL");	


static int queue_flag = 0;						/**< Flag used in waitqueue mechanism. This is a provisional solution, probablyl needs better implementation */


#ifdef __DEBUG__
	#include "debug_pscip.h"		
#endif

#include "pscip.h"					
#include "k_pscip.h"				
#include "k_pscip.c"			

#ifdef __DEBUG__
	#include "debug_pscip.c"		
#endif

int pscip_minor = PSCIP_MINOR_NUMBER;			/**< Global variable with first minor numer associated with the driver.  */

static char ver[10] = "1.0";					/**< Module/driver version. */		
const char *module_name = "pscip";				/**< module name. */

//driver paramters
static int pscip_dev_number = 0;				/**< Module parameter - number of IPs. Number of  devices (IP modules) which will be in the computer, if not set, the DEFAULT number will be give, is limited by the MAX_DEV number */
static int hack_driver = 0;						/**< Module paramter - options for developer. At the moment ot used */
static int pscip_major = PSCIP_MAJOR_NUMBER		/**< Global variable with major numer associated with the driver. A default major number can be set in the header, 
												if defalut value is 0 the major number is set dynamically unless user pass major number as a parameter  */
//parameters registration												
	/** Macro defining pscip_dev_number module parameters. */
	module_param(pscip_dev_number, int, 0);		

	/** Macro defining hack_driver module parameters. */													
	module_param(hack_driver, int, 0);																	
															
	/** Macro defining pscip_major module parameters. */	
	module_param(pscip_major, int, 0);																	

//module param description
	MODULE_PARM_DESC(pscip_major, "PSCIP major number, if defined by user");
	MODULE_PARM_DESC(pscip_dev_number, "PSCIP number of devices which can be run using this driver");
	MODULE_PARM_DESC(hack_driver, "PSCIP special functions for developers:\n -- no func implemented at the moment --");
//interrupt functions
	/** Tasklet used to send a SIGIO to user space, chan 0. */
	void pscip_irq_bh0(unsigned long);
	
	/** Tasklet used to send a SIGIO to user space, chan 0. */
	void pscip_irq_bh1(unsigned long);  
	
	/** Function registering interrupt in the cernel.*/
	int pscip_assign_irq(pscip_Dev dev);
	
	/** Interrupt hanlder. */
	irqreturn_t pscip_irq_handler(int pciirq, void *dev_id);
	
// File operation prototypes 
	/** Function called when the node represeting driver is opened. */
	static int pscip_open(struct inode *, struct file *);
	
	/** Function called when the node represeting driver is closed. */
	static int pscip_release(struct inode *, struct file *);
	
	/** Function implementing ioctl control. */
	static int pscip_ioctl(struct inode *, struct file *, unsigned int, unsigned long);
	
//PCI functions
	/** Function called when the device (Carrier card) is detected on the PCI bus.  */
	static int probe(struct pci_dev *dev, const struct pci_device_id *id);
	
	/** Function called when the device (Carrier) is removed from the PCI bus. */
	static void remove(struct pci_dev *pcidev);
//data function
	/** Function checks data retreived from the user. */
	 static int pscip_check_input_data(pscip_t val);


/** 	File operation structure.  
	Associates the driver (major number)  with device operations. 
	The structure is a collection of function pointers. Each open file is associated with its own set of functions 
*/
static struct file_operations pscip_fops = 
{
	.owner		=   THIS_MODULE,
	.ioctl		=	pscip_ioctl,
	.open		=	pscip_open,
	.release	=	pscip_release
	
};

/**	Structure describing PCI device (IP). 
	Structure consists of a number of function callbacks and variables that describe the PCI driver to the PCI core 
*/
static struct pci_driver pscip_driver = {
	.name = "pscip",
	.id_table = ids,
	.probe = probe,
	.remove = remove,
};

		

static pscip_Proc pscip_procdev;  					/**<  Proc filesystem structure . */
static struct proc_dir_entry  *proc_model_dir;		/**< Structure defining entry in proc file system. */


/*****************************************************************
 *                           Sys calls                           *
 *****************************************************************/
/**
	Does any initialization for later operations.
	While creating first reference (first file/node open) interrupt on give IP are enabled
	
	@param *inode - contains cdev structure, allows to get our device structure
	@param *filp pointer to a file structure	
	
 */
 static int pscip_open(struct inode *inode,struct file *filp)
{

	pscip_Dev dev; 																	// device information 
	int err = 0;																	/** Error code. Store error code in case of errro */
	dev = container_of(inode->i_cdev, npscip_Dev, cdev);							//getting the device structure out of char dev structure
	filp->private_data = dev; 														//storing pointer to device information (pscip_Dev strucutre) in private_date for easier acces in future

	#ifdef __DEBUG__
	printk(KERN_ALERT "DEBUG_M: pscip_open() - start \n");	
	printk(KERN_ALERT "Openned device with name: %s, carrier number: %d, IP number: %d,IP number on the carrier: %d \n",dev->device_name,dev->carrier_number, dev->pscip_number,dev->IPnumber);	

	#endif

	if (down_interruptible(&dev->sem))												//added by ML!!!!!!!!!!!!
		return -ERESTARTSYS;

	if(!dev->refcount)																//enabling/setting interrupts when oppening the device for the first time
	{
		err = pscip_assign_irq(dev);
		if(err)																		//if not able to assing interrupt, return the error
			goto err_assign_irq;

		err = k_irq_enable(dev);													//hardware interrupt initialization/enablin
		if(err)																		//if not able to assing interrupt, return the error
			goto err_irq_enable;
	}
	dev->refcount++;																// increment the reference coutner
	up(&dev->sem);																	//added by ML!!!!!!!!!!!!!
	
	#ifdef __DEBUG__
	printk(KERN_ALERT "DEBUG_M: pscip_open() - end \n");	
	#endif

	return 0;  																		// success 
	
	err_irq_enable:
		free_irq(dev->irq, (void *) dev);
	err_assign_irq:
		up(&dev->sem);	
	return err;
}
/* end pscip_open() */


/**
	Invoked when the file structure is being released. If the last reference is closed, interrupts are disabled on give IP represented by the file/node
	
	@param *inode - contains cdev structure, allows to get our device structure
	@param *filp pointer to a file structure

 */
 static int pscip_release(struct inode *inode,struct file *filp)
{

	pscip_Dev dev; 																	// device information 
	int err = 0;																	// error code
	dev = container_of(inode->i_cdev, npscip_Dev, cdev);							//getting the device structure out of char dev structure
	filp->private_data = dev;
	
	#ifdef __DEBUG__
	printk(KERN_ALERT "DEBUG_M: pscip_release() - beginnig \n");	
	#endif
	
	if (down_interruptible(&dev->sem))												//added by ML!!!!!!!!!!!!
		return -ERESTARTSYS;

	if (dev->refcount)																//decrease reference counter
		dev->refcount--;

	if(!dev->refcount)																//if last closed reference, disable interrupts
	{
		err = k_irq_disable(dev);	
		if(err)																		//if not able to assing interrupt, return the error
			goto err_irq_disable;		
		free_irq(dev->irq, (void *) dev);											//Release interrupt handler when device is closed for the last time.

	}

	up(&dev->sem);																	//added by ML

	#ifdef __DEBUG__
	printk(KERN_ALERT "DEBUG_M: pscip_release() - end \n");	
	#endif
	
	return 0;

	err_irq_disable:
		up(&dev->sem);	
		return err;
}

/* end pscip_release() */


/**
	Used to read-write I/O operations and IP configuration 
	
	@param *pscip_t	- data retreived from the user to be checked
	
	@return 	- 0 if ok, -1 if error

 */
 static int pscip_check_input_data(pscip_t val)
{
	if(val.chan != 0 && val.chan != 1)
		return -PSCIP_PARAM_ERR;
	return 0;
}


/**
	Used to read-write I/O operations and IP configuration 
	
	@param *inode	- contains cdev structure, allows to get our device structure
	@param *filp 	- pointer to a file structure
	@param cmd	- command to be executed
	@param arg		- data passed from/to kernel space

 */
 static int pscip_ioctl(struct inode *inode,struct file *filp,unsigned int cmd, unsigned long arg)
{
	pscip_t val;
	int ret;
	pscip_fpga_t fpga_val;
	pscip_stats_t stats_val;
	int id_pscip = -1; 												// use driver data struct 	
	pscip_Dev dev = filp->private_data;  							// device information
	#ifdef __W_R_TIME__
	struct timeval w_r_time_1;											//used for time measurement
	struct timeval w_r_time_2;
	#endif
	#ifdef __DEBUG__
	printk(KERN_ALERT "DEBUG_M: pscip_ioctl() beginnig \n");	
	#endif
	
// lock ioctl to avoid race conditions 
	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;
	
	switch (cmd) 
	{			
	//Set high priority ps-current 
		case PSCIP_WRHIPRIO :	
		
			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: --\t Setting current -beginning\t--\n");
			#endif
			
			if (copy_from_user (&val, (void *) arg, sizeof (pscip_t))) 
			{		
				up(&dev->sem);
				return (-EFAULT);		
			}
			
			if ((ret = pscip_check_input_data(val)) < 0) 			
			{
				up(&dev->sem);
				return (-ret);
			}

			if ((ret = k_pscip_wrhiprio(&val, id_pscip, dev)) < 0) {				
				up(&dev->sem);
				return (-ret);
			}	
			
			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: --\t Setting current -finished\t--\n");
			#endif			
			
			break;			
	
	// Write action					
		case PSCIP_WRITE :	
			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: --\t Writing - beginning \t--\n");
			#endif		
			#ifdef __W_R_TIME__
			do_gettimeofday(&w_r_time_1);
			#endif

			if (copy_from_user (&val, (void *) arg, sizeof (pscip_t)))
			{			
				up(&dev->sem);
				return (-EFAULT);		
			}

			if ((ret = pscip_check_input_data(val)) < 0) 			
			{
				up(&dev->sem);
				return (-ret);
			}
		
			if ((ret = k_pscip_write(&val, id_pscip, dev)) < 0) 
			{			
				 up(&dev->sem);
				 return (-ret);
			}
			#ifdef __W_R_TIME__
			do_gettimeofday(&w_r_time_2);
			printk(KERN_ALERT "TIME: time of writing to PSC: %ld [usec]\n", (long)(w_r_time_2.tv_usec - w_r_time_1.tv_usec));
			#endif
			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: --\t Writing - finished \t--\n");
			#endif		
			
			break;
	
	// Read action 		
		case PSCIP_READ:

			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: --\t Reading - beginning \t--\n");
			#endif	
			#ifdef __W_R_TIME__
			do_gettimeofday(&w_r_time_1);
			#endif

			if (copy_from_user (&val, (void *) arg, sizeof (pscip_t))) 
			{			
				up(&dev->sem);
				return (-EFAULT);		
			}
	
			if ((ret = pscip_check_input_data(val)) < 0) 			
			{
				up(&dev->sem);
				return (-ret);
			}
	
			if ((ret = k_pscip_read(&val, id_pscip, dev)) < 0) 
			{
				up(&dev->sem);
				  return (-ret);
			}			

		//copy entire I/O for link 1 memory to user space
			if (copy_to_user ((void*) arg, &val, sizeof (pscip_t))) 
			{
				up(&dev->sem);				
				return (-EFAULT);
			}
			#ifdef __W_R_TIME__
			do_gettimeofday(&w_r_time_2);
			printk(KERN_ALERT "TIME: time of reading from PSC: %ld [usec]\n", (long)(w_r_time_2.tv_usec - w_r_time_1.tv_usec));
			#endif
			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: --\t Reading - finished \t--\n");
			#endif	
			
			break;	
	// Read action (used for testing IPs) 		
		case PSCIP_RDTEST:

			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: --\t Reading - beginning \t--\n");
			#endif	
			#ifdef __W_R_TIME__
			do_gettimeofday(&w_r_time_1);
			#endif
			if (copy_from_user (&val, (void *) arg, sizeof (pscip_t))) 
			{			
				up(&dev->sem);
				return (-EFAULT);		
			}
		
			if ((ret = pscip_check_input_data(val)) < 0) 			
			{
				up(&dev->sem);
				return (-ret);
			}

			if ((ret = k_pscip_read_test(&val, id_pscip, dev)) < 0) 
			{
				up(&dev->sem);
				  return (-ret);
			}			

		//copy entire I/O for link 1 memory to user space
			if (copy_to_user ((void*) arg, &val, sizeof (pscip_t))) 
			{
				up(&dev->sem);				
				return (-EFAULT);
			}
			#ifdef __W_R_TIME__
			do_gettimeofday(&w_r_time_2);
			printk(KERN_ALERT "TIME: time of reading from PSC: %ld [usec]\n", (long)(w_r_time_2.tv_usec - w_r_time_1.tv_usec));
			#endif
			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: --\t Reading - finished \t--\n");
			#endif	
			
			break;	
	
	// Write waveform action 		
		case PSCIP_WRWAVE :		

			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: --\t Wave writing - beginning \t--\n");
			#endif	
			
			if (copy_from_user (&val, (void *) arg, sizeof (pscip_t))) 
			{			
				up(&dev->sem);
				return (-EFAULT);		
			}		
			
			if ((ret = pscip_check_input_data(val)) < 0) 			
			{
				up(&dev->sem);
				return (-ret);
			}
			
			if ((ret = k_pscip_wrwave(&val, id_pscip, dev)) < 0) 
			{				
				up(&dev->sem);				
				return (-ret);
			}	

			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: -\t Wave writing - finsihed \t--\n");
			#endif				
			break;
	
		case PSCIP_RDWAVE:
			
			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: --\t Wave reading - beginning \t--\n");
			#endif
			
		// Read waveform action 		 
			if (copy_from_user (&val, (void *) arg, sizeof (pscip_t))) 
			{			
				up(&dev->sem);
				return (-EFAULT);		
			}
		
			if ((ret = pscip_check_input_data(val)) < 0) 			
			{
				up(&dev->sem);
				return (-ret);
			}

			if ((ret = k_pscip_rdwave(&val, id_pscip, dev)) < 0) 
			{
				up(&dev->sem);				
				return (-ret);
			}	

			if (copy_to_user ((void*) arg, &val, sizeof (pscip_t))) 
			{
				up(&dev->sem);				
				return (-EFAULT);
			}
			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: --\t Wave reading - finished \t--\n");
			#endif	
		
			break;						
			
	//Read link register status 		
		case PSCIP_RDLINKSTAT:
			
			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: --\t reading link status - beginning \t--\n");
			#endif	
						
			if ((ret = k_pscip_rdlinkstat(&val, id_pscip, dev)) < 0) 
			{
				up(&dev->sem);				
				return (-ret);
			}				

			if (copy_to_user ((void*) arg, &val, sizeof (pscip_t))) 
			{
				up(&dev->sem);				
				return (-EFAULT);
			}
			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: --\t reading link status - finished \t--\n");
			#endif	
			
			break;
	// Read FPGA status 		
		case PSCIP_RDFPGASTAT:
		
			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: --\t reading FPGA status - beginning \t--\n");
			#endif	
			
			if ((ret = k_pscip_rdfpgastat(&fpga_val, id_pscip, dev)) < 0) 
			{
				up(&dev->sem);				
				return (-ret);
			}				
		
			if (copy_to_user ((void*) arg, &fpga_val, sizeof(pscip_fpga_t))) 
			{
				up(&dev->sem);				
				return (-EFAULT);
			}
			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: --\t reading FPGA status - finished \t--\n");
			#endif	
			break;		
			
	// Read communication statistics 	
		case PSCIP_RDSTATISTICS:
			
			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: --\n reading statistics - beginning \t--\n");
			#endif	
			
			if ((ret = k_pscip_rdstatistics(&stats_val, id_pscip, dev)) < 0) 
			{
				up(&dev->sem);				
				return (-ret);
			}				
			if (copy_to_user ((void*) arg, &stats_val, sizeof (pscip_stats_t))) 
			{
				up(&dev->sem);				
				return (-EFAULT);
			}
			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: --\t reading statistics - finished \t--\n");
			#endif	
			break;		
		
	//PCI carrier physical address of the beginning of interface registers:
		case PSCIP_PCIADDR:
	
			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: --\t reading PCI address - beginning \t--\n");
			#endif	
			
			ret = dev->pciaddr;	
	
			if (copy_to_user((void*) arg, &ret, sizeof (uint32_t))) 
			{
				up(&dev->sem);
				return (-EFAULT);
			}	
			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: ---\t reading PCI address - finished \t--\n");
			#endif			
			break;		
		
		case PSCIP_RESET:
			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: --\t reseting - beginning \t--\n");
			#endif	

			if ((ret = k_pscip_reset(id_pscip, dev)) < 0) 
			{
				up(&dev->sem);				
				return (-ret);
			}
	
			val.address = 0x0;
			val.stat = 0x0;
			val.data=0x0;
			val.chan = 0;
			if ((ret = k_pscip_read(&val, id_pscip, dev)) < 0) 
			{
				up(&dev->sem);
				  return (-ret);
			}	
	
			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: --\t finished - beginning \t--\n");
			#endif	
			break;	
			
		// Write given IP register - using pscip_t structure to pass the value from user, s
		// if pscip_t val;
		// we use val.stat -> to keep value to be writen to given IP register
		// we use val.address for keeping address
			case PSCIP_WRITEIPREG :		
			
			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: --\t IP register writing - beginning \t--\n");
			#endif	
			
			if (copy_from_user (&val, (void *) arg, sizeof (pscip_t))) 
			{			
				up(&dev->sem);
				return (-EFAULT);		
			}		
			IP_write_byte(dev->pIP_IOspace[val.chan]+(val.address), val.stat);

			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: --\t IP register writing - finsihed \t--\n");
			#endif				
			break;
		// read given IP register - using pscip_t structure to pass the value from user, s
		// if pscip_t val;
		// we use val.stat -> to keep value to be writen to given IP register
		// we use val.address for keeping address
		case PSCIP_READIPREG:
			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: --\t IP register reading - beginning \t--\n");
			#endif	
		 
			if (copy_from_user (&val, (void *) arg, sizeof (pscip_t))) 
			{			
				up(&dev->sem);
				return (-EFAULT);		
			}
			
			IP_read_byte(dev->pIP_IOspace[val.chan]+(val.address), &(val.stat));

			if (copy_to_user ((void*) arg, &val, sizeof (pscip_t))) 
			{
				up(&dev->sem);				
				return (-EFAULT);
			}
			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: --\t IP register reading - finished \t--\n");
			#endif	
		
			break;						

	//clear FPGA error counter	
		case PSCIP_CLRCOUNTER:
			
			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: --\t Clearing FPGA errro counter - beginning \t--\n");
			#endif	
			
			if (copy_from_user (&val, (void *) arg, sizeof (pscip_t))) 
			{			
				up(&dev->sem);
				return (-EFAULT);		
			}
			if ((ret = k_pscip_clear_err_counter(&val,id_pscip, dev)) < 0) 
			{
				up(&dev->sem);				
				return (-ret);
			}
			
			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: ---\t Clearing FPGA errro counter - finished \t--\n");
			#endif	
			
			break;
			
	//dump all IP registers		
		case PSCIP_DUMPFPGAREGS:

			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: --\t Reading content of all FPGA registers - beginning \t--\n");
			#endif	
			
			if (copy_from_user (&val, (void *) arg, sizeof (pscip_t))) 
			{			
				up(&dev->sem);
				return (-EFAULT);		
			}
			
			k_pscip_dump_all_registers(&val, id_pscip, dev);
			
			#ifdef __DEBUG__
			IP_DEBUG_read_FPGA_stat(dev->pIP_IOspace[val.chan]+PSCIP_FPGA_REG);
			IP_DEBUG_read_IP_interface_reg(dev->pIPinterfaceReg);
			printk(KERN_ALERT "DEBUG_M: --\t Reading content of all FPGA registers- finished \t--\n");
			#endif	
			
			break;		

#ifdef __FOFB__
		//Set high priority ps-current with no checking
		case PSCIP_WRHIPRIO_UNCHECKED :

			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: --\t Writing High Priority UNCHECKED - beginning\t--\n");
			#endif

			if (copy_from_user (&val, (void *) arg, sizeof (pscip_t))) 
			{
				up(&dev->sem);
				return (-EFAULT);
			}

			if ((ret = pscip_check_input_data(val)) < 0)
			{
				up(&dev->sem);
				return (-ret);
			}

			if ((ret = k_pscip_wrhiprio_unchecked(&val, id_pscip, dev)) < 0) {
				up(&dev->sem);
				return (-ret);
			}

			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: --\t Writing High Priority UNCHECKED - finished\t--\n");
			#endif
			break;

		// Write with no checking
		case PSCIP_WRITE_UNCHECKED :
			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: --\t Writing UNCHECKED - beginning \t--\n");
			#endif
			#ifdef __W_R_TIME__
			do_gettimeofday(&w_r_time_1);
			#endif

			if (copy_from_user (&val, (void *) arg, sizeof (pscip_t)))
			{
				up(&dev->sem);
				return (-EFAULT);
			}

			if ((ret = pscip_check_input_data(val)) < 0)
			{
				up(&dev->sem);
				return (-ret);
			}

			if ((ret = k_pscip_write_unchecked(&val, id_pscip, dev)) < 0)
			{
				 up(&dev->sem);
				 return (-ret);
			}
			#ifdef __W_R_TIME__
			do_gettimeofday(&w_r_time_2);
			printk(KERN_ALERT "TIME: time of writing to PSC UNCHECKED: %ld [usec]\n", (long)(w_r_time_2.tv_usec - w_r_time_1.tv_usec));
			#endif
			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: --\t Writing UNCHECKED - finished \t--\n");
			#endif
			break;

		// Read without checking
		case PSCIP_READ_UNCHECKED:
			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: --\t Reading unchecked - beginning \t--\n");
			#endif
			#ifdef __W_R_TIME__
			do_gettimeofday(&w_r_time_1);
			#endif

			if (copy_from_user (&val, (void *) arg, sizeof (pscip_t)))
			{
				up(&dev->sem);
				return (-EFAULT);
			}

			if ((ret = pscip_check_input_data(val)) < 0)
			{
				up(&dev->sem);
				return (-ret);
			}

			if ((ret = k_pscip_read_unchecked(&val, id_pscip, dev)) < 0) 
			{
				up(&dev->sem);
				return (-ret);
			}

			//copy entire I/O for link 1 memory to user space
			if (copy_to_user ((void*) arg, &val, sizeof (pscip_t)))
			{
				up(&dev->sem);
				return (-EFAULT);
			}
			#ifdef __W_R_TIME__
			do_gettimeofday(&w_r_time_2);
			printk(KERN_ALERT "TIME: time of reading from PSC: %ld [usec]\n", (long)(w_r_time_2.tv_usec - w_r_time_1.tv_usec));
			#endif
			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: --\t Reading unchecked - finished \t--\n");
			#endif
			break;

		// Enable interruptions
		case PSCIP_ENABLE_INTERRUPTIONS :
			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: --\t Enabling interruptions - beginning \t--\n");
			#endif
			//hardware interrupt enabling
			ret = k_irq_enable(dev);
			if (ret) {
				printk(KERN_ALERT "ERROR: --\t ERROR ENABLING IRQ: 0x%X\n", ret);
				free_irq(dev->irq, (void *) dev);
			}
			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: --\t Enabling interruptions - finished \t--\n");
			#endif
			break;

		// Disable interruptions
		case PSCIP_DISABLE_INTERRUPTIONS :
			//hardware interrupt disabling
			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: --\t Disabling interruptions - beginning \t--\n");
			#endif
			ret = k_irq_disable(dev);
			if (ret) {
				printk(KERN_ALERT "ERROR: --\t ERROR DISABLING IRQ: 0x%X\n", ret);
			}
			#ifdef __DEBUG__
			printk(KERN_ALERT "DEBUG_M: --\t Disabling interruptions - finished \t--\n");
			#endif
			break;

#endif /* __FOFB__*/
	
		default:	
			up(&dev->sem);
			return -EINVAL;
		
	}	// end switch 	

	up(&dev->sem);	
	
	#ifdef __DEBUG__
	printk(KERN_ALERT "DEBUG_M: pscip_ioctl() - end\n");	
	#endif

	return 0;
	
}

/* end pscip_ioctl() */

/*****************************************************************
 *                        IRQ Handler                            *
 *****************************************************************/
/**
	pscip_irq_bh0 - called for interrupts on channel 0 of IP
	tasklets are a special function that may be scheduled to run, in software
	interrupt context, at a system-determined safe time. They may be scheduled to
	run multiple times, but tasklet scheduling is not cumulative; the tasklet runs only
	once, even if it is requested repeatedly before it is launched.
	NOTE:	
	tasklet function should return irqreturn_t but here this functions are initialized in "unconventional" way and they need to return void.....
	
	@param data 	-  data to be passed to the tasklet - anything cased to unsigned long - probably pointer
 
 */
void pscip_irq_bh0(unsigned long data)
//irqreturn_t pscip_irq_bh0(unsigned long data)
{

	pscip_Dev dev = (pscip_Dev) data;
	#ifdef __DEBUG__
	
	printk(KERN_ALERT "DEBUG_M: pscip_irq_bh0() - beginning\n");	
	
	printk(KERN_ALERT "DEBUG_M: >>>>>>>>>>>>>	T A S K L E T	0	<<<<<<<<<<<<<<<<<<<<<\n");	
	#endif
	if (!dev) 
	{
		#ifdef __DEBUG__
		printk(KERN_ALERT "/tDEBUG_M: if(!dev).....\n");	
		#endif
		return;
//		return IRQ_NONE;
	}	
	#ifdef __DEBUG__
	printk(KERN_ALERT "\nDEBUG_M:before spin locking..\n");	
	printk(KERN_ALERT "\nDEBUG_M: tasklet 0, spin_lock()\n"); 
	#endif


	spin_lock(&dev->lock[0]);


	#ifdef __DEBUG__
	printk(KERN_ALERT "\nDEBUG_M:after spin locking, waiting for sth....\n");	
	#endif


	queue_flag = 1;
	

	#ifdef __DEBUG__
	printk(KERN_ALERT "\nDEBUG_M: before wake_up...\n");	
	#endif


	wake_up_interruptible(&dev->queue[0]);


	#ifdef __DEBUG__
	printk(KERN_ALERT "\nDEBUG_M: after wake_up..., before spin unlock\n");	
	#endif


	spin_unlock(&dev->lock[0]);


	#ifdef __DEBUG__
	printk(KERN_ALERT "\nDEBUG_M: after spin unlock\n");	
	#endif

	#ifdef __DEBUG__
	printk(KERN_ALERT "DEBUG_M: pscip_irq_bh0() - end\n");	
	#endif
}
 /* end pscip_irq_bh0 */


/**
	pscip_irq_bh1 - called for interrupts on channel 1 of IP
	tasklets are a special function that may be scheduled to run, in software
	interrupt context, at a system-determined safe time. They may be scheduled to
	run multiple times, but tasklet scheduling is not cumulative; the tasklet runs only
	once, even if it is requested repeatedly before it is launched.
	NOTE:	
	tasklet function should return irqreturn_t but here this functions are initialized in "unconventional" way and they need to return void.....
	
	@param data 	-  data to be passed to the tasklet - anything cased to unsigned long - probably pointer
 
 */
void pscip_irq_bh1(unsigned long data)
//irqreturn_t pscip_irq_bh1(unsigned long data)
{

	pscip_Dev dev = (pscip_Dev) data;

	#ifdef __DEBUG__
	printk(KERN_ALERT "DEBUG_M: pscip_irq_bh1() - beginning\n");	
	
	printk(KERN_ALERT "DEBUG_M: >>>>>>>>>>>>>	T A S K L E T	1	<<<<<<<<<<<<<<<<<<<<<\n");	
	#endif	
	
	if (!dev) 
	{
		#ifdef __DEBUG__
		printk(KERN_ALERT "/tDEBUG_M: if(!dev).....\n");	
		#endif
		return;
//		return IRQ_NONE;
	}	
	#ifdef __DEBUG__
	printk(KERN_ALERT "\nDEBUG_M:before spin locking..\n");	

	printk(KERN_ALERT "\nDEBUG_M: tasklet 1, spin_lock()\n"); 
	#endif


	spin_lock(&dev->lock[1]);


	#ifdef __DEBUG__
	printk(KERN_ALERT "\nDEBUG_M:after spin locking, waiting for sth....\n");	
	#endif

	queue_flag = 1;


	#ifdef __DEBUG__
	printk(KERN_ALERT "\nDEBUG_M: before wake_up...\n");	
	#endif


	wake_up_interruptible(&dev->queue[1]);


	#ifdef __DEBUG__
	printk(KERN_ALERT "\nDEBUG_M: after wake_up..., before spin unlock\n");	
	#endif


	spin_unlock(&dev->lock[1]);
	

	#ifdef __DEBUG__
	printk(KERN_ALERT "\nDEBUG_M: after spin unlock\n");	
	#endif


	#ifdef __DEBUG__
	printk(KERN_ALERT "DEBUG_M: pscip_irq_bh1() - end\n");	
	#endif
}

/* end pscip_irq_bh1 */




/**
	Called when an interrupt occures. It recognize whether the interrupt should be handled, 
	checks errors and schedules appropraite tasklet function.
	Irq handler immediately reset pending interrupts to avoid CPU freezing
	
	@param pciirq	- interrupt number registered for this device,hardware-wise number of interrupt line
	@param *dev_id	- a sort of client data
	@param *regs	-  holds a snapshot of the processor�s context before the processor entered interrupt code
*/
 
irqreturn_t pscip_irq_handler(int pciirq, void *dev_id)
{

	pscip_Dev dev = dev_id;
	uint32_t chan,err;
	uint16_t word;
	#ifdef __DEBUG__
	printk(KERN_ALERT "DEBUG_M: pscip_irq_handler() - begining\n");	
	#endif

 	if (!(ioread32(dev->pPCIinterfaceReg + 0x4c ) & PLX9030_INTCSR_LINTI1_STAT))
    		return IRQ_NONE;
	chan = k_if_handle_irq(dev);
	if(chan < 0)		 //check whether interrupt is connected with the give IP, if yes, value greater than 0 is returned, ex, for IP 1, interrupt 0 is registererd
		return IRQ_NONE;
	#ifdef __DEBUG__
	printk(KERN_ALERT "DEBUG_M: >>>>>>>>>>>>>	I T E R R U P T	 chan %d	<<<<<<<<<<<<<<<<<<<<<\n",chan);	
	#endif	
 
// reset error flag
 	dev->stats[chan].err_flag = 0;
 
// to avoid interrupt flooding read immediately status register 
	IP_read_word(dev->pIP_IOspace[chan] + PSCIP_REG_STAT, &word);

	if (word == 0) 
		dev->stats[chan].err_flag |= PSCIP_ERROR_MSK;
	
// check errors in status register 
	if (word & PSCIP_ERROR_MSK) 
	{
	// general error 
		dev->stats[chan].err_flag |= PSCIP_ERROR_MSK;
	
	//read FPGA error counter  - pasuje
		IP_read_word(dev->pIP_IOspace[chan] + PSCIP_FPGA_HIGH, &word);
		err = (word >> 8) & 0xff;
	// check what kind of error happened; update error stats 
		if (word & PSCIP_LINKDOWN_MSK) 
		{
			dev->stats[chan].err_flag |= PSCIP_LINKDOWN_MSK;
			dev->stats[chan].linkdown_cnt += err;
		}
	// check what kind of error happened; update error stats 
		if (word & PSCIP_TXERR_MSK)
		{
			dev->stats[chan].err_flag |= PSCIP_TXERR_MSK;
			dev->stats[chan].txerr_cnt += err;
		}			
		
	// check link errors 
		IP_read_word(dev->pIP_IOspace[chan] + PSCIP_FPGA_MID, &word);

		dev->stats[chan].link_err_flag[PSCIP_WRHPRIO_IDX] = ((word>>10) & 0x1f);
		dev->stats[chan].link_err_flag[PSCIP_WRITE_IDX] = ((word>>5) & 0x1f);
		dev->stats[chan].link_err_flag[PSCIP_READ_IDX] = (word & 0x1f);
		
		IP_read_word(dev->pIP_IOspace[chan] + PSCIP_FPGA_LOW, &word);		

		dev->stats[chan].link_err_flag[PSCIP_WRWAVE_IDX] = ((word>>5) & 0x1f);
		dev->stats[chan].link_err_flag[PSCIP_RDWAVE_IDX] = (word & 0x1f);		

	// clear errors from FPGA 
		IP_write_word(dev->pIP_IOspace[chan] + PSCIP_FPGA_HIGH,PSCIP_ERRCLEAR_MSK | 0x0);
		IP_write_word(dev->pIP_IOspace[chan] + PSCIP_FPGA_LOW,0x0);							//writing low word so that the FPGA reads all register (6 bytes)
	}

	dev->irqflag[chan] = PSCIP_IRQ_SERVED;

// wake up read/write 
		tasklet_hi_schedule(&dev->pscip_tsk[chan]);	

	#ifdef __DEBUG__
	printk(KERN_ALERT "DEBUG_M: pscip_irq_handler() - end\n");	
	#endif
	return IRQ_HANDLED;
}	
/* end irqhandler */

/** 
	Calles function which registers interrupt handler
	@param dev 	- device structure
*/

 int pscip_assign_irq(pscip_Dev dev)
{
	int err;
	#ifdef __DEBUG__
	printk(KERN_ALERT "DEBUG_M: pscip_assign_irq() - beginning\n");	
	#endif
	
	err = request_irq(
                dev->irq,
                pscip_irq_handler,																				//& pscip_irq_handler ->>????
				IRQF_SHARED,																						// PCI interrupts have to be shared
                "pscip",
                dev);
	if (err)
		printk(KERN_ALERT "Cannot reguster interrupt %d\n", dev->irq);
	
	#ifdef __DEBUG__
	printk(KERN_ALERT "DEBUG_M: pscip_assign_irq() - end\n");
	#endif
	
	return err;
}
/*****************************************************************
 *                      procfs calls                             *
 *****************************************************************/
/**
	Create a proc filesystem read entry	
	
	@param *page	- page pointer, buffer to which date is writen
	@param **start	- used by the function to say where the interesting data has been written
	@param offset	- the same as count
	@param count	- 	
	@param *eof	- points to an integer that must be set by the driver to signal that it has no more data to return
	@param data	- 
 */
 static int pscip_read_proc(char *page, char **start, off_t offset, int count, int *eof, void *data)
{

	pscip_Proc procdev = data;
	pscip_Dev dev = &pscip_devices[procdev->count];

	int len = 0,
	    id_pscip = -1,																// if parameter id_pscip passed to function k_pscip_rdfpgastat() with negative value - the function reads passed by third arg device struct
		i;
	pscip_fpga_t fpga_val;	
	
		len += sprintf(page+len, "\n ************ PSCIP Driver ver.%s ********\n",ver);
	
	if(!dev->IP_in_slot)															//check whether the IP was detected in the slot
	{
		len += sprintf(page+len, " - IP %d not detected in slot on carrier %d  --\n",dev->IPnumber,dev->carrier_number);
		len += sprintf(page+len, " --- No information about IP can be provided---\n");
	}
	else																			// IP was detected in the slot - show the info
	{
		len += sprintf(page+len, " ----- IP number %d on carrier number %d  -----\n",dev->IPnumber,dev->carrier_number);
			for (i = 0; i < PSCIP_MAX_CHAN; i++) 
		{
		len += sprintf(page+len, "\n --------------------- Channel %d -------------\n",i);
			len += sprintf(page+len, " Link status: ");

			fpga_val.channel = i;
			if (k_pscip_rdfpgastat(&fpga_val, id_pscip, dev) < 0)
				goto out;
			if (fpga_val.high_data & PSCIP_LINKDOWN_MSK)
				len += sprintf(page+len, "link down\n");
			else if (fpga_val.high_data & PSCIP_TXERR_MSK)
				len += sprintf(page+len, "tx error\n");				
			else {
				len += sprintf(page+len, "ok\n");
				len += sprintf(page+len, "  Csontroller status:\n");
				if (dev->stats[i].cntrl_stat & PSCIP_INPUTBUF_MSK)
					len += sprintf(page+len, "    - Buffer full\n");
				else
					len += sprintf(page+len, "    - Buffer not full\n");			
				if (dev->stats[i].cntrl_stat & PSCIP_DSP_MSK)
					len += sprintf(page+len, "    - DSP stopped\n");
				else
					len += sprintf(page+len, "    - DSP running\n");	
				if (dev->stats[i].cntrl_stat & PSCIP_REMLOC_MSK)
					len += sprintf(page+len, "    - Local control\n");
				else
					len += sprintf(page+len, "    - Remote control\n");							
			}
				
			len += sprintf(page+len, "  Rx packets: %d\n",dev->stats[i].rx);
			len += sprintf(page+len, "  Tx packets: %d\n",dev->stats[i].tx);
			len += sprintf(page+len, "  Tx errors:  %d\n",dev->stats[i].txerr_cnt);	
			len += sprintf(page+len, "  Link errors: %d\n",dev->stats[i].linkdown_cnt);
		
		}	
	}
	len += sprintf(page+len, "\n");
	*eof = 1;	
	return len;

	//IP error 
	out:
		printk(KERN_ERR "PSCIP error\n");	
		len+=sprintf(page+len, " PSCIP error\n\n");
		*eof = 1;	
		up(&dev->sem);

	return len;	

	return 0;		//added for 2.6 - temporary to avoid warnings
}


/* end pscip_read_proc() */


/**
	creat a proc file system write entry
	@param filep	- file structure pointer
	@param *buffer	- data from the user
	@param count	- 
	@param *data	- 
 
 */
 static int pscip_write_proc(struct file *filp, const char *buffer, unsigned long count, void *data)
{
	pscip_Proc procdev = data;	
	pscip_Dev dev = &pscip_devices[procdev->count];
	uint32_t wval1 = 0,wval2 = 0, wval3 = 0, wval4=0;
	pscip_t val;
	unsigned int code;
	int id_pscip = -1;
	int ret = 0;	
	#ifdef __DEBUG__
	printk(KERN_ALERT "DEBUG_M: pscip_write_proc - beginning\n");
	#endif		
	
	sscanf(buffer, "%d %x %x %x %x\n", &code /*command number*/ \
									, &wval1 /*channel*/ \
									, &wval2 /*status*/ \
									, &wval3 /*address*/ \
									, &wval4 /*data*/);	
					
	
// lock ioctl to avoid race conditions 
	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;
	
// check channel value; 0 or 1 are allowed 
	if ((wval1 > 1) && (code <= PSCIP_PROC_WRWAVE))										//if we want to write, the channel must be 0 or 1
		goto out;
	
	if ((code > PSCIP_PROC_WRWAVE) && (code != PSCIP_PROC_RESET))						//if we want to reset, entire IP is reseted - does not matter which channel
		goto out;
		
	switch (code) 
	{
	// high priority write	
		case PSCIP_PROC_HPRIO:	
			val.chan = wval1;
			val.stat = wval2;		
			val.address = wval3;
			val.data = wval4;   
			if((ret = k_pscip_wrhiprio(&val, id_pscip, dev))<0) 
				goto out;			
			break;
	//write
		case PSCIP_PROC_WRITE:	
			val.chan = wval1;
			val.stat = wval2;		
			val.address = wval3;
			val.data = wval4;    
			if((ret = k_pscip_write(&val, id_pscip, dev))<0) 
				goto out;			
			break;		
	//write wave
		case PSCIP_PROC_WRWAVE:	
			val.chan = wval1;
			val.stat = wval2;		
			val.address = wval3;
			val.data = wval4;    
			if((ret = k_pscip_wrwave(&val, id_pscip, dev))<0) 
				goto out;			
			break;
	//reset IP	
		case PSCIP_PROC_RESET:	

			if((ret = k_pscip_reset(id_pscip, dev))<0) 
				goto out;			
			break;
		
		default: 	
			return -EINVAL;
	}
	
	up(&dev->sem);		
	#ifdef __DEBUG__
	printk(KERN_ALERT "DEBUG_M: pscip_write_proc - en\n");
	#endif	
	
	return count;

	out:
		printk(KERN_ERR "PSCIP error\n");
		up(&dev->sem);
		return count;

	
}

/* end pscip_write_proc() */


/**
	Register proc filesystem entries. It creates the appropriate directory in /proc/driver/ and fills it with nodes representing each IP
	It registers functions implemented for proc_fs interface
 */

 static int pscip_procfs_register(void)
{

	struct proc_dir_entry *procfs_file;
	char root_dir[30]="driver", model_dir[30], dev_dir[10];
	int i,k;
	#ifdef __DEBUG__
	printk(KERN_ALERT "DEBUG_M: pscip_procfs_register - beginning\n");
	#endif	

// register proc entry under "driver" entry 
	sprintf(model_dir,"%s/%s",root_dir,module_name);
	proc_model_dir = proc_mkdir(model_dir, NULL);
	if (proc_model_dir == NULL)
		goto err_proc_mkdir;
	
// allocate proc device data structure 
	pscip_procdev = kmalloc(pscip_dev_number*sizeof(npscip_Proc), GFP_KERNEL);
	if (!pscip_procdev) 
		goto err_kmalloc;
		
	memset(pscip_procdev, 0, pscip_dev_number*sizeof(npscip_Proc));
	// 	example of proc filesystem entry
	//	/proc/driver/pscip/ pscip0
	//	/proc/driver/pscip/pscip1
	//	.....
	//	/proc/driver/pscip/pscippN
							
	for (i = 0; i < pscip_dev_number; i++) 
	{
		sprintf(dev_dir,"pscip%d",i);
		procfs_file = create_proc_entry(dev_dir, S_IRUGO|S_IWUSR, proc_model_dir);
		if (procfs_file == NULL) 
		{
//release also proc_fs successfull registered entries			
			for (k = 0; k < i; k++) 
			{
			 	sprintf(dev_dir,"pscip%d",i);	
				remove_proc_entry(dev_dir, proc_model_dir);
			}
			goto err_create_proc_entry;

		}
		pscip_procdev[i].count = i;																			// i=device (minor) number 
		
// initialize proc filesystem file entries 
		procfs_file->data = &pscip_procdev[i];
		procfs_file->read_proc = pscip_read_proc;
		procfs_file->write_proc = pscip_write_proc;
	}
	#ifdef __DEBUG__
	printk(KERN_ALERT "DEBUG_M: pscip_procfs_register - end\n");
	#endif	
	return 0;
      
	err_create_proc_entry:
		kfree(pscip_procdev);
	err_kmalloc:
		remove_proc_entry(model_dir, NULL);
	err_proc_mkdir:
   
	#ifdef __DEBUG__
	printk(KERN_ALERT "DEBUG_M: pscip_procfs_register - end with error\n");
	#endif	
   return -ENOMEM;		
   
}

/* end pscip_procfs_register() */


/*************************************************************************************/

/**
	Initializatin of interrupt and concurrency related mechanisms.
	The functions registers tasklests for each channel on the calling IP, registers spinlocks and waitqueues.
	
	@param i	- number of device ( IP ) in the device structure
	
 */

static void init_pscip(int i)
{	
	#ifdef __DEBUG__
	printk(KERN_ALERT "DEBUG_M: init_pscip() - beginning\n");	
	#endif

// initialize tasklet; channel 0/1
	tasklet_init(&pscip_devices[i].pscip_tsk[0],pscip_irq_bh0, (unsigned long)&pscip_devices[i]); 					
	tasklet_init(&pscip_devices[i].pscip_tsk[1],pscip_irq_bh1, (unsigned long)&pscip_devices[i]); 

// initialize locks; channel 0/1 	
	spin_lock_init(&pscip_devices[i].lock[0]);
	spin_lock_init(&pscip_devices[i].lock[1]);	


// initialize queues; channel 0/1 
	init_waitqueue_head(&pscip_devices[i].queue[0]);
	init_waitqueue_head(&pscip_devices[i].queue[1]);	
	
	#ifdef __DEBUG__
	printk(KERN_ALERT "DEBUG_M: spin waitqueue initialized 1\n");	
	#endif
	
// reset timestamp registers; channel 0/1 
	do_gettimeofday(&pscip_devices[i].tv[0]);pscip_devices[i].tv[0].tv_sec -= PSCIP_LINKDOWN_TOUT+1;
	do_gettimeofday(&pscip_devices[i].tv[1]);pscip_devices[i].tv[1].tv_sec -= PSCIP_LINKDOWN_TOUT+1;

	#ifdef __DEBUG__
	printk(KERN_ALERT "DEBUG_M: init_pscip() - end\n");	
	#endif

}

/* end init_pscip */


/**
	Checks whether device is in the slot by reading specific registers holdilng IP IDs. 
 */
static int checkmem_device(int i)
{
		uint8_t model_id = 0;
		uint8_t manufacture_id = 0;
		
		IP_read_byte(pscip_devices[i].pIP_IOspace[0] + PSCIP_MODEL_REG, &model_id);
		IP_read_byte(pscip_devices[i].pIP_IOspace[0] + PSCIP_MANIFAC_REG ,&manufacture_id);
		if ((model_id != PSCIP_MODEL_ID) || (manufacture_id != PSCIP_MANIFAC_ID)) 
		{	
			printk(KERN_ERR "PSCIP PS controller model error; model id: 0x%x manufacturer id 0x%x\n If the values are 0xFF, it is most probable there is no IP in the SLOT on the carrier. \n Check slot number: %d\n",model_id,manufacture_id,pscip_devices[i].IPnumber);					 
			return -EFAULT ;	
		}	


	return 0;

}
/* end checkmem_device() */

/** 
	It performs most of the initialization for both IPs on the carrier and registers the character device in the kernel
	
	@param *dev	- PCI device structure representing PCI device
	@param *id		- 

*/


static int probe(struct pci_dev *dev, const struct pci_device_id *id)
{

	int index;																				//count the number of already registered IPs, indicates the first IP on the carrier
	int err;																				//error
	int i = 0 ; 																			//IP on the carrier
	int devno; 																				//
	int carrier_number;																		//carrier number
	#ifdef __DEBUG__
	printk(KERN_ALERT "DEBUG_M: probe() - beginning\n");
	pscip_get_PCI_info(dev)	;
	#endif	

	if((dev == NULL) || (id == NULL)) 														//not good but I don't know what it means
	{
		printk(KERN_ALERT "probe(): (dev == NULL) || (id == NULL)\n");	
		return -1;
	}
	
	for(index = 0; (index < pscip_dev_number) && (pscip_devices[index].IPnumber >= 0); ++index);//finding out the first free device number
	if(index == pscip_dev_number - 1)														//if there are no 2 free device numbers - no good, we need two device numbers for one carrier
	{
		printk(KERN_ERR "\n%s: No free place in driver_struct, the number of PCI carriers\
		pluged in this computer extended the number you declared when installing the driver\
		(%d). You need to reinsert the driver with an appropriate \
		number.\n\n", "PSCIP", pscip_dev_number);
		err = index;
		goto err_pscip_dev_number;
	}
	else
	{
		carrier_number = index % MAX_NUMER_OF_IPs_ON_CARRIER;
		num_pscip = index + 1;
		printk(KERN_INFO "\n%s: The IPs on the Carrier nr %d will be represented by:	\n \
		Major number: %d, minor numbers: %d and %d. \n \
		In the /proc/driver IPs will be represented bb: \n \
		/dev/pscip%d  and /dev/pscip%d nodes \n\n",module_name, carrier_number, pscip_major, pscip_minor + index, \
		pscip_minor + index + 1, pscip_minor + index, pscip_minor + index + 1);
	}
	

	

//initializing cdev structure embedded in device structure
	for( i = 0; i < 2; i++)
	{
		cdev_init(&pscip_devices[index + i].cdev, &pscip_fops);									
		pscip_devices[index + i].cdev.owner = THIS_MODULE;
		pscip_devices[index + i].cdev.ops = &pscip_fops;		

// initialize queues 		
		init_waitqueue_head(&pscip_devices[index + i].queue[0]);
		init_waitqueue_head(&pscip_devices[index + i].queue[1]);				 		
		
// initialize semaphores 
		sema_init(&pscip_devices[index + i].sem,1);					

		devno = MKDEV(pscip_major, pscip_minor + index + i);

// AS SOON AS cdev_add  RETURNS, THE DEVICE IS "LIVE" AND ITS OPERATION S CAN BE CALLED BY THE KERNEL
		err = cdev_add (&(pscip_devices[index + i].cdev), devno, 1);
		if (err)
		{
			printk(KERN_NOTICE "Error %d adding pscip%d", err, index + i);
			goto err_cdev_add;
		}
		sprintf(pscip_devices[index + i].device_name, "%s%d", "pscip", index + i);
		pscip_devices[index + i].carrier_number			=		carrier_number;				//can be helpfull to physically recognise which carrier it is
		pscip_devices[index + i].IPnumber				=		i;							// A BIG PROBLEM : there are two IPs
		pscip_devices[index + i].pscip_number			=		index + i;					//number of all registered IPs, starting with 0

	}	

//initialization of pci_dev structure embeded in device structure - it is embedded in the structure of the first IP on the Carrier only !!!!!!
		pscip_devices[index].pcidev = dev;
		pci_set_drvdata(dev, &(pscip_devices[index]));

		
// enable PCI device  accessing it 
		err = pci_enable_device(dev);
		if(err)
		{
			printk(KERN_NOTICE "Could not enable PCI device");
			goto err_pci_enable_device;	
		}
		
		err = pci_request_regions(dev,"pscip");
		if(err)
		{	
			printk(KERN_NOTICE "Negative answer to requesting I/O region for PCI");
			goto err_pci_request_region;	
		}

// initialization of the PCI carrier parameters - accessing the device	- getting the location (physica) of registers
	for( i = 0; i < 2; i++)
	{
		pscip_devices[index + i].pciaddr				=		pci_resource_start(dev, 0);		//a bit stupid - did to keep the code similar to vme code but make no sense to have this address so it will have to be changed later
	// PCI Configuration register
		pscip_devices[index + i].PCIinterfaceAddr		=		pci_resource_start(dev, 0);
		pscip_devices[index + i].PCIinterfaceLen		=		pci_resource_len(dev, 0);
		pscip_devices[index + i].PCIinterfaceFlags		=		pci_resource_flags(dev, 0);
	//IP Interface register
		pscip_devices[index + i].IPinterfaceAddr		=		pci_resource_start(dev, 2);
		pscip_devices[index + i].IPinterfaceLen			=		pci_resource_len(dev, 2);
		pscip_devices[index + i].IPinterfaceFlags		=		pci_resource_flags(dev, 2);
	//I/O space memory region - here we communicate with IP,
		pscip_devices[index + i].IP_IOspaceAddr			=		pci_resource_start(dev, 3);
		pscip_devices[index + i].IP_IOspaceLen			=		pci_resource_len(dev, 3);
		pscip_devices[index + i].IP_IOspaceFlags		=		pci_resource_flags(dev, 3);
		//interrupts
		pscip_devices[index + i].irq					=		dev->irq;							//I am not sure about that
		
//mapping PCI regions to kernel addressing -> getting the virtual addresses which will be used to accessing the registers
		pscip_devices[index + i].pPCIinterfaceReg		=		pci_iomap(dev, 0, pscip_devices[index + i].PCIinterfaceLen);	
		pscip_devices[index + i].pIPinterfaceReg		=		pci_iomap(dev, 2, pscip_devices[index + i].IPinterfaceLen);

		if(i == 1)		
			pscip_devices[index + i].pIP_IOspace[0]		=		pci_iomap(dev, 3, pscip_devices[index + i].IP_IOspaceLen) + PSCIP_IP_OFFSET;		//virtual address of channel 0 on IP 1
		else
			pscip_devices[index + i].pIP_IOspace[0]		=		pci_iomap(dev, 3, pscip_devices[index + i].IP_IOspaceLen);							//virtual address of channel 0 on IP 0

		pscip_devices[index + i].pIP_IOspace[1]			=		pscip_devices[index + i].pIP_IOspace[0]	+	PSCIP_CHAN_OFFSET;						//virtual address of channel 1 on IP 0 or 1

//check whether the IP is in the slot
		if(! checkmem_device(index + i))
		{
			pscip_devices[index + i].IP_in_slot			=		TRUE;
			printk(KERN_INFO "\nIP was detected in the slot %d on the Carrier number %d\n\n",pscip_devices[index + i].IPnumber, pscip_devices[index + i].carrier_number);	
		}
		else
		{
			pscip_devices[index + i].IP_in_slot			=		FALSE;
			printk(KERN_ALERT "\nOn Carrier %d IP %d was not detected in the slot\n\n",pscip_devices[index + i].carrier_number,pscip_devices[index + i].IPnumber);	
		}
				
		if (! pscip_devices[index + i].pPCIinterfaceReg ) 
		{
			printk(KERN_NOTICE "\nCould not allocate virtual address\n\n");
		    goto err_virtaul_alloc;
		}
		if (! pscip_devices[index + i].pIPinterfaceReg ) 
		{
			printk(KERN_NOTICE "\nCould not allocate virtual address\n\n");
		    goto err_virtaul_alloc;
		}

		if (! pscip_devices[index + i].pIP_IOspace[0] ) 
		{	
			printk(KERN_NOTICE "\nCould not allocate virtual address\n\n");
		    goto err_virtaul_alloc;
		}

		
		init_pscip(index + i);
		
		
	}
	
	#ifdef __DEBUG__
	printk(KERN_ALERT "DEBUG_M: probe() - end\n");
	#endif	


   return 0;
	
	err_virtaul_alloc:
		pci_release_regions(dev);
	err_pci_request_region:
		 pci_disable_device(dev);
	err_pci_enable_device:
		cdev_del(&pscip_devices[index].cdev);
		cdev_del(&pscip_devices[index + 1].cdev);
	err_cdev_add:
	err_pscip_dev_number:
	
	#ifdef __DEBUG__
	printk(KERN_ALERT "DEBUG_M: probe() - end ended with error\n");
	#endif	
	return err;

}
	
/**
	Does all the unregister/remove/free stuff in reverse order than probe
	
	@param *pcidev	- structure representing PCI device
*/
static void remove(struct pci_dev *pcidev)
{

	pscip_Dev dev = pci_get_drvdata(pcidev);											//retreaveing device structure from pci_dev structure - only dev structure of the first IP on the carrier can be obtained in such a way
	int index = dev->pscip_number;														//finding out the IP number of the first IP on the Carrier
 	pci_iounmap(pcidev, dev->pPCIinterfaceReg);											//unmapping virtual address
 	pci_iounmap(pcidev, dev->pIPinterfaceReg);											//unmapping virtual address
	pci_iounmap(pcidev, dev->pIP_IOspace[0]);											//unmapping virtual address
 	pci_release_regions(pcidev);														//releasing region
 	pci_disable_device(pcidev);															//disabling PCI carrier
	cdev_del(&pscip_devices[index].cdev);												//char device deletion of first IP on the carrier
	cdev_del(&pscip_devices[index + 1].cdev);											//char device deletion of second IP on the carrier

	#ifdef __DEBUG__
	printk(KERN_ALERT "DEBUG_M: remove()- end\n");	
	#endif	

}



/**	Function called when the module is deinserted. 
	Clean up module from kernel - all  unregister staff
 */
static void exit_pscip_module(void)
{ 
	dev_t chrdev = 0;																	// dev_t structure needed to supply unregister_chrdev_region with parameter
	int i;																				// counting IPs
	char root_dir[30]="driver", model_dir[30], dev_dir[10];								// used for proc fs 
	
	#ifdef __DEBUG__
	printk(KERN_ALERT "exit_pscip_module() - beginning\n");	
	#endif	
	
	chrdev = MKDEV(pscip_major, pscip_minor);											//populate dev_t
	
//	kfree(pscip_devices);																//free kallocated memory
	unregister_chrdev_region(chrdev,pscip_dev_number);									//use chrdev to free chrdev region
	pci_unregister_driver(&pscip_driver);												// give holidays for poor PCI

	for (i = 0; i < pscip_dev_number; i++) 
	{																				
		sprintf(dev_dir,"pscip%d",i);
		remove_proc_entry(dev_dir, proc_model_dir);										//removing proc entrie
	} 
	sprintf(model_dir,"%s/%s",root_dir,module_name);
	remove_proc_entry(model_dir,NULL);													//goodbye to proc fs

	#ifdef __DEBUG__
	printk(KERN_ALERT "exit_pscip_module() - end\n");	
	#endif	
}
/* end cleanup_module */
	


/**	Function called when the driver/module is inserted. 
	It establishes how many devices ( IPs )  the driver will enable to operate. It registers major and minor numbers for the driver,
	registers driver, prepare memory for device structure, initializes device structure table with 0/null values and create proc_fs entries in
	/proc/driver folder.
	
 */
static int init_pscip_module(void)
{ 
	
    int i = 0;																			//counting device
    int j = 0;																			//counting channel
	dev_t chrdev = 0;	
	int err = 0;																		//error value
	
	printk(KERN_INFO "\n************ PSCI driver ***********\n");
	printk(KERN_INFO "* for kernel 2.6 and PCI interface *\n");
	printk(KERN_INFO "*     very development version     *\n");
	printk(KERN_INFO "************************************\n\n");
	
	#ifdef __DEBUG__
	printk(KERN_ALERT "DEBUG_M: init_pscip_module() - beginning\n");	
	#endif	
//determining number of IPs
	if(pscip_dev_number==0)																//if the number of IPs is not provided when inserting the module, a DEFAULT number is set
		pscip_dev_number = PSCIP_DEFAULT_DEV;
	if(pscip_dev_number >= PSCIP_MAX_DEV)												// the number of devices cannot be greater than MAX_DEV
	{
		printk("ERROR: numer of devices passed as a parameter \
		pscip_dev_number = %d is greater than maximum allowed \
		number: %d \n",pscip_dev_number,PSCIP_MAX_DEV);
		return -EPERM;																	//operation not permitted
	}
//char device registration
	if (pscip_major) 																	// if the major number is determine by the user (passing by parameter) than we use this method to register character device
	{
		chrdev = MKDEV(pscip_major, pscip_minor);										//putting major and minor number into dev_t structure - > not allowed to do it by hand, need to use macro
		err = register_chrdev_region(chrdev, pscip_dev_number, "pscip");				//registering region to the device numbers determined by the user
	} 
	else																				//dynamic allocation of major number if it is not defined by user - preffered way
	{
		err = alloc_chrdev_region(&chrdev, pscip_minor, pscip_dev_number, "pscip");		//dynamically alllocating device numbers (major and minor)
		pscip_major = MAJOR(chrdev);													// finding out what was allocated
	}
	if (err < 0) 
	{
		printk("ERROR: Cannot register device number\n");
		goto err_chrdev_region;
	}

//memory allocation
/*
	pscip_devices = kmalloc(pscip_dev_number*sizeof(npscip_Dev), GFP_KERNEL);			//because the number od devices can be chosen dynamically, we allocate memory for the table of device structures dynamically, kmalloc is needed
	if (!pscip_devices) 
	{
		printk("ERROR: Cannot register device number\n");
		goto err_kmalloc;
	}
*/
	memset(pscip_devices, 0, sizeof(pscip_Dev)*pscip_dev_number);						//allocate and clear memory for all devices

//parameters initialization for all devices
	for(i = 0; i < pscip_dev_number; i++)
	{
		pscip_devices[i].pciaddr = 0;  													// pci (physical) address 
		for(j = 0; j < PSCIP_MAX_CHAN; j++)		
		{
			pscip_devices[i].irqflag[j] = 0;											
			pscip_devices[i].irqdata[j] = 0;
		}
		pscip_devices[i].PCIinterfaceAddr = 0;											//address (physical) of PCI interface registers
		pscip_devices[i].PCIinterfaceLen = 0;											// length of PCI interface register
		pscip_devices[i].PCIinterfaceFlags = 0;
		pscip_devices[i].IPinterfaceAddr = 0;  											//address  (physical)  of IP interface register - local adress 2
		pscip_devices[i].IPinterfaceLen = 0;											//length  (physical)  of IP interface register	
		pscip_devices[i].IPinterfaceFlags = 0;
		pscip_devices[i].IP_IOspaceAddr = 0;											//address  (physical)  of the beginnning of IP I/O space - local addrress 3
		pscip_devices[i].IP_IOspaceLen = 0;												//length of IP I/O space 
		pscip_devices[i].IP_IOspaceFlags = 0;
		
		pscip_devices[i].pPCIinterfaceReg = NULL;										//virtual address of PIC interface regs
		pscip_devices[i].pIPinterfaceReg = NULL;										//Local space 0  (virtual address)

		for(j = 0; j < PSCIP_MAX_CHAN; j++)		
			pscip_devices[i].pIP_IOspace[j] = NULL;										//virtual address: chan=0 for link 1 and chan=1 for link 2
		
		pscip_devices[i].IPnumber = -1;													//there are max two IPs on one carrier: 0 or 1 - used to knwo which one to reset
		pscip_devices[i].carrier_number = -1 ;											//number of the carrier starting from the carrier with the lowest PCI address (hopefully)
		pscip_devices[i].refcount = 0;													//keeps the number of opened devices
		pscip_devices[i].irq = 0;														// interrupt line number
		pscip_devices[i].IP_in_slot = FALSE;
	}	

//proc file system init
	err = pscip_procfs_register();														//create representation of PSCIP in /proc/driver
	if (err < 0) 
	{
		printk(KERN_ERR "can't register proc filesystem\n");
		goto err_pscip_procfs_register;
	}	
	
	if (pci_register_driver(&pscip_driver) != 0) 										// register pci driver as soon as registered the probe is called for each carrier
	{
		printk("Failed to register 'pci_driver' \n");
		goto err_pci_register;
	}
	
//info	
	printk(KERN_INFO "\n The module %s has been successfully inserted\n", "pscip");	
	printk(KERN_INFO "The module is represented by Major Nuber: %d and minor numbers:\
	%d to %d\n\n", pscip_major, pscip_minor, (pscip_minor + pscip_dev_number - 1));	

	
	#ifdef __DEBUG__
	printk(KERN_ALERT "DEBUG_M: init_pscip_module() - end, the device has been registered with device major number: %d\n",pscip_major);	
	#endif	


	return 0;
	
//error	
	err_pscip_procfs_register:
		kfree(pscip_devices);
//	err_kmalloc:
		unregister_chrdev_region(chrdev,pscip_dev_number);
	err_chrdev_region:
		pci_unregister_driver(&pscip_driver);
	err_pci_register:			
	return -ENOMEM;
} 
/* end init_module */

/** Registering first function to be called when inserting the module. */
module_init(init_pscip_module);
/** Registering last function to be called when de-inserting the module. */
module_exit(exit_pscip_module);
