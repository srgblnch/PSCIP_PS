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

pscip_t val;
//	typedef struct {
//			unsigned 	int 	chan; 	 /* channel 0/1 */
//			unsigned 	char 	stat,   	/* status register */ 
//						address;  	/* address register */
//					int 	data;							
//	} pscip_t; 
/********************************************************************************
how to start the program:
$ executable <devidx(0,1..20)> <chan(0,1)> <address> <r/w/a/t> <data type(i,x,f)> <value(only write)> \n",argv[0]);

parameter:
-	<devidx(0,1..20)>	- device index
-	<chan(0,1)> 		- chanal
-	<address> 		- adress - address passed by fiber to the power supply, not vme addres!!!!!!!!!!
-	<r/w/a/t>		- read/write/write wave/read wave
-	<data type(i,x,f)>	- data type??: integer, hex, float
-	<value(only write)> - value, if writing
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
	int fd,i;
	int err,tmp;
	char devname[20];
	float ftmp;
	int cnt = 0;
	char command=0;
	char addr[10];
	char data[10];
	char cformat[10];
	char format=0;
	char cos=0;
	unsigned long pciaddr=0;
	
	
	if ((argc != 6) && (argc != 3)) 
	{
//		printf("Usage: %s <devidx(0,1..20)> <chan(0,1)> <address> <r/w/a/t> <data type(i,x,f)> <value(only write)> \n",argv[0]);
		printf("Usage: %s <devidx(0,1..20)> <chan(0,1)>  \n",argv[0]);
		printf("NOTE: At this stage of development the program should be opened: %s 0 0  \n",argv[0]);
		return 0;
	}	
	sprintf(devname,"/dev/pscip%d",atoi(argv[1]));
					 
	if ((fd = open(devname, O_RDWR)) < 0) 
	{
		printf("Cannot open /dev/pscip\n");
		return 0;
	}	 
	else
		printf("%s has been openned sucessfully\n", devname);

	while(command != 'q')
	{
		printf("Please, give the command: \n r\t - for read\n w \t - for read\n o \t - for read(test version) \n a \t - for write wave \n t \t - for read wave \n v \t - read given IP address \n c \t -write given IP address with given data \n s \t - for reset IP \n p - for clearing ERROR counter \n x \t - for current setting\n y \t - for reading PCI address & all regs \n n \t - for turning on the device\n d \t - dump all the IP regs\n f \t - for off the device \n q \t to EXIT\n");
		
		scanf("%c",&command);
		cos=getchar();
		if(command == 'q')
			return 0;
		if(command != 's'  && command != 'y' && command != 'n' && command != 'f' && command != '1' && command != '2' && command != '3' && command != 'd' && command != 'p')
		{
			printf("Please, give the address of Power Supply\n");	
		//gets(addr);
		//cos = getchar();
	
			scanf("%s", addr);
			cos=getchar();
		}
		if (command == 'n')
		{
			printf("Turning on the device ............. \n");
			val.chan = atoi(argv[2]);
			val.stat = 0xc0; /* reading status register */ 
			val.address = 0x3c;
			val.data = 0x00000001;
			if ((err = ioctl(fd, PSCIP_WRITE, &val)))
			{
				printf("Ioctl error: %s, err_type=%d\n",pscip_get_errmsg(err),err);
				//return 0;
				
			}
			else
				printf("The device should be on - the LED should be green, if it's not..... it's not good\n");
			
		}else if (command == 'd')
		{
			val.chan = atoi(argv[2]);	
			printf("Choosen chanal:  %d\n",val.chan );
			if ((err = ioctl(fd, PSCIP_DUMPFPGAREGS, &val)))
			{
				printf("Ioctl error: %s, err_type=%d\n",pscip_get_errmsg(err),err);
				//return 0;
				
			}
				
		}

		//read register 0x24 in PSC
		else if (command == '1')
		{
			printf("reading address 0x24 ............. \n");
			val.chan = atoi(argv[2]);
			val.stat = 0x40; /* reading status register */ 
			val.address = 0x24;
			val.data = 0;
			if ((err = ioctl(fd, PSCIP_READ, &val)))
			{
				printf("Ioctl error: %s, err_type=%d\n",pscip_get_errmsg(err),err);
				//return 0;
				
			}
			else
				printf("The device should be off - the LED should go off \n");
			printf("the value of the register with address 0x%x is: %d (0x%x)\n",val.address, val.data,val.data);
		}
		//read reg 0x3b in PSC
		else if (command == '2')
		{
			printf("reading address 0x3b ............. \n");
			val.chan = atoi(argv[2]);
			val.stat = 0x40; /* reading status register */ 
			val.address = 0x3b;
			val.data = 0;
			if ((err = ioctl(fd, PSCIP_READ, &val)))
			{
				printf("Ioctl error: %s, err_type=%d\n",pscip_get_errmsg(err),err);
				//return 0;
				
			}
			else
				printf("The device should be off - the LED should go off \n");
			printf("the value of the register with address 0x%x is: %d (0x%x)\n",val.address, val.data,val.data);
		}
		//read register 0x3c in PSC
		else if (command == '3')
		{
			printf("reading address 0x3c............. \n");
			val.chan = atoi(argv[2]);
			val.stat = 0x40; /* reading status register */ 
			val.address = 0x3c;
			val.data = 0;
			if ((err = ioctl(fd, PSCIP_READ, &val)))
			{
				printf("Ioctl error: %s, err_type=%d\n",pscip_get_errmsg(err),err);
				//return 0;
				
			}
			else
				printf("The device should be off - the LED should go off \n");
			printf("the value of the register with address 0x%x is: %d (0x%x) \n",val.address, val.data,val.data);
		}
		//turn OFF fast converter
		else if (command == 'f')
		{
			printf("Turning off the device ............. \n");
			val.chan = atoi(argv[2]);
			val.stat = 0xc0; /* reading status register */ 
			val.address = 0x3c;
			val.data = 0;
			if ((err = ioctl(fd, PSCIP_WRITE, &val)))
			{
				printf("Ioctl error: %s, err_type=%d\n",pscip_get_errmsg(err),err);
				//return 0;
				
			}
			else
				printf("The device should be off - the LED should go off \n");
			
		}
		//read data from PSC
		else if (command == 'r') 		//read
		{
			printf("Reading ............. \n");
			printf("Please, give the data format: \n i\t - for integer\n x \t - for hex \n f \t -for float \n");
			//format = getchar();
			scanf("%c",&format);
			cos=getchar();
			while(1) 
			{
				
				val.chan = atoi(argv[2]);
				val.stat = 0x40; /* reading status register */ 
				sscanf(addr, "%2x", &tmp);
				printf("given addres: %3x\n", tmp);
				val.address = tmp;
				//val.address = 0x9C;
			
				if ((err = ioctl(fd, PSCIP_READ, &val)))
				{
					printf("Ioctl error: %s, err_type=%d\n",pscip_get_errmsg(err),err);
					//return 0;
					break;
				}	
				else 
				{
					if (format == 'i') {
						printf("value addr(0x%x) = %d\n",val.address,val.data);
					}
					else if (format == 'x') {	
						printf("value addr(0x%x) = 0x%x\n",val.address,val.data);
					}
					else if (format == 'f') {
						printf("value addr(0x%x) = %f\n",val.address,*(float*)(&val.data));
					}
					else {
						printf("Error: invalid data type option (i,x,f)\n");
						break;	
					}
				}
				
					usleep(10000);	
					break;
			}
			
		}
		//read data from PSC (test version)
		else if (command == 'o') 		//read
		{
			printf("Reading (test)............. \n");

			printf("Please, give data to be sent to PS\n");
			//gets(data);
			//cos = getchar();
			scanf("%s", data);
			cos=getchar();
		
			sscanf(data, "%8x", &tmp);
			val.data = tmp;
			printf("given data: %x\n",val.data);

			printf("Please, give the data format: \n i\t - for integer\n x \t - for hex \n f \t -for float \n");
			//format = getchar();
			scanf("%c",&format);
			cos=getchar();
			while(1) 
			{
				
				val.chan = atoi(argv[2]);
				val.stat = 0x40; /* reading status register */ 
				sscanf(addr, "%2x", &tmp);
				printf("given addres: %3x\n", tmp);
				val.address = tmp;
				//val.address = 0x9C;
				//val.data = 0x12345678;
			
				if ((err = ioctl(fd, PSCIP_RDTEST, &val)))
				{
					printf("Ioctl error: %s, err_type=%d\n",pscip_get_errmsg(err),err);
					//return 0;
					break;
				}	
				else 
				{
					if (format == 'i') {
						printf("value addr(0x%x) = %d\n",val.address,val.data);
					}
					else if (format == 'x') {	
						printf("value addr(0x%x) = 0x%x\n",val.address,val.data);
					}
					else if (format == 'f') {
						printf("value addr(0x%x) = %f\n",val.address,*(float*)(&val.data));
					}
					else {
						printf("Error: invalid data type option (i,x,f)\n");
						break;	
					}
				}
				
					usleep(10000);	
					break;
			}
			
		}
		//write data to PSC
		else if (command == 'w') 
		{
			printf("Writing ............. \n");
			printf("Please, give data to be sent to PS\n");
			//gets(data);
			//cos = getchar();
			scanf("%s", data);
			cos=getchar();
			printf("Please, give the data format: \n i\t - for integer\n x \t - for hex \n f \t -for float \n");
			scanf("%c",&format);
			cos=getchar();
			//format = getchar();
			val.chan = atoi(argv[2]);		
			
			/*sscanf(argv[7], "%3x", &tmp);
			val.stat = tmp;*/
			val.stat = 0xc0; /* writing status register */ 
			sscanf(addr, "%2x", &tmp);
			val.address = tmp;	
			//val.address = 0x9C;
			
			if (format == 'i') 
			{
				val.data = atoi(data);
				printf("given data: %d\n",val.data);
			}
			else if (format == 'x') 
			{	
				sscanf(data, "%8x", &tmp);
				val.data = tmp;
				printf("given data: %x\n",val.data);
			}
			else if (format == 'f') 
			{
				ftmp = atof(data);
				val.data = *(int *) (&ftmp);
				printf("given data: %f\n",val.data);
			}
			else 
			{
				printf("Error: invalid data type option (i,x,f)\n");
				//break;	
			}

			if (err = ioctl(fd,  PSCIP_WRITE, &val)) 
			{ /* PSCIP_WRITE , PSCIP_WRHIPRIO */
				printf("Ioctl error: %s, err_type=%d\n",pscip_get_errmsg(err),-err-PSCIP_ERR_OFFSET);
				//break;
			}	
			usleep(10000);	
						
		}
		//write wave to PSC
		else if (command == 'a') 
		{	
			printf("Writing wave............. \n");
			printf("Please, give data to be sent to PS\n");
			//gets(data);
			//cos = getchar();
			scanf("%s", data);
			cos=getchar();

			printf("Please, give the data format: \n i\t - for integer\n x \t - for hex \n f \t -for float \n");
			//format = getchar();
			scanf("%c",&format);
			cos=getchar();

			val.chan = atoi(argv[2]);		
			
			/*sscanf(argv[7], "%3x", &tmp);
			val.stat = tmp;*/

			val.stat = 0xC0; // writing status register 
			sscanf(addr, "%2x", &tmp);
			val.address = tmp;	
			//val.address = 0x9C;

			if (format == 'i') 
			{
				val.data = atoi(data);
				printf("given data: %d\n",val.data);
			}
			else if (format == 'x') 
			{	
				sscanf(data, "%8x", &tmp);
				val.data = tmp;
				printf("given data: %x\n",val.data);
			}
			else if (format == 'f') 
			{
				ftmp = atof(data);
				val.data = *(int *) (&ftmp);
				printf("given data: %f\n",val.data);
			}
			else 
			{
				printf("Error: invalid data type option (i,x,f)\n");
				//break;	
			}

			if (err = ioctl(fd,  PSCIP_WRWAVE, &val)) { /* PSCIP_WRITE , PSCIP_WRHIPRIO */
				printf("Ioctl error: %s, err_type=%d\n",pscip_get_errmsg(err),-err-PSCIP_ERR_OFFSET);
				//break;
			}	
			usleep(100000);	
						
		}
		//write data to register in IP
		else if (command == 'c') 
		{	
			printf("Writing byte to address in IP............. \n");
			printf("Please, give data to be sent to PS\n");
			//gets(data);
			//cos = getchar();
			scanf("%s", data);
			cos=getchar();
	
			sscanf(data, "%2x", &tmp);
			val.stat = tmp;
			printf("given data: %x\n",val.stat);


			val.chan = atoi(argv[2]);		
			

			sscanf(addr, "%2x", &tmp);
			val.address = tmp;	

			if (err = ioctl(fd,  PSCIP_WRITEIPREG, &val)) { /* PSCIP_WRITE , PSCIP_WRHIPRIO */
				printf("Ioctl error: %s, err_type=%d\n",pscip_get_errmsg(err),-err-PSCIP_ERR_OFFSET);
				//break;
			}	
			usleep(100000);	
						
		}
		//read wave
		else if (command == 't') 
		{
			printf("Reading wave............ \n");
			

			printf("Please, give the data format: \n i\t - for integer\n x \t - for hex \n f \t -for float \n");
			//format = getchar();
			scanf("%c",&format);
			cos=getchar();
			val.chan = atoi(argv[2]);

			
		
			val.stat = 0x40; /* reading status register */ 
			sscanf(addr, "%2x", &tmp);
			val.address = tmp;
			//val.address = 0x9C;
		
			if ((err = ioctl(fd, PSCIP_RDWAVE, &val))) 
			{
				printf("Ioctl error: %s, err_type=%d\n",pscip_get_errmsg(err),err);
				//break;
			}	
			else 
			{
				if (format == 'i') 
				{
					printf("value addr(0x%x) = %d\n",val.address,val.data);
				}
				else if (format == 'x') 
				{	
					printf("value addr(0x%x) = 0x%x\n",val.address,val.data);
				}
				else if (format == 'f') 
				{
					printf("value addr(0x%x) = %f\n",val.address,*(float*)(&val.data));
				}
				else 
				{
					printf("Error: invalid data type option (i,x,f)\n");
					//break;	
				}
			}

			usleep(100000);	
			
		}
		//read IP register
		else if (command == 'v') 
		{
			printf("Reading IP register............ \n");
			

			
			format='x';
		
			val.stat = 0x40; /* reading status register */ 
			sscanf(addr, "%2x", &tmp);
			val.address = tmp;

		
			if ((err = ioctl(fd, PSCIP_READIPREG, &val))) 
			{
				printf("Ioctl error: %s, err_type=%d\n",pscip_get_errmsg(err),err);
				//break;
			}	

			printf("value addr(0x%x) = 0x%x\n",val.address,val.stat);


	
			
		}
		// reset IP 
		else if (command == 's') 
		{	
			printf("Reseting IP........... \n");
			val.chan = atoi(argv[2]);	
			if ((err = ioctl(fd, PSCIP_RESET))) 
			{
				printf("Ioctl error: %s, err_type=%d\n",pscip_get_errmsg(err),err);
				//break;
			}	
		
		}
		//reset FPGA error counter
		else if (command == 'p') 
		{	
			printf("Reseting IP........... \n");
			val.chan = atoi(argv[2]);	
			if ((err = ioctl(fd, PSCIP_CLRCOUNTER, &val))) 
			{
				printf("Ioctl error: %s, err_type=%d\n",pscip_get_errmsg(err),err);
				//break;
			}	
		
		}
		//set current
		else if (command == 'x') 
		{	
			printf("Setting current ............. \n");
			printf("Please, give the current to be set \n");

			scanf("%s", data);
			cos=getchar();
			val.chan = atoi(argv[2]);		
			
			sscanf(addr, "%2x", &tmp);
			val.stat = 0xc0; /* writing status register */ 
			val.address = tmp;	

			
			val.data = atoi(data);
			printf("given data: %d\n",val.data);
			
			if (err = ioctl(fd,  PSCIP_WRHIPRIO, &val)) 
			{ /* PSCIP_WRITE , PSCIP_WRHIPRIO */
				printf("Ioctl error: %s, err_type=%d\n",pscip_get_errmsg(err),-err-PSCIP_ERR_OFFSET);
				//break;
			}	
			usleep(10000);	
						
		}
		//read PCI virtual base address and dump all the info if in DEBUG mode
		else if (command == 'y') 
		{	
			printf("Reading PCIaddres........... \n");
			printf("----------------------------------------------- N O T E -----------------------------------------\n");	
			printf("----- 		if in DEBUG mode (#define DEGUG in pscip.c) this function read all registers   	-----\n");
			printf("-----	from the Carrier and IP and prints them to linux log file ($cat /var/log/messages)	-----\n");
			printf("-------------------------------------------------------------------------------------------------\n");
			
			val.chan = atoi(argv[2]);	
			if ((err = ioctl(fd, PSCIP_PCIADDR, &pciaddr))) 
			{
				printf("Ioctl error: %s, err_type=%d\n",pscip_get_errmsg(err),err);
				//break;
			}	
			printf("PCI Address: 0x%x\n", pciaddr);
		}
		
		else 
		{
			printf("Error: invalid action type (r/w/a/t/x/s/q/y)\n");
			//return 0;
		}
	}

	close(fd);
	return 0;
		
}
