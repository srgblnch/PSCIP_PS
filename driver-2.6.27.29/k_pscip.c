/** \file k_pscip.c 
	Contains function directly interfacing hardware/kernel. 
	The  file can be viewed like a lower layer of driver interface
 *	PSCIP Driver for Linux Kernel 
 *	Functions to access hardware
 *
 * 	GG - 18.07.2005: First implementation 
 *  	GG - 08.02.2006: When setting status writew in the same stat and addr register
 *
 * 	22/10/2008 - Start of porting the driver :
 *	-	from 2.4.x to 2.6.x
 *	- 	from VME to PCIGG 
 * 
 */

static int num_pscip = 0; 								 /**< Number of registered devices.  */
//static pscip_Dev pscip_devices;  /* private data struct */
static npscip_Dev pscip_devices[PSCIP_MAX_DEV];			/**< Static table of structures holding device datea. */

/** The structure is used to define a list of the different types of PCI devices that a driver supports.  */
static struct pci_device_id ids[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_TEWS, PCI_DEVICE_ID_TCPI100_PCI), },
	{ PCI_DEVICE(PCI_VENDOR_ID_TEWS, PCI_DEVICE_ID_TCP221_cPCI), },
	{ 0, }
};

/** Registering the  table defining a list of the different types of CI devices that the driver supports. */
MODULE_DEVICE_TABLE(pci, ids);											


/** Reads byte from the device. 
	@param *address	- virtual address of the register to be read
	@param *val	- structure containing information about the channel/link we are using
*/
void IP_read_byte(void *address,  uint8_t *val)
{
	*val = ioread8(address);			
	rmb();
	#ifdef __DEBUG__
	printk(KERN_ALERT "\t\t DEBUG_K: readb(): 0x%x\n", *val);
	#endif
}
/** Reads word from the device. 
	@param *address		- virtual address of the register to be read
	@param *val	- structure containing information about the channel/link we are using
*/
void IP_read_word(void *address, uint16_t *val)
{
	*val = ioread16(address);
	rmb();
	#ifdef __DEBUG__
	printk(KERN_ALERT "\t\t DEBUG_K: readw(): 0x%x\n", *val);
	#endif
}

/** Writing byte to the device.
	@param *address		- virtual address of the register to be writen to 
	@param *val	- structure containing information about the channel/link we are using
 */
void IP_write_byte(void *address,  uint8_t val)
{
	iowrite8(val,address);
	wmb();
	#ifdef __DEBUG__
	printk(KERN_ALERT "\t\t DEBUG_K: writeb(): 0x%x\n",val);
	#endif
}

/** Writing word to the device. 
	@param *address		- virtual address of the register to be writen to 
	@param *val	- structure containing information about the channel/link we are using
*/
void IP_write_word(void *address,  uint16_t val)
{
	iowrite16(val,address);
	wmb();
	#ifdef __DEBUG__
	printk(KERN_ALERT "\t\t DEBUG_K: writew(): 0x%x\n",val);
	#endif
}


/** 	Wait after linkdown.
	Wait some seconds after a linkdown event, maybe to avoid requesting data
	after a local control unit reboot
	
	@param *val	- structure containing information about the channel/link we are using
	@param id_pscip	- number of the device ( IP ) we are using, if negative value, the device passed by third argument is used
	@param dev		- device structure contaning informatin about the device (IP)
	
 */
int k_pscip_wait_linkdown(pscip_t *val, int id_pscip, pscip_Dev dev)
{ 

	struct timeval cur_tv;
	long tv_sec;
	uint16_t word;
	#ifdef __DEBUG__
	printk(KERN_ALERT "\t\t DEBUG_K (chan %d): k_pscip_wait_linkdown() - beginning \n",val->chan);
	#endif	

	if ((id_pscip >= 0) && (id_pscip < num_pscip))
		dev = &pscip_devices[id_pscip];	 
	
	#ifdef __DEBUG__
	printk(KERN_ALERT "\t\t DEBUG_K: k_pscip_wait_linkdown() - 1\n");
	#endif	

// IMPORTANT
	// we read here two timse for the following reason - when reading for the first time after reset the high word indicates "link down" which is not true - i don't know about
	// side effects of reading two times - I don't know yet
// IMPORTANT
	#ifdef __DEBUG__
	printk(KERN_ALERT "\t\t DEBUG_K : k_pscip_wait_linkdown() - 2\n");
	#endif	
	

	IP_read_word(dev->pIP_IOspace[val->chan]+PSCIP_FPGA_HIGH, &word);
	
	#ifdef __DEBUG__
	printk(KERN_ALERT "\t\t DEBUG_K : k_pscip_wait_linkdown() - 3\n");
	#endif

	IP_read_word(dev->pIP_IOspace[val->chan]+PSCIP_FPGA_HIGH, &word);

	#ifdef __DEBUG__
	printk(KERN_ALERT "\t\t DEBUG_K: k_pscip_wait_linkdown() - 4\n");
	#endif	

	if (word & PSCIP_ERRCNT_MSK) 	
	{
		IP_write_word(dev->pIP_IOspace[val->chan]+PSCIP_FPGA_HIGH, PSCIP_ERRCLEAR_MSK | 0x0000);
		IP_write_word(dev->pIP_IOspace[val->chan]+PSCIP_FPGA_LOW, 0x0000);							//added by ML, FPGA reacts to writing to the low word, without this line, the counter is not cleared
	}

	#ifdef __DEBUG__
	printk(KERN_ALERT "\t\t DEBUG_K: k_pscip_wait_linkdown() - 5\n");
	#endif	

			
	if (word & PSCIP_LINKDOWN_MSK)
	{
		do_gettimeofday(&dev->tv[val->chan]);
		dev->stats[val->chan].err_flag |= PSCIP_LINKDOWN_MSK;
		dev->stats[val->chan].linkdown_cnt++;
		#ifdef __DEBUG__
		printk(KERN_ALERT "\t\t DEBUG_K: ERROR: PSCIP_LINK_ERR -> PSCIP_LINKDOWN_MSK\n");
		#endif	
		return -PSCIP_LINK_ERR;

	}
	else 
	{
		do_gettimeofday(&cur_tv);
		tv_sec = (cur_tv.tv_sec-dev->tv[val->chan].tv_sec);	
		if (tv_sec < PSCIP_LINKDOWN_TOUT) 
		{
			dev->stats[val->chan].err_flag |= PSCIP_LINKDOWN_MSK;
			dev->stats[val->chan].linkdown_cnt++;
			#ifdef __DEBUG__
			printk(KERN_ALERT "\t\t DEBUG_K: ERROR: PSCIP_LINK_ERR -> PSCIP_LINKDOWN_TOUT\n");
			#endif	
			return -PSCIP_LINK_ERR;

		}	
	}	
	
	#ifdef __DEBUG__
	printk(KERN_ALERT "\t\t DEBUG_K (chan %d): k_pscip_wait_linkdown() - end \n",val->chan);	
	#endif	
	
	return 0;

	
} 
/* end k_pscip_wait_linkdown() */


