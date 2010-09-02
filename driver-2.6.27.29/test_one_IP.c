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

#define DELAY 			100
#define DEF_TESTS_NUMBER 1000
#define DEF_IP_NUMBER 0

//	typedef struct {
//			unsigned 	int 	chan; 	 /* channel 0/1 */
//			unsigned 	char 	stat,   	/* status register */ 
//						address;  	/* address register */
//					int 	data;							
//	} pscip_t; 
/********************************************************************************
how to start the program:
$ executable <dev_ref_idx(0,1..20)> <dev_test_idx(0,1..20)>

parameter:
-	<dev_ref_idx(0,1..20)>	- 	- reference device index
-	<dev_test_idx(0,1..20)> 	- test device index

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
int fd_test,i;
int delay_us;
int verbose_mode = 0;
int number_of_tests = 0;
char read_byte_from_IP_address(int fd, char address, int chan);
int read_word(int fd, pscip_t *val);
int perform_write_word(int fd, pscip_t *val);
int perform_read_word(int fd, pscip_t *val);
int test_data_correctness(pscip_t val_ref,pscip_t val_test);
int connect_to_driver(int argc, char *argv[]);
int test_write_IP(int fd,int chan_in, int chan_out);
int test_read_IP(int fd,int chan_in, int chan_out);
void info(int argc, char *argv[]);
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
	int writing_0_to_1;
	int writing_1_to_0;
	int reading_by_0;
	int reading_by_1;

	int IP_writing;
	int IP_reading;

	delay_us = DELAY;


	if(connect_to_driver(argc, argv)==-1)
		return 0;
//////////////////////// TEST /////////////////////////////


///////////// write
	printf("\n1) Writing data from channal 0 to channal 1 (%d times) :\n",number_of_tests);	
	printf("\n                      ............\n");	
	writing_0_to_1 = test_write_IP(fd_test,0,1);
	
	printf("\n2) Writing data from channal 1 to channal 0 (%d times) :\n",number_of_tests);	
	printf("\n                      ............\n");	
	writing_1_to_0 = test_write_IP(fd_test,1,0);


///////////// reading
	printf("\n3) Reading data from using channel 0 from channel 1 (%d times) :\n",number_of_tests);		
	printf("\n                      ............\n");	
	reading_by_0 = test_read_IP(fd_test,0,1);
	
	printf("\n4) Reading data from using channel 1 from channel 0 (%d times) :\n",number_of_tests);		
	printf("\n                      ............\n");	
	reading_by_1 = test_read_IP(fd_test,1,0);


///////////////////////// TEST OUTCOME//////////////////////////////
	printf("\n\n");
	printf("\t==========================================\n");	
	printf("\t	     IP TEST SUMMARY		\n");
	printf("\t==========================================\n");
	printf("\n");


	printf("\tWriting data from ch0 to ch1: ");
	if(writing_0_to_1 != 0)
		printf("FAILED\n");
	else
		printf("OK\n");


	printf("\tWriting data from ch1 to ch0: ");
	if(writing_1_to_0 != 0)
		printf("FAILED\n");
	else
		printf("OK\n");


	printf("\tReading data from ch1 to ch0: ");
	if(reading_by_0 != 0)
		printf("FAILED\n");
	else
		printf("OK\n");


	printf("\tReading data from ch0 to ch1: ");
	if(reading_by_1 != 0)
		printf("FAILED\n");
	else
		printf("OK\n");

	
	printf("\n");
	printf("\t------------------------------------------\n");
	printf("\n");

	if(!writing_0_to_1 && !writing_1_to_0)
	{
		printf("\tWriting - OK\n");
		IP_writing = 0;
	}
	else
	{
		printf("\tWriting - FAILED, error detected \n");
		IP_writing = -1;
	}
	if(!reading_by_0 && !reading_by_1 )
	{
		printf("\tReading - OK\n");
		IP_reading = 0;
	}
	else
	{
		printf("\tReading - FAILED, error detected\n");
		IP_reading = 1;
	}

	printf("\n");
	printf("\t------------------------------------------\n");
	printf("\n");

	if(!IP_writing && !IP_reading)
	{
		printf("\t\t     TEST PASSED\n");
		printf("\t\t  Congratulations :-)\n");
	}
	else
	{
		printf("\t\t     TEST FAILED\n");
		printf("\t\t     SORRY :-(\n");
	}
	printf("\n");
	printf("\t==========================================\n");
	printf("\n");
/////////////////////// TEST END /////////////////////////////
	close(fd_test);
	return 0;
		
}
int test_write_IP(int fd,int chan_in, int chan_out)
{
	pscip_t val_ch_in;
	pscip_t val_ch_out;
	char status;
	char address;
	int data;
	int number_of_test = number_of_tests;
	int i;
	int result = 0;
	int err;
	status = 0x00;
	address = 0xFF;
	data = 0x00000000;

//==============================================================
	val_ch_out.chan = chan_out;
	val_ch_in.chan = chan_in;
	val_ch_in.stat = status; /* reading status register */ 
	val_ch_in.address = address;
	val_ch_in.data = data;
		
	perform_write_word(fd, &val_ch_in);
	usleep(delay_us);
	read_word(fd, &val_ch_out);
