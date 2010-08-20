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
	int number = 0;
	
	pscip_t val_test;	


	if ((argc != 2)) 
	{

		printf("Usage: %s <devidx(0,1..20)>   \n",argv[0]);
		printf("NOTE: At this stage of development the program should be opened: %s 0  \n",argv[0]);
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
	
	val_test.chan = 0;
	val_test.stat = 0; /* reading status register */ 
	val_test.address = 0;
	val_test.data = 0;


	while(1)
	{
		printf("%d loop number\n", number);
		
		if ((err = ioctl(fd_test, PSCIP_READ, &val_test)))
		{
			printf("Ioctl error: %s, err_type=%d\n",pscip_get_errmsg(err),err);
			//return 0;
		}
	
		printf("reading chan=%d from test IP:\n",val_test.chan);
		printf("stat=0x%x:\n",val_test.stat);
		printf("address=0x%x:\n",val_test.address);
		printf("data=0x%x:\n",val_test.data);
		number ++ ;
	}



//////////////////////// TEST END /////////////////////////////
	close(fd_test);
	return 0;
		
}