/**	Execute high priority PS-current setting.
	
	@param *val	- structure containing information about the channel/link we are using
	@param id_pscip	- number of the device ( IP ) we are using, if negative value, the device passed by third argument is used
	@param dev		- device structure contaning informatin about the device (IP)
	
 */
int k_pscip_wrhiprio(pscip_t *val, int id_pscip, pscip_Dev dev) 
{
	
	uint8_t 		byte;
	uint16_t 		word;
	uint32_t 		tmp_data = 0;
	uint32_t 		cnt=0;
	int32_t 		ret = 0;


	#ifdef __DEBUG__
	printk(KERN_ALERT "\t\t DEBUG_K (chan %d): k_pscip_wrhiprio() - beginning \n", val->chan);	
	#endif		

	
	if ((id_pscip >= 0) && (id_pscip < num_pscip))
		dev = &pscip_devices[id_pscip];	 


	if ((ret = k_pscip_wait_linkdown(val, -1, dev)) < 0)
	{
		#ifdef __DEBUG__
		printk(KERN_ALERT "\t\t DEBUG_K: ERROR: linkdown \n");	
		#endif	
		goto error;
	}
	dev->irqflag[val->chan] = PSCIP_IRQ_REQUEST;
	word = (val->stat << 8) | val->address;	
	IP_write_word(dev->pIP_IOspace[val->chan]+PSCIP_HPRIOWRITE_REG,word);	
	
	word = (val->data >> 16) & 0xffff;	
	IP_write_word(dev->pIP_IOspace[val->chan]+PSCIP_HPRIOWRITE_DATA, cpu_to_be16(word));
	word = val->data & 0xffff;	
	IP_write_word(dev->pIP_IOspace[val->chan]+PSCIP_HPRIOWRITE_DATA + 0x2, cpu_to_be16(word));

//		spin_lock_irqsave(&dev->lock[val->chan],flags); 
		wait_event_interruptible_timeout(dev->queue[val->chan],queue_flag != 0, 1);
		queue_flag = 0;
//		spin_unlock_irqrestore(&dev->lock[val->chan],flags);
	
	while (dev->irqflag[val->chan] != PSCIP_IRQ_SERVED) 
	{
		udelay(1);
		if ((cnt++) > PSCIP_BUSY_TOUT) 
		{

			ret = (-PSCIP_TOUT_ERR);
			#ifdef __DEBUG__
			printk(KERN_ALERT "\t\t DEBUG_K: ERROR: PSCIP_BUSY_TOUT\n");	
			#endif
			goto error;
		}
	}
	
	if (dev->stats[val->chan].err_flag & PSCIP_ERROR_MSK) 
	{			

		#ifdef __DEBUG__
			ret = (-PSCIP_TX_ERR);
			if (dev->stats[val->chan].err_flag & PSCIP_LINKDOWN_MSK)
			{
				ret = (- PSCIP_LINK_ERR);
				printk(KERN_ALERT "\t\t DEBUG_K: ERROR: PSCIP_ERROR_MSK -> PSCIP_LINK_ERR\n");
			}
			else
				printk(KERN_ALERT "\t\t DEBUG_K: ERROR: PSCIP_ERROR_MSK -> PSCIP_TX_ERR\n");	
		#else
			ret = (-PSCIP_TX_ERR);
			if (dev->stats[val->chan].err_flag & PSCIP_LINKDOWN_MSK)
				ret = (- PSCIP_LINK_ERR);
		#endif
		goto error;
	}		

	if (val->stat & PSCIP_ECHO_MSK) 
	{
		IP_read_word(dev->pIP_IOspace[val->chan]+PSCIP_HPRIOWRITE_DATA, &word);

		tmp_data = (be16_to_cpu(word) & 0xffff) << 16;
		IP_read_word(dev->pIP_IOspace[val->chan]+PSCIP_HPRIOWRITE_DATA+0x2, &word);
		tmp_data += (be16_to_cpu(word)& 0xffff);
		if (tmp_data != val->data) 
		{
			ret = (-PSCIP_MISMATCH_ERR);
			#ifdef __DEBUG__
			printk(KERN_ALERT "\t\t DEBUG_K: ERROR: PSCIP_ERROR_MSK\n");	
			#endif
			goto error;
		}
	}	

// update controller status
	IP_read_byte(dev->pIP_IOspace[val->chan]+PSCIP_HPRIOWRITE_STAT, &byte);	//ok
	dev->stats[val->chan].cntrl_stat = byte & (PSCIP_INPUTBUF_MSK |
		PSCIP_DSP_MSK | PSCIP_REMLOC_MSK | PSCIP_LINK_MSK);				
		
	dev->stats[val->chan].tx++;	
	
	dev->irqflag[val->chan] = PSCIP_IRQ_DONE;
	
	#ifdef __DEBUG__
	printk(KERN_ALERT "\t\t DEBUG_K (chan %d): k_pscip_wrhiprio() - end \n",val->chan);	
	#endif	
	return ret;

	error:
		dev->irqflag[val->chan] = PSCIP_IRQ_DONE;

		#ifdef __DEBUG__
		printk(KERN_ALERT "\t\t DEBUG_K: k_pscip_wrhiprio() - error \n");	
		#endif	
	return ret;

	
} 
/* end k_pscip_wrhiprio() */
																	

/**	Execute writing action.
	The function writes given data (2 words) to given address in the PS, the information is passed with a given status.
	
 	@param *val	- structure containing information about the channel/link we are using
	@param id_pscip	- number of the device ( IP ) we are using, if negative value, the device passed by third argument is used
	@param dev		- device structure contaning informatin about the device (IP)
 */
