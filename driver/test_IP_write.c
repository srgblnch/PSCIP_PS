/*************************************************************************************
interpretation by Maciek

the executed test program peforms the action/command defined by providing appropratate argument,
command is performed on a particular channel of choosen device (minor number) on a VME specified 
by the address

everything is defined by the arguments inputed when invoking program
*************************************************************************************/

/*
 * test.c: use async notification to read stdin
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "pscip.h"


//	typedef struct {
//			unsigned 	int 	chan; 	 /* channel 0/1 */
//			unsigned 	char 	stat,   	/* status register */ 
//						address;  	/* address register */
//					int 	data;							
//	} pscip_t; 
/********************************************************************************
how to start the program:
$ executable <devidx(0,1..20)> 

parameter:
-	<devidx(0,1..20)>	- 	- device index
********************************************************************************/
static char psc_errmsg[][PSCIP_SIZE_MSG] = {
	"Echo value doesn't match",	
	"Remote control disabled",
	"Generic reading error",	
	"Data timeout",
	"Input buffer full",
	"Dsp stopped",
	"Link is down",
	"Transmission error",	
	"Parameter error",
	"VME bus error"	
};
/*
 * pscip_get_errmsg()
 * Convert error code in a error message
 */
 

 static char *pscip_get_errmsg(int err)
{
	int offset = err - PSCIP_IOCTL_MAGIC;
	if ((offset >= 0) && (offset < (sizeof(psc_errmsg)/PSCIP_SIZE_MSG))) 
		return psc_errmsg[offset];
			
	return "Linux error message";

	return 0;		//added for 2.6 - temporary to avoid warnings	
} 
/* pscip_get_errmsg() */

int main(int argc, char *argv[])
{
	int fd_ref,fd_test,i;
	int err,tmp;
	char ref_devname[20];
	char test_devname[20];
	float ftmp;
	int cnt = 0;
	char command=0;
	char addr[10];
	char data[10];
	char cformat[10];
	char format=0;
	char cos=0;
	unsigned long pciaddr=0;

	pscip_t val_ref;
	pscip_t val_test;	
	
	if ( (argc != 2)) 
	{
//		printf("Usage: %s <devidx(0,1..20)> <chan(0,1)> <address> <r/w/a/t> <data type(i,x,f)> <value(only write)> \n",argv[0]);
		printf("Usage: %s <devidx(0,1..20)>   \n",argv[0]);
		printf("NOTE: At this stage of development the program should be opened: %s 0   \n",argv[0]);
		return 0;
	}	
	sprintf(test_devname,"/dev/pscip%d",atoi(argv[1]));
		

	if ((fd_test = open(test_devname, O_RDWR)) < 0) 
	{
		printf("Cannot open test /dev/%s\n",test_devname);
		return 0;
	}	 
	else
	{
		printf("%s has been openned sucessfully\n", test_devname);
	}	

//////////////////////// TEST /////////////////////////////
	
	while(1)
	{
		val_ref.chan = 0;
		val_ref.stat = 0xc0; /* reading status register */ 
		val_ref.address = 0x3c;
		val_ref.data = 0x12345678;
		printf("writing to chan=%d of reference IP:\n",val_ref.chan);
		printf("stat=0x%x:\n",val_ref.stat);
		printf("address=0x%x:\n",val_ref.address);
		printf("data=0x%x:\n",val_ref.data);
		
		if ((err = ioctl(fd_test, PSCIP_WRITE, &val_ref)))
		{
			printf("Ioctl error: %s, err_type=%d\n",pscip_get_errmsg(err),err);
			//return 0;
		}
	}
	

//////////////////////// TEST END /////////////////////////////
	close(fd_ref);
	close(fd_test);
	return 0;
		
}