//=============================================================


	if(verbose_mode != 0)	
	{
		printf("====================================================================\n");
		printf("      reference IP     |       test IP             |  correctness  |\n");	
	}
	for(i = 0 ; i < number_of_test; i++)	
	{
		val_ch_out.chan = chan_out;
		val_ch_in.chan = chan_in;
		val_ch_in.stat = status; /* reading status register */ 
		val_ch_in.address = address;
		val_ch_in.data = data;
		
		perform_write_word(fd, &val_ch_in);
			
		usleep(delay_us);

		read_word(fd, &val_ch_out);
		err = test_data_correctness(val_ch_in,val_ch_out);
		if(err < 0 )
			result = err;
		if(number_of_test>=0xFFFFFFFF )
		{	
			data++;
		}
		else
		{
			data	= data	 + (0xFFFFFFFF/(number_of_test+1));
		}
		status	=(int)( i * (float)(0xFF/((float)number_of_test+1)));
		address	= 0xFF - (int)( i * (float)(0xFF/((float)number_of_test+1)));
	}
	if(verbose_mode != 0)
	{
		printf("====================================================================\n");
	}
	return result;

}
int test_read_IP(int fd,int chan_in, int chan_out)
{
	pscip_t val_ch_out;
	pscip_t val_ch_in;
	char status;
	char address;
	int data;
	int number_of_test =number_of_tests;
	int i;
	int result = 0;
	int err;
	status = 0x00;
	address = 0xFF;
	data = 0x00000000;
//============================================================================
	val_ch_out.chan = chan_out;
	val_ch_in.chan = chan_in;
	val_ch_in.stat = status; /* reading status register */ 
	val_ch_in.address = address;
	val_ch_in.data = data;
		
	perform_read_word(fd, &val_ch_in);
	usleep(delay_us);
	read_word(fd, &val_ch_out);
//===========================================================================
	
	if(verbose_mode != 0)	
	{
		printf("====================================================================\n");
		printf("      reference IP     |       test IP             |  correctness  |\n");	
	}
	for(i = 0 ; i < number_of_test; i++)	
	{
		val_ch_out.chan = chan_out;
		val_ch_in.chan = chan_in;
		val_ch_in.stat = status; /* reading status register */ 
		val_ch_in.address = address;
		val_ch_in.data = data;
		
		perform_read_word(fd, &val_ch_in);

		usleep(delay_us);

		read_word(fd, &val_ch_out);
		err = test_data_correctness(val_ch_in,val_ch_out);
		if(err < 0 )
			result = err;
		
		if(number_of_test>=0xFFFFFFFF )
		{	
			data++;
		}
		else
		{
			data	= data	 + (0xFFFFFFFF/(number_of_test+1));
		}
		status	=(int)( i * (float)(0xFF/((float)number_of_test+1)));
		address	= 0xFF - (int)( i * (float)(0xFF/((float)number_of_test+1)));
		
	}
	if(verbose_mode != 0)
	{
		printf("====================================================================\n");
	}
	return result;

}
char read_byte_from_IP_address(int fd, char address, int chan)
{
	int err;
	pscip_t val;	
	val.chan = chan;
	val.stat = 0x40; /* reading status register */ 
	val.address = address;
	
	if ((err = ioctl(fd, PSCIP_READIPREG, &val))) 
	{
//		printf("Ioctl error: %s, err_type=%d\n",pscip_get_errmsg(err),err);
	}	

	return val.stat;
}
char write_byte_to_IP_address(int fd, char address, char data)
{
	int err;
	pscip_t val;	
	val.chan = 0;
	val.stat = data; /* reading status register */ 
	val.address = address;
	
	if ((err = ioctl(fd, PSCIP_WRITEIPREG, &val))) 
	{
		return -1;
	}	

	return 0;
}
int connect_to_driver(int argc, char *argv[])//		verbose_mode = DEF_TESTS_NUMBER;
{
	char ref_devname[20];
	char test_devname[20];
	int IP_number;
	int wrong_input_arg =0;
	int test_time;
/*
	if ((argc != 4) && (argc != 3) && (argc != 5)) 
	{
		printf("Usage: %s <devidx(0,1..20)> <chan(0,1)>  \n",argv[0]);
		printf("NOTE: At this stage of development the program should be opened: %s 0 0  \n",argv[0]);
		return 0;
	}	
*/
	if (argc == 4)
	{
		if(!strcmp(argv[1],"-vnp") || !strcmp(argv[1],"-vpn")|| !strcmp(argv[1],"-pvn")|| !strcmp(argv[1],"-pnv")|| !strcmp(argv[1],"-vpn"))
		{	
			number_of_tests = atoi(argv[3]);
			IP_number = atoi(argv[2]);
			verbose_mode = 1;
		}
		if(!strcmp(argv[1],"-np") || !strcmp(argv[1],"-pn"))
		{	
			number_of_tests = atoi(argv[3]);
			IP_number = atoi(argv[2]);
			verbose_mode = 0;
		}
	}
	else if (argc == 3)
	{
		if(!strcmp(argv[1],"-n"))
		{
			number_of_tests =  atoi(argv[2]);
			IP_number = DEF_IP_NUMBER;
			verbose_mode = 0;
		}
		else if(!strcmp(argv[1],"-p"))
		{	
			IP_number = atoi(argv[2]);
			number_of_tests = DEF_TESTS_NUMBER;
			verbose_mode = 0;
		}
		else if(!strcmp(argv[1],"-vn") || !strcmp(argv[1],"-nv"))
		{	
			number_of_tests = atoi(argv[2]);
			IP_number = DEF_IP_NUMBER;
			verbose_mode = 1;
		}
		else
		{
			wrong_input_arg = 1;
		}
		
		
	} 
	else if (argc == 2)
	{
		if(!strcmp(argv[1],"-v"))
		{
			verbose_mode = 1;
		}
		else if(!strcmp(argv[1],"-h") || !strcmp(argv[1],"-?"))
		{
			info(argc,argv);
			return -1;
		}
		else
			wrong_input_arg = 1;
		IP_number = DEF_IP_NUMBER;
		number_of_tests = DEF_TESTS_NUMBER;
		
	}
	else if (argc == 1)
	{
		wrong_input_arg = 0;
		verbose_mode = 0;
		IP_number = DEF_IP_NUMBER;
		number_of_tests = DEF_TESTS_NUMBER;
		
	}
	else
	{
		wrong_input_arg = 1;
		
	}
	if (wrong_input_arg==1) 
	{
		//printf("Usage: %s [-p <devidx(0,1..20)> | -v | -n <number>] \n",argv[0]);
		info(argc,argv);
		return -1;
	}

	
	sprintf(test_devname,"/dev/pscip%d",IP_number);
	printf("\n\n\n");
	printf("\t==========================================\n");	
	printf("\tThis is a program to test IP communication\n");
	printf("\t==========================================\n");

	 

	if ((fd_test = open(test_devname, O_RDWR)) < 0) 
	{
		printf("Cannot open test /dev/%s\n",test_devname);
		return -1;
	}	 
	else
	{
		printf("\t%s has been openned sucessfully \n", test_devname);
		test_time = (number_of_tests * (delay_us + 6000 ) * 4 )/1000000;
		printf("\t=----------------------------------------=\n");	
		printf("\t= > Estimated time of test: %d seconds < =\n",test_time );
		printf("\t=----------------------------------------=\n");
	}	
	return 0;
}
void info(int argc, char *argv[])
{

	printf("\n");
	printf("Usage: %s [-p <devidx(0,1..20)> | -v | -n <number>| -vn <number> | -pn <devidx(0,1..20)> <number> -vpn <devidx(0,1..20)> <number> ] \n",argv[0]);
	printf("\n");
	printf("-n <number>             defines number of repetitions of operation of reading/writing\n");	
	printf("-v                      defines verbose mode - writen/read data is printed out\n");
	printf("-p <devidx(0,1..20)>    number of tested IP,ex if 1 chosen, /dev/pscip1 is openned, by default /dev/pscip0 is openned\n");
	printf("\n");

}
int read_word(int fd, pscip_t *val)
{
	char byte[4];


	val->address 	= read_byte_from_IP_address(fd, 0x28,val->chan);
	val->stat	= read_byte_from_IP_address(fd, 0x29,val->chan);
	byte[0] 	= read_byte_from_IP_address(fd, 0x2b,val->chan);
	byte[1] 	= read_byte_from_IP_address(fd, 0x2a,val->chan);
	byte[2] 	= read_byte_from_IP_address(fd, 0x2d,val->chan);
	byte[3] 	= read_byte_from_IP_address(fd, 0x2c,val->chan);
	val->data = 0x0;
	val->data = (0xFF & byte[3]) | ((0xFF & byte[2]) << 8) | ((0xFF & byte[1]) << 16) | ((0xFF & byte[0]) << 24);
	return 0;
}
int perform_write_word(int fd, pscip_t *val)
{
	int err;
	if ((err = ioctl(fd, PSCIP_WRITE, val)))
	{
//		printf("Ioctl error: %s, err_type=%d\n",pscip_get_errmsg(err),err);

	}
}
int perform_read_word(int fd, pscip_t *val)
{
	int err;
/*	
	write_byte_to_IP_address(fd, 0x11, val->stat);
	write_byte_to_IP_address(fd, 0x10, val->address);
	
	write_byte_to_IP_address(fd, 0x13, ((val->data) >> 16) & 0xFF);
	write_byte_to_IP_address(fd, 0x12, ((val->data) >> 24) & 0xFF);
	
	write_byte_to_IP_address(fd, 0x15, ((val->data) >> 0) & 0xFF);
	write_byte_to_IP_address(fd, 0x14, ((val->data) >> 8) & 0xFF);
*/
	
	if ((err = ioctl(fd, PSCIP_RDTEST, val)))
	{
//		printf("Ioctl error: %s, err_type=%d\n",pscip_get_errmsg(err),err);

	}

}
/**
return 
 0 -> no errors
-1 -> error detected
*/
int test_data_correctness(pscip_t val_ref,pscip_t val_test)
{
	int result = 0;
	if(verbose_mode != 0)
	{
		printf("=======================|===========================|===============|\n");
		printf("status  = 0x%8x   |   status  = 0x%8x    |  ",val_ref.stat, val_test.stat);
		if(val_test.stat == val_ref.stat)
			printf("     OK      |\n");
		else
		{
			printf("status error |\n");
			result = -1;
		}
	
		printf("address = 0x%8x   |   address = 0x%8x    |  ",val_ref.address, val_test.address);
		if(val_test.address == val_ref.address)
			printf("     OK      |\n");
		else
		{
			printf("address error|\n");
			result = -1;
		}
	
		printf("data    = 0x%8x   |   data    = 0x%8x    |  ",val_ref.data, val_test.data);
		if(val_test.data == val_ref.data)
			printf("     OK      |\n");
		else
		{
			printf("data error   |\n");
			result = -1;
		}
	}
	else
	{
		if(val_test.stat == val_ref.stat)
			result = 0;
		else
			result = -1;
		if(val_test.address == val_ref.address)
			result = 0;
		else
			result = -1;

		if(val_test.data == val_ref.data)
			result = 0;
		else
			result = -1;
	}
	
	return result;
}