int k_pscip_write(pscip_t *val, int id_pscip, pscip_Dev dev) 
{

	uint32_t tmp_data = 0;
	uint32_t cnt=0;
	uint8_t	byte;
	uint16_t word;
	int32_t ret = 0;

//	unsigned long flags;

	#ifdef __DEBUG__
	printk(KERN_ALERT "\t\t DEBUG_K (chan %d): k_pscip_write() - beginning \n",val->chan);	
	#endif	

	if ((id_pscip >= 0) && (id_pscip < num_pscip))
		dev = &pscip_devices[id_pscip];	 	

	
	if ((ret = k_pscip_wait_linkdown(val, -1, dev)) < 0) 
		goto error;
	
	dev->irqflag[val->chan] = PSCIP_IRQ_REQUEST;
	word = (val->stat << 8) | val->address;	
	IP_write_word(dev->pIP_IOspace[val->chan]+PSCIP_WRITE_REG,word);	
	word = (val->data >> 16) & 0xffff;
	IP_write_word(dev->pIP_IOspace[val->chan]+PSCIP_WRITE_DATA, word);

// set data register 
	word = val->data & 0xffff;	
	IP_write_word(dev->pIP_IOspace[val->chan]+PSCIP_WRITE_DATA +0x02, word);


//		spin_lock_irqsave(&dev->lock[val->chan],flags); 

		wait_event_interruptible_timeout(dev->queue[val->chan],queue_flag != 0, 1);
		queue_flag = 0;
//		spin_unlock_irqrestore(&dev->lock[val->chan],flags);

	while (dev->irqflag[val->chan] != PSCIP_IRQ_SERVED) 
	{
		udelay(1);
		if ((cnt++) > PSCIP_BUSY_TOUT) 
		{
			#ifdef __DEBUG__
			printk(KERN_ALERT "\t\t DEBUG_K: ERROR: PSCIP_BUSY_TOUT\n");	
			#endif
			ret = (-PSCIP_TOUT_ERR);
			#ifdef __DEBUG__
			printk(KERN_ALERT "\t\t DEBUG_K: k_pscip_write() - end \n");	
			#endif		
			goto error;

		}
	}

	if (dev->stats[val->chan].err_flag & PSCIP_ERROR_MSK) 
	{
		ret = (-PSCIP_TX_ERR);
		if (dev->stats[val->chan].err_flag & PSCIP_LINKDOWN_MSK)
			ret = (- PSCIP_LINK_ERR);

		#ifdef __DEBUG__
		if (dev->stats[val->chan].err_flag & PSCIP_LINKDOWN_MSK)
				printk(KERN_ALERT "\t\t DEBUG_K: ERROR: PSCIP_ERROR_MSK ->PSCIP_LINK_ERR\n");
		else
			printk(KERN_ALERT "\t\t DEBUG_K: ERROR: PSCIP_ERROR_MSK -> PSCIP_TX_ERR\n");	
		#endif
		#ifdef __DEBUG__
		printk(KERN_ALERT "\t\t DEBUG_K: k_pscip_write() - end \n");	
		#endif		
		goto error;
	}	

	if (val->stat & PSCIP_ECHO_MSK) 
	{
		IP_read_word(dev->pIP_IOspace[val->chan] + PSCIP_WRITE_DATA, &word);
		tmp_data = ( word & 0xffff) << 16;
		IP_read_word(dev->pIP_IOspace[val->chan] + PSCIP_WRITE_DATA + 0x02, &word);
		tmp_data += (word & 0xffff);
		if (tmp_data != val->data)
		{
			#ifdef __DEBUG__
			printk(KERN_ALERT "\t\t DEBUG_K: ERROR: PSCIP_MISMATCH_ERR\n");	
			#endif
			ret = (-PSCIP_MISMATCH_ERR); 
			#ifdef __DEBUG__
			printk(KERN_ALERT "\t\t DEBUG_K: k_pscip_write() - end \n");	
			#endif		
			goto error;
		}
	}	

// update controller status 
	IP_read_byte(dev->pIP_IOspace[val->chan]+PSCIP_WRITE_STAT,&byte);	//ok
	dev->stats[val->chan].cntrl_stat = byte & (PSCIP_INPUTBUF_MSK |
		PSCIP_DSP_MSK | PSCIP_REMLOC_MSK | PSCIP_LINK_MSK);	

	dev->stats[val->chan].tx++;	
	
	dev->irqflag[val->chan] = PSCIP_IRQ_DONE;
	#ifdef __DEBUG__
	printk(KERN_ALERT "\t\t DEBUG_K (chan %d): k_pscip_write() - end \n",val->chan);	
	#endif			
	return 0;

	error:
		dev->irqflag[val->chan] = PSCIP_IRQ_DONE;
		#ifdef __DEBUG__
		printk(KERN_ALERT "\t\t DEBUG_K (chan %d): k_pscip_write() - error \n", val->chan);	
		#endif	
	return ret;

} 
/* end k_pscip_write() */
																	

/**	Execute reading action. 
	Function reads data (2 words) from a given address in PS
	
	@param *val	- structure containing information about the channel/link we are using
	@param id_pscip	- number of the device ( IP ) we are using, if negative value, the device passed by third argument is used
	@param dev		- device structure contaning informatin about the device (IP)
 */
int k_pscip_read(pscip_t *val, int id_pscip, pscip_Dev dev) 
{

	uint32_t tmp_data = 0,cnt=0;
	int32_t ret = 0;
	uint8_t byte;
	uint16_t word;
	
//	unsigned long flags;
	#ifdef __DEBUG__
//	int output = 0;
	printk(KERN_ALERT "\t\t DEBUG_K (chan %d): k_pscip_read() - beginning \n",val->chan);	
	#endif	

	if ((id_pscip >= 0) && (id_pscip < num_pscip)) 
	{
		dev = &pscip_devices[id_pscip];	 
	}


	if ((ret = k_pscip_wait_linkdown(val, -1, dev)) < 0)
		goto error;	
	
	dev->irqflag[val->chan] = PSCIP_IRQ_REQUEST;	
	

// set stat register 
	word = (val->stat << 8) | val->address;	
	IP_write_word(dev->pIP_IOspace[val->chan]+PSCIP_READ_REG,word);
	
// write some value just to trigger the packet to be sent 
	IP_write_word(dev->pIP_IOspace[val->chan]+PSCIP_READ_DATA + 0x02, 0);



		#ifdef __DEBUG__
		printk(KERN_ALERT "\t\t DEBUG_K: k_pscip_read() -before spin_lock\n");	
		#endif

//		spin_lock_irqsave(&dev->lock[val->chan],flags); 


		#ifdef __DEBUG__
		printk(KERN_ALERT "\t\t DEBUG_K: k_pscip_read() -after spin_lock, before wait_event, the queue_flag before wait_eventbleble: %d\n",queue_flag );	
		#endif


		wait_event_interruptible_timeout(dev->queue[val->chan],queue_flag != 0, 1);


		#ifdef __DEBUG__
		printk(KERN_ALERT "\t\t DEBUG_K: k_pscip_read() -after wait_event, before spin_unlock, after wait_event, the queue_flag : %d, \n",queue_flag);	
		#endif

		queue_flag = 0;


//		spin_unlock_irqrestore(&dev->lock[val->chan],flags);

		#ifdef __DEBUG__
		printk(KERN_ALERT "\t\t DEBUG_K: k_pscip_read() -after  spin_unlock\n");	
		#endif



	while (dev->irqflag[val->chan] != PSCIP_IRQ_SERVED) 
	{
		udelay(1);
		if ((cnt++) > PSCIP_BUSY_TOUT) 
		{
			#ifdef __DEBUG__
			printk(KERN_ALERT "\t\t DEBUG_K: ERROR: PSCIP_BUSY_TOUT\n");	
			#endif	
			ret = (-PSCIP_TOUT_ERR);
			goto error;
		}
	}
	
// update controller status 

	IP_read_byte(dev->pIP_IOspace[val->chan] + PSCIP_READ_STAT, &byte);  //ok
	if (dev->stats[val->chan].err_flag & PSCIP_ERROR_MSK) 
	{	
		#ifdef __DEBUG__
			ret = (-PSCIP_TX_ERR);
			if (dev->stats[val->chan].err_flag & PSCIP_LINKDOWN_MSK)
			{
				ret = (- PSCIP_LINK_ERR);
				printk(KERN_ALERT "\t\t DEBUG_K: ERROR: PSCIP_ERROR_MSK -> PSCIP_LINK_ERR\n");	
			}
			else
				printk(KERN_ALERT "\t\t DEBUG_K: ERROR: PSCIP_ERROR_MSK - > PSCIP_TX_ERR\n");	
		#else
			ret = (-PSCIP_TX_ERR);
			if (dev->stats[val->chan].err_flag & PSCIP_LINKDOWN_MSK)
				ret = (- PSCIP_LINK_ERR);
		#endif	
		goto error;
	}	
	

	IP_read_word(dev->pIP_IOspace[val->chan] + PSCIP_READ_DATA, &word);
	tmp_data = (word & 0xffff) << 16;
	IP_read_word(dev->pIP_IOspace[val->chan] + PSCIP_READ_DATA + 0x02, &word);
	tmp_data += (word  & 0xffff);
	
	val->data = tmp_data;

// update controller status 
	IP_read_byte(dev->pIP_IOspace[val->chan] + PSCIP_READ_STAT, &byte);  //ok
	
	IP_read_byte(dev->pIP_IOspace[val->chan] + PSCIP_READ_STAT, &(val->stat));  //ok
	IP_read_byte(dev->pIP_IOspace[val->chan] + PSCIP_READ_ADDR, &(val->address));  //ok
	
	dev->stats[val->chan].cntrl_stat = byte & (PSCIP_INPUTBUF_MSK |
		PSCIP_DSP_MSK | PSCIP_REMLOC_MSK | PSCIP_LINK_MSK);	
	
	dev->stats[val->chan].rx++;
	
	dev->irqflag[val->chan] = PSCIP_IRQ_DONE;
	#ifdef __DEBUG__
	printk(KERN_ALERT "\t\t DEBUG_K (chan %d): k_pscip_read() - end \n",val->chan);	
	#endif			
	return 0;

	error:
		dev->irqflag[val->chan] = PSCIP_IRQ_DONE;
	#ifdef __DEBUG__
		printk(KERN_ALERT "\t\t DEBUG_K (chan %d): k_pscip_read() - error \n",val->chan);	
	#endif
	return ret;

} 
/* end k_pscip_read() */


/**	Write wave value. 
	Function writes datat (2 words) of waveform to given address in PS
		
	@param *val	- structure containing information about the channel/link we are using
	@param id_pscip	- number of the device ( IP ) we are using, if negative value, the device passed by third argument is used
	@param dev		- device structure contaning informatin about the device (IP)
 */
int k_pscip_wrwave(pscip_t *val, int id_pscip, pscip_Dev dev) 
{

	uint32_t tmp_data = 0,cnt=0;
	int32_t ret = 0;
	uint16_t word;
	uint8_t byte;


//	unsigned long flags;


	#ifdef __DEBUG__
	printk(KERN_ALERT "\t\t DEBUG_K (chan %d): k_pscip_wrwave() - beginning \n",val->chan);	
	#endif	
	if ((id_pscip >= 0) && (id_pscip < num_pscip))
		dev = &pscip_devices[id_pscip];	 	


	if ((ret = k_pscip_wait_linkdown(val, -1, dev)) < 0)
		goto error;

	dev->irqflag[val->chan] = PSCIP_IRQ_REQUEST;

// set stat register 
	word = (val->stat << 8) | val->address;	
	IP_write_word(dev->pIP_IOspace[val->chan]+PSCIP_WRWAVE_REG,word);
// set data register 
	word = (val->data >> 16) & 0xffff;
	IP_write_word(dev->pIP_IOspace[val->chan]+PSCIP_WRWAVE_DATA,word);
// set data register 
	word = val->data & 0xffff;
	IP_write_word(dev->pIP_IOspace[val->chan]+PSCIP_WRWAVE_DATA + 0x2,word);

//		spin_lock_irqsave(&dev->lock[val->chan],flags); 
		wait_event_interruptible_timeout(dev->queue[val->chan],queue_flag != 0, 1);
		queue_flag = 0;
//		spin_unlock_irqrestore(&dev->lock[val->chan],flags);

	while (dev->irqflag[val->chan] != PSCIP_IRQ_SERVED)
	{
		udelay(1);
		if ((cnt++) > PSCIP_BUSY_TOUT) 
		{
			#ifdef __DEBUG__
				printk(KERN_ALERT "\t\t DEBUG_K: ERROR: PSCIP_BUSY_TOUT\n");	
			#endif	
			ret = (-PSCIP_TOUT_ERR);
			goto error;
		}
	}

	
	if (dev->stats[val->chan].err_flag & PSCIP_ERROR_MSK) 
	{
		#ifdef __DEBUG__

			ret = (-PSCIP_TX_ERR);
			if (dev->stats[val->chan].err_flag & PSCIP_LINKDOWN_MSK)
			{
				ret = (- PSCIP_LINK_ERR);
				printk(KERN_ALERT "\t\t DEBUG_K: ERROR: PSCIP_ERROR_MSK -> PSCIP_LINKDOWN_MSK\n");
			}
			else
				printk(KERN_ALERT "\t\t DEBUG_K: ERROR: PSCIP_ERROR_MSK -> PSCIP_TX_ERR\n");	
		#else
			ret = (-PSCIP_TX_ERR);
			if (dev->stats[val->chan].err_flag & PSCIP_LINKDOWN_MSK)
				ret = (- PSCIP_LINK_ERR);	
		#endif	
		goto error;
	}	

	if (val->stat & PSCIP_ECHO_MSK) 
	{
		IP_read_word(dev->pIP_IOspace[val->chan]+PSCIP_WRWAVE_DATA ,&word);
		tmp_data = (word & 0xffff) << 16;
		IP_read_word(dev->pIP_IOspace[val->chan]+PSCIP_WRWAVE_DATA + 0x2 ,&word);
		tmp_data += (word & 0xffff);
		if (tmp_data != val->data)
		{
			ret = (-PSCIP_MISMATCH_ERR); 
			#ifdef __DEBUG__
				printk(KERN_ALERT "\t\t DEBUG_K: ERROR: PSCIP_MISMATCH_ERR\n");	
			#endif	
			goto error;			
		}	
	}	

// update controller status 
	IP_read_byte(dev->pIP_IOspace[val->chan]+PSCIP_WRWAVE_STAT ,&byte);  //ok
	dev->stats[val->chan].cntrl_stat = byte & (PSCIP_INPUTBUF_MSK |
		PSCIP_DSP_MSK | PSCIP_REMLOC_MSK | PSCIP_LINK_MSK);	

	dev->stats[val->chan].tx++;	

	dev->irqflag[val->chan] = PSCIP_IRQ_DONE;
	#ifdef __DEBUG__
		printk(KERN_ALERT "\t\t DEBUG_K (chan %d): k_pscip_wrwave() - end \n",val->chan);	
	#endif
	return 0;

	error:
		dev->irqflag[val->chan] = PSCIP_IRQ_DONE;
		#ifdef __DEBUG__
			printk(KERN_ALERT "\t\t DEBUG_K: k_pscip_wrwave() - error \n");	
		#endif
	return ret;
} 
/* end k_pscip_wrwave() */
																	

/**	Execute reading action
	Function reads data (2 words) of a waveform from given address in PS
	
	@param *val	- structure containing information about the channel/link we are using
	@param id_pscip	- number of the device ( IP ) we are using, if negative value, the device passed by third argument is used
	@param dev		- device structure contaning informatin about the device (IP)

 */
int k_pscip_rdwave(pscip_t *val, int id_pscip, pscip_Dev dev) 
{

	uint32_t tmp_data = 0,cnt=0;
	int32_t ret = 0;
	uint16_t word;
	uint8_t byte;
	

//		unsigned long flags;

	#ifdef __DEBUG__
		printk(KERN_ALERT "\t\t DEBUG_K (chan %d): k_pscip_rdwave() - beginning \n",val->chan);	
	#endif
	if ((id_pscip >= 0) && (id_pscip < num_pscip))
		dev = &pscip_devices[id_pscip];	 


	if ((ret = k_pscip_wait_linkdown(val, -1, dev)) < 0)
		goto error;	

	dev->irqflag[val->chan] = PSCIP_IRQ_REQUEST;
		
//set stat register 
	word = (val->stat << 8) | val->address;	
	IP_write_word(dev->pIP_IOspace[val->chan]+PSCIP_RDWAVE_REG,word);
// write some value just to trigger the packet to be sent 
	IP_write_word(dev->pIP_IOspace[val->chan]+PSCIP_RDWAVE_DATA+0x2, 0);	

//		spin_lock_irqsave(&dev->lock[val->chan],flags); 
		wait_event_interruptible_timeout(dev->queue[val->chan],queue_flag != 0, 1);
		queue_flag = 0;
//		spin_unlock_irqrestore(&dev->lock[val->chan],flags);

	while (dev->irqflag[val->chan] != PSCIP_IRQ_SERVED) 
	{
		udelay(1);
		if ((cnt++) > PSCIP_BUSY_TOUT)
		{
			#ifdef __DEBUG__
				printk(KERN_ALERT "\t\t DEBUG_K: ERROR: PSCIP_BUSY_TOUT\n");	
			#endif	
			ret = (-PSCIP_TOUT_ERR);
			goto error;
		}
	}
	dev->irqflag[val->chan] = 0;
	
	if (dev->stats[val->chan].err_flag & PSCIP_ERROR_MSK) 
	{
		ret = (-PSCIP_TX_ERR);
		if (dev->stats[val->chan].err_flag & PSCIP_LINKDOWN_MSK)
			ret = (- PSCIP_LINK_ERR);
		#ifdef __DEBUG__
			printk(KERN_ALERT "\t\t DEBUG_K: ERROR: PSCIP_ERROR_MSK\n");	
		#endif
		goto error;
	}	
	
// get data register 
	IP_read_word(dev->pIP_IOspace[val->chan]+PSCIP_RDWAVE_DATA, &word);
	
	tmp_data = (word & 0xffff) << 16;
	IP_read_word(dev->pIP_IOspace[val->chan]+PSCIP_RDWAVE_DATA+0x2, &word);
	tmp_data += (word & 0xffff);	
	val->data = tmp_data;	
	
// update controller status 
	IP_read_byte(dev->pIP_IOspace[val->chan]+PSCIP_RDWAVE_STAT, &byte);  //ok
	dev->stats[val->chan].cntrl_stat = byte & (PSCIP_INPUTBUF_MSK |
		PSCIP_DSP_MSK | PSCIP_REMLOC_MSK | PSCIP_LINK_MSK);	

	dev->stats[val->chan].rx++;

	dev->irqflag[val->chan] = PSCIP_IRQ_DONE;
	#ifdef __DEBUG__
		printk(KERN_ALERT "\t\t DEBUG_K (chan %d): k_pscip_rdwave() - end \n",val->chan);	
	#endif		
	return 0;

	error:
		dev->irqflag[val->chan] = PSCIP_IRQ_DONE;
		#ifdef __DEBUG__
			printk(KERN_ALERT "\t\t DEBUG_K: k_pscip_rdwave() - error \n");	
		#endif
	return ret;

}
 /* end k_pscip_rdwave() */


/**	Read link status.
	Reads FPGA status register
	@param *val	- structure reflecting status registers in FPGA - is filled with IP debug information
	@param id_pscip	- number of the device ( IP ) we are using, if negative value, the device passed by third argument is used
	@param dev		- device structure contaning informatin about the device (IP)
 */
int k_pscip_rdlinkstat(pscip_t *val, int id_pscip, pscip_Dev dev) 
{

	uint8_t byte;
	#ifdef __DEBUG__
		printk(KERN_ALERT "\t\t DEBUG_K (chan %d): k_pscip_rdlinkstat() - beginning \n",val->chan);	
	#endif	
	if ((id_pscip >= 0) && (id_pscip < num_pscip))
		dev = &pscip_devices[id_pscip];	 
		
	switch (val->address) {
		case PSCIP_WRHPRIO_IDX:
			IP_read_byte(dev->pIP_IOspace[val->chan]+PSCIP_HPRIOWRITE_STAT, &byte);	 //ok
			break;
		case PSCIP_WRITE_IDX:
			IP_read_byte(dev->pIP_IOspace[val->chan]+PSCIP_WRITE_STAT, &byte);	//ok
			break;
		case PSCIP_READ_IDX:
			IP_read_byte(dev->pIP_IOspace[val->chan]+PSCIP_READ_STAT, &byte);	//ok
			break;
		case PSCIP_WRWAVE_IDX:
			IP_read_byte(dev->pIP_IOspace[val->chan]+PSCIP_WRWAVE_STAT, &byte);  //ok
			break;
		case PSCIP_RDWAVE_IDX:
			IP_read_byte(dev->pIP_IOspace[val->chan]+PSCIP_RDWAVE_STAT, &byte);  //ok
			break;
		default:
				return (-PSCIP_PARAM_ERR);								
	}
	
	val->data = byte & 0xff;
	#ifdef __DEBUG__
		printk(KERN_ALERT "\t\t DEBUG_K (chan %d): k_pscip_rdlinkstat() - end \n",val->chan);	
	#endif		
	return 0;

}
 /* end k_pscip_rdlinkstat() */


/**	Read FPGA  status register
	FPGA  status register is for debugging purposes and reflects the error status of the FPGA.
	@param *val	- structure reflecting status registers in FPGA - is filled with IP debug information
	@param id_pscip	- number of the device ( IP ) we are using, if negative value, the device passed by third argument is used
	@param dev		- device structure contaning informatin about the device (IP)
 */
int k_pscip_rdfpgastat(pscip_fpga_t *val, int id_pscip, pscip_Dev dev) 
{

	uint16_t word;
	#ifdef __DEBUG__
		printk(KERN_ALERT "\t\t DEBUG_K  k_pscip_rdfpgastat() - beginning \n");	
	#endif
	if ((id_pscip >= 0) && (id_pscip < num_pscip))
		dev = &pscip_devices[id_pscip];	 
		
	IP_read_word(dev->pIP_IOspace[val->channel]+PSCIP_FPGA_HIGH, &word);	
	val->high_data = word;

	IP_read_word(dev->pIP_IOspace[val->channel]+PSCIP_FPGA_MID, &word);	
	val->mid_data = word;
	
	IP_read_word(dev->pIP_IOspace[val->channel]+PSCIP_FPGA_LOW, &word);	
	val->low_data = word;
	#ifdef __DEBUG__
		printk(KERN_ALERT "\t\t DEBUG_K: k_pscip_rdfpgastat() - end \n");	
	#endif	
	return 0;

}
 /* end k_pscip_rdfpgastat() */


/**	Read transmission statistics. 
	Retursn to the user all the information about errors stored in device structure during operation of the driver. 
	It does not read informatin from IP but rather from device structure flags.
	
	@param *val	- structure filled with the information about errors
	@param id_pscip	- number of the device ( IP ) we are using, if negative value, the device passed by third argument is used
	@param dev		- device structure contaning informatin about the device (IP)	
 */
int k_pscip_rdstatistics(pscip_stats_t *val, int id_pscip, pscip_Dev dev) 
{

	int i;
	#ifdef __DEBUG__
		printk(KERN_ALERT "\t\t DEBUG_K : k_pscip_rdstatistics() - beginning \n");	
	#endif	
	if ((id_pscip >= 0) && (id_pscip < num_pscip))
		dev = &pscip_devices[id_pscip];	 
		
	val->rx = dev->stats[val->channel].rx;
	val->tx = dev->stats[val->channel].tx;
	val->err_flag = dev->stats[val->channel].err_flag;			
	
	for (i = 0; i < 5; i++)
		val->link_err_flag[i] = dev->stats[val->channel].link_err_flag[i];
	
	val->linkdown_cnt = dev->stats[val->channel].linkdown_cnt;
	val->txerr_cnt = dev->stats[val->channel].txerr_cnt;
	#ifdef __DEBUG__
		printk(KERN_ALERT "\t\t DEBUG_K: k_pscip_rdstatistics() - end \n");	
	#endif		
	return 0;

}
 /* end k_pscip_rdstatistics() */



/**	Reset IP.
	This is a hardware reset which is triggered from the Carrier.
	
	@param id_pscip	- number of the device ( IP ) we are using, if negative value, the device passed by third argument is used
	@param dev		- device structure contaning informatin about the device (IP)
 */
int k_pscip_reset(int id_pscip, pscip_Dev dev) 
{

	#ifdef __DEBUG__
	printk(KERN_ALERT "\t\t DEBUG_K : k_pscip_reset() - beginning \n");	
	#endif	
	
	if ((id_pscip >= 0) && (id_pscip < num_pscip))
		dev = &pscip_devices[id_pscip];	 
	
	if(dev->IPnumber==0)
		IP_write_word(dev->pIPinterfaceReg + IP_RESET, 0x01);
	else if(dev->IPnumber==1)
		IP_write_word(dev->pIPinterfaceReg + IP_RESET, 0x02);
	else
		return - ENXIO;
									
	#ifdef __DEBUG__
	printk(KERN_ALERT "\t\t DEBUG_K : k_pscip_reset() - end \n");	
	#endif		
	return 0;	
}
 /*end k_pscip_reset() */
 
/**	Clean error counter in FPGA status register. 
	Error coutner indicates the number of errors which occured (ex. due to transmition error). 
	The couter is clean before every read/write function from/to PS 

	@param *val	- structure containing information about the channel/link we are using
	@param id_pscip	- number of the device ( IP ) we are using, if negative value, the device passed by third argument is used
	@param dev		- device structure contaning informatin about the device (IP)
*/ 
int k_pscip_clear_err_counter(pscip_t *val, int id_pscip, pscip_Dev dev)
{
	#ifdef __DEBUG__
	printk(KERN_ALERT "\t\t DEBUG_K (chan %d): k_pscip_reset() - beginning \n",val->chan);	
	#endif	
	if ((id_pscip >= 0) && (id_pscip < num_pscip))
		dev = &pscip_devices[id_pscip];	 
		
	IP_write_byte(dev->pIP_IOspace[val->chan]+PSCIP_FPGA_HIGH, 0x00);
	udelay(1000);
	IP_write_word(dev->pIP_IOspace[val->chan]+PSCIP_FPGA_HIGH, PSCIP_ERRCLEAR_MSK);
	IP_write_word(dev->pIP_IOspace[val->chan]+PSCIP_FPGA_LOW, 0x0000);
	return 0;
} 
/**	Dumps content of all the registers of both IPs on the carrier and of Local Space O (BADR 2) on the Carrier.
	Local Space 0 contains IP A/B control registers, revision ID and status register.
	It does not metter which IP calles the function, it always dumps information for entire carrier address I/O space used for interfacing IPs
	IF __DEBUG__ defined, it also interprets content of registers in FPGA status register of calling IP and content of PCI Carrier Status register

	@param *val	- structure containing information about the channel/link we are using
	@param id_pscip	- number of the device ( IP ) we are using, if negative value, the device passed by third argument is used
	@param dev		- device structure contaning informatin about the device (IP)
*/
int k_pscip_dump_all_registers( pscip_t *val, int id_pscip, pscip_Dev dev)
{
	int i=0;
	int local_address;
	int local_chan;
	int local_IP;
	uint16_t word;
	uint16_t word_2;
	printk(KERN_ALERT "\t\t DEBUG_K: ==================================================================================\n");
	printk(KERN_ALERT "\t\t DEBUG_K: ====================== Local Space 1 =============================================\n");
			
	for(i=0x00;i < 0x01FF; i=i+0x02)
	{
		if( i < PSCIP_CHAN_OFFSET)	//first link/channel or first IP
		{
			local_chan = 0;
			local_IP = 0;
			local_address = i;
		}
		else if(i <  2 * PSCIP_CHAN_OFFSET) //second link/channel of second IP
		{
			local_chan = 1;
			local_IP = 0;		
			local_address = i - PSCIP_CHAN_OFFSET;				
		}	
		else if(i < PSCIP_IP_OFFSET)	//bewteen first and second IP
		{
			local_chan = -1;
			local_IP = 0;		
			local_address = 0xff;				
		}
		else if(i < PSCIP_IP_OFFSET + PSCIP_CHAN_OFFSET)	//first link/channle of second IP
		{
			local_chan = 0;
			local_IP = 1;	
			local_address = i - PSCIP_IP_OFFSET;					
		}
		else if(i < PSCIP_IP_OFFSET + 2 * PSCIP_CHAN_OFFSET)	//second link/channel of second IP
		{
			local_chan = 1;
			local_IP = 1;						
			local_address = i - PSCIP_IP_OFFSET + PSCIP_CHAN_OFFSET;
		}
		else if(i < 0x1FF)	//after second IP
		{
			local_chan = -1;
			local_IP = 1;			
			local_address = 0xff;			
		}
		if(dev->IPnumber)	//IO space of second IP is shifted by the offset, so to show entire carrier memory space we need to shif it back
			IP_read_word(dev->pIP_IOspace[0] - PSCIP_IP_OFFSET + i, &word);
		else						
			IP_read_word(dev->pIP_IOspace[0]+i, &word);
		printk(KERN_ALERT "\t\tvalue: 0x%4x, local addr: 0x%4x, local chan: %d, local IP: %d ----choosen chan: %d, choosen IP: %d, glob addr: 0x%4x \n",word,local_address, local_chan, local_IP, val->chan, dev->IPnumber,i);
			
	}
	printk(KERN_ALERT "\t\t DEBUG_K: ==================================================================================\n");
	printk(KERN_ALERT "\t\t DEBUG_K: ====================== Local Space 0 =============================================\n");
	for(i=0x00;i < 0x0C; i=i+0x02)
	{
		IP_read_word(dev->pIPinterfaceReg+i, &word);
		printk(KERN_ALERT "\t\tvalue: 0x%4x, addr: 0x%4x,  chan: %d, local IP: %d \n",word,i, val->chan, dev->IPnumber);
	}
	printk(KERN_ALERT "\t\t DEBUG_K: ==================================================================================\n");
	printk(KERN_ALERT "\t\t DEBUG_K: ====================== PCI configuration Register ================================\n");
	for(i=0x00;i < 0x74; i=i+0x04)
	{
		IP_read_word(dev->pPCIinterfaceReg+i, &word);
		IP_read_word(dev->pPCIinterfaceReg+i+2, &word_2);
		printk(KERN_ALERT "\t\t DEBUG_K: value: 0x%4x%4x, addr: 0x%4x,  chan: %d, local IP: %d \n",word_2,word,i, val->chan, dev->IPnumber);
	}

	return 0;
}
/**	Does hardware magic to enable interrupt on calling IP. 
	Called when first reference to give IP (/dev/pscipX) is openned
	Interrupt 0 is triggered by channel/link 0
	Interrupt 1 is triggered by channel/link 1
	
	@param dev		- device structure contaning informatin about the device (IP)
*/
int k_irq_enable(pscip_Dev dev)
{
	#ifdef __DEBUG__
	printk(KERN_ALERT "\t\t DEBUG_K: k_irq_enable() - beginning\n");	
	#endif	
	
	if(dev->IPnumber == 0)
		IP_write_word(dev->pIPinterfaceReg + IP_A_CONTROL, IP_INT0_EN | IP_INT1_EN | 0x0000);
	else if(dev->IPnumber ==1)
		IP_write_word(dev->pIPinterfaceReg + IP_B_CONTROL, IP_INT0_EN | IP_INT1_EN | 0x0000);
	else
		return - ENXIO; 																				//no such device or address
 
	#ifdef __DEBUG__
	printk(KERN_ALERT "\t\t DEBUG_K: k_irq_enable() - end\n");	
	#endif
	
  return 0;
}
/**	Does hardware magic to disable interrupt on calling IP. 
	Called when last reference to give IP (/dev/pscipX) is closed

	@param dev		- device structure contaning informatin about the device (IP)
*/
int k_irq_disable(pscip_Dev dev)
{
	#ifdef __DEBUG__
	printk(KERN_ALERT "\t\t DEBUG_K: k_irq_disable() - beginningn");	
	#endif
	
	if(dev->IPnumber == 0)
		IP_write_word(dev->pIPinterfaceReg + IP_A_CONTROL, 0x0000);
	else if(dev->IPnumber ==1)
		IP_write_word(dev->pIPinterfaceReg + IP_B_CONTROL, 0x0000);	
	else
		return - ENXIO;		//"No such device or address"

	#ifdef __DEBUG__
	printk(KERN_ALERT "\t\t DEBUG_K: k_irq_disable() - end\n");	
	#endif
	
	return 0;
}
/**	Veryfies the source of the interrupt.
	It checks whether the interupt was triggered by an IP represented by dev device structure registerred with given interrupt handler
	
	@param dev		- device structure contaning informatin about the device (IP)
	@return 		- returns number of channel which triggered the interrupt or error	 

*/
int k_if_handle_irq(pscip_Dev dev)
{
	uint16_t word = 0 ;
	
	#ifdef __DEBUG__
	printk(KERN_ALERT "\t\t DEBUG_K: k_if_handle_irq() - beginning\n");	
	#endif

	IP_read_word(dev->pIPinterfaceReg + IP_STATUS, &word);

	#ifdef __DEBUG__
	printk(KERN_ALERT "\t\t DEBUG_K: k_if_handle_irq() - end\n");	
	#endif

	if(dev->IPnumber == 0 )
	{
		if(word & IP_INT0_A)
			return 0;		//chan 0
		else if(word & IP_INT1_A)
			return 1; 		//chan1
	}
	else if(dev->IPnumber == 1)
	{
		if(word & IP_INT0_B)
			return 0;		//chan 0
		else if(word & IP_INT1_B)
			return 1; 		//chan1
	}
	else 
		return -1;			//something wrong

	return 0;
}



/**	Execute reading action. 
	Function reads data (2 words) from a given address in PS, it is used for tests because it writes the data provided so that it can be read on 
	another IP, the k_pscip_read() function writes 0 regardless of the data provided
	
	@param *val	- structure containing information about the channel/link we are using
	@param id_pscip	- number of the device ( IP ) we are using, if negative value, the device passed by third argument is used
	@param dev		- device structure contaning informatin about the device (IP)
 */
int k_pscip_read_test(pscip_t *val, int id_pscip, pscip_Dev dev) 
{

	uint32_t tmp_data = 0,cnt=0;
	int32_t ret = 0;
	uint8_t byte;
	uint16_t word;
	
//		unsigned long flags;
	#ifdef __DEBUG__
//	int output = 0;
	printk(KERN_ALERT "\t\t DEBUG_K: k_pscip_read() - beginning \n");	
	#endif	

	if ((id_pscip >= 0) && (id_pscip < num_pscip)) 
	{
		dev = &pscip_devices[id_pscip];	 
	}


	if ((ret = k_pscip_wait_linkdown(val, -1, dev)) < 0)
		goto error;	
	
	dev->irqflag[val->chan] = PSCIP_IRQ_REQUEST;	
	

// set stat register 
	word = (val->stat << 8) | val->address;	
	IP_write_word(dev->pIP_IOspace[val->chan]+PSCIP_READ_REG,word);


///////////	old	//////////////////////////////////////////////////////////////////	
// write some value just to trigger the packet to be sent 
//	IP_write_word(dev->pIP_IOspace[val->chan]+PSCIP_READ_DATA + 0x02, 0);
//////////	end of old	/////////////////////////////////////////////////////////////


///////////	new	//////////////////////////////////////////////////////////////////
	word = (val->data >> 16) & 0xffff;
	IP_write_word(dev->pIP_IOspace[val->chan]+PSCIP_READ_DATA, word);

// set data register 
	word = val->data & 0xffff;	
	IP_write_word(dev->pIP_IOspace[val->chan]+PSCIP_READ_DATA +0x02, word);
//////////	end of new	///////////////////////////////////////////////////////////////


		#ifdef __DEBUG__
		printk(KERN_ALERT "\t\t DEBUG_K: k_pscip_read() -before spin_lock\n");	
		#endif

//		spin_lock_irqsave(&dev->lock[val->chan],flags); 


		#ifdef __DEBUG__
		printk(KERN_ALERT "\t\t DEBUG_K: k_pscip_read() -after spin_lock, before wait_event, the queue_flag before wait_eventbleble: %d\n",queue_flag );	
		#endif


		wait_event_interruptible_timeout(dev->queue[val->chan],queue_flag != 0, 1);


		#ifdef __DEBUG__
		printk(KERN_ALERT "\t\t DEBUG_K: k_pscip_read() -after wait_event, before spin_unlock, after wait_event, the queue_flag : %d, \n",queue_flag);	
		#endif

		queue_flag = 0;


//		spin_unlock_irqrestore(&dev->lock[val->chan],flags);

		#ifdef __DEBUG__
		printk(KERN_ALERT "\t\t DEBUG_K: k_pscip_read() -after  spin_unlock\n");	
		#endif



	while (dev->irqflag[val->chan] != PSCIP_IRQ_SERVED) 
	{
		udelay(1);
		if ((cnt++) > PSCIP_BUSY_TOUT) 
		{
			#ifdef __DEBUG__
			printk(KERN_ALERT "\t\t DEBUG_K: ERROR: PSCIP_BUSY_TOUT\n");	
			#endif	
			ret = (-PSCIP_TOUT_ERR);
			goto error;
		}
	}
	
// update controller status 

	IP_read_byte(dev->pIP_IOspace[val->chan] + PSCIP_READ_STAT, &byte);  //ok
	if (dev->stats[val->chan].err_flag & PSCIP_ERROR_MSK) 
	{	
		#ifdef __DEBUG__
			ret = (-PSCIP_TX_ERR);
			if (dev->stats[val->chan].err_flag & PSCIP_LINKDOWN_MSK)
			{
				ret = (- PSCIP_LINK_ERR);
				printk(KERN_ALERT "\t\t DEBUG_K: ERROR: PSCIP_ERROR_MSK -> PSCIP_LINK_ERR\n");	
			}
			else
				printk(KERN_ALERT "\t\t DEBUG_K: ERROR: PSCIP_ERROR_MSK - > PSCIP_TX_ERR\n");	
		#else
			ret = (-PSCIP_TX_ERR);
			if (dev->stats[val->chan].err_flag & PSCIP_LINKDOWN_MSK)
				ret = (- PSCIP_LINK_ERR);
		#endif	
		goto error;
	}	
	

	IP_read_word(dev->pIP_IOspace[val->chan] + PSCIP_READ_DATA, &word);
	tmp_data = (word & 0xffff) << 16;
	IP_read_word(dev->pIP_IOspace[val->chan] + PSCIP_READ_DATA + 0x02, &word);
	tmp_data += (word  & 0xffff);
	
	val->data = tmp_data;

// update controller status 
	IP_read_byte(dev->pIP_IOspace[val->chan] + PSCIP_READ_STAT, &byte);  //ok
	
	IP_read_byte(dev->pIP_IOspace[val->chan] + PSCIP_READ_STAT, &(val->stat));  //ok
	IP_read_byte(dev->pIP_IOspace[val->chan] + PSCIP_READ_ADDR, &(val->address));  //ok
	
	dev->stats[val->chan].cntrl_stat = byte & (PSCIP_INPUTBUF_MSK |
		PSCIP_DSP_MSK | PSCIP_REMLOC_MSK | PSCIP_LINK_MSK);	
	
	dev->stats[val->chan].rx++;
	
	dev->irqflag[val->chan] = PSCIP_IRQ_DONE;
	#ifdef __DEBUG__
	printk(KERN_ALERT "\t\t DEBUG_K: k_pscip_read() - end \n");	
	#endif			
	return 0;

	error:
		dev->irqflag[val->chan] = PSCIP_IRQ_DONE;
	#ifdef __DEBUG__
		printk(KERN_ALERT "\t\t DEBUG_K: k_pscip_read() - error \n");	
	#endif
	return ret;

} 
/* end k_pscip_read() */


/* EOF */
