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

#define DEF_TESTS_NUMBER 	1000
#define DEF_REF_DEVNUMBER 	0
#define DEF_TEST_DEVNUMBER	1
#define DELAY 			100

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
int delay_us;
int fd_ref,fd_test,i;
int verbose_mode = 0;
int number_of_tests = 0;
char read_byte_from_IP_address(int fd, char address, int chan);
int read_word(int fd, pscip_t *val);
int perform_write_word(int fd, pscip_t *val);
int perform_read_word(int fd, pscip_t *val);
int test_data_correctness(pscip_t val_ref,pscip_t val_test);
int connect_to_driver(int argc, char *argv[]);
int test_write_IP(int fd_writing_IP,int fd_reading_IP, int chan);
int test_read_IP(int fd_reading_IP,int fd_testing_IP, int chan);
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
	int ref_to_test_write_result_ch0;
	int ref_to_test_write_result_ch1;
	int test_to_ref_write_result_ch0;
	int test_to_ref_write_result_ch1;

	int ref_to_test_read_result_ch0;
	int ref_to_test_read_result_ch1;
	int test_to_ref_read_result_ch0;
	int test_to_ref_read_result_ch1;

	int IP_ch0;
	int IP_ch1;

	delay_us = DELAY;

	if(connect_to_driver(argc, argv)==-1)
		return 0;
//////////////////////// TEST /////////////////////////////


///////////// write
	printf("\n2) Writing random data from reference IP to tested IP on channal 0 : (%d times ) :\n", number_of_tests);	
	printf("\n                      ............\n");
	ref_to_test_write_result_ch0 = test_write_IP(fd_ref,fd_test,0);
	
	printf("\n3) Writing random data from reference IP to tested IP on channal 1 : (%d times ) :\n", number_of_tests);	
	printf("\n                      ............\n");
	ref_to_test_write_result_ch1 = test_write_IP(fd_ref,fd_test,1);

	printf("\n4) Writing random data from tested IP to reference IP on channal 0 : (%d times ) :\n", number_of_tests);	
	printf("\n                      ............\n");
	test_to_ref_write_result_ch0 = test_write_IP(fd_test, fd_ref,0);
	
	printf("\n5) Writing random data from tested IP to reference IP on channal 1 : (%d times ) :\n", number_of_tests);	
	printf("\n                      ............\n");
	test_to_ref_write_result_ch1 = test_write_IP(fd_test,fd_ref,1);

///////////// reading
	printf("\n6) Reading random data from tested IP on channal 0 : (%d times ) :\n", number_of_tests);	
	printf("\n                      ............\n");
	ref_to_test_read_result_ch0 = test_read_IP(fd_ref,fd_test,0);
	
	printf("\n7) Reading random data from tested IP on channal 1 : (%d times ) :\n", number_of_tests);	
	printf("\n                      ............\n");
	ref_to_test_read_result_ch1 = test_read_IP(fd_ref,fd_test,1);

	printf("\n8) Reading random data reference IP on channal 0 : (%d times ) :\n", number_of_tests);	
	printf("\n                      ............\n");
	test_to_ref_read_result_ch0 = test_read_IP(fd_test, fd_ref,0);
	
	printf("\n9) Reading random data reference IP on channal 1 : (%d times ) :\n", number_of_tests);	
	printf("\n                      ............\n");
	test_to_ref_read_result_ch1 = test_read_IP(fd_test,fd_ref,1);

///////////////////////// TEST OUTCOME//////////////////////////////
	printf("\n\n");
	printf("\t==========================================\n");	
	printf("\t	     IP TEST SUMMARY		\n");
	printf("\t==========================================\n");
	printf("\n");


	printf("\tWriting to tested IP on chan 0: ");
	if(ref_to_test_write_result_ch0 != 0)
		printf("FAILED\n");
	else
		printf("OK\n");


	printf("\tWriting by tested IP on chan 0: ");
	if(test_to_ref_write_result_ch0 != 0)
		printf("FAILED\n");
	else
		printf("OK\n");


	printf("\tWriting to tested IP on chan 1: ");
	if(ref_to_test_write_result_ch1 != 0)
		printf("FAILED\n");
	else
		printf("OK\n");


	printf("\tWriting by tested IP on chan 1: ");
	if(test_to_ref_write_result_ch1 != 0)
		printf("FAILED\n");
	else
		printf("OK\n");

	printf("\n");
	printf("\t------------------------------------------\n");
	printf("\n");

	printf("\tReading to tested IP on chan 0: ");
	if(ref_to_test_read_result_ch0 != 0)
		printf("FAILED\n");
	else
		printf("OK\n");


	printf("\tReading by tested IP on chan 0: ");
	if(test_to_ref_read_result_ch0 != 0)
		printf("FAILED\n");
	else
		printf("OK\n");


	printf("\tReading to tested IP on chan 1: ");
	if(ref_to_test_read_result_ch1 != 0)
		printf("FAILED\n");
	else
		printf("OK\n");


	printf("\tReading by tested IP on chan 1: ");
	if(test_to_ref_read_result_ch1 != 0)
		printf("FAILED\n");
	else
		printf("OK\n");

	printf("\n");
	printf("\t------------------------------------------\n");
	printf("\n");

	if(!ref_to_test_write_result_ch0 && !test_to_ref_write_result_ch0 && !ref_to_test_read_result_ch0 && !test_to_ref_read_result_ch0)
	{
		printf("\tChannal 0 - OK\n");
		IP_ch0 = 0;
	}
	else
	{
		printf("\tChannal 0 - FAILED error detected\n");
		IP_ch0 = -1;
	}
	if(!ref_to_test_write_result_ch1 && !test_to_ref_write_result_ch1 && !ref_to_test_read_result_ch1 && !test_to_ref_read_result_ch1)
	{
		printf("\tChannal 0 - OK\n");
		IP_ch1 = 0;
	}
	else
	{
		printf("\tChannal 1 - FAILED, error detected\n");
		IP_ch1 = 1;
	}

	printf("\n");
	printf("\t------------------------------------------\n");
	printf("\n");

	if(!IP_ch0 && !IP_ch1)
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
	close(fd_ref);
	close(fd_test);
	return 0;
		
}
int test_write_IP(int fd_writing_IP,int fd_reading_IP, int chan)
{
	pscip_t val_ref;
	pscip_t val_test;
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
//========================================================
	val_test.chan = chan;
	val_ref.chan = chan;
	val_ref.stat = status; /* reading status register */ 
	val_ref.address = address;
	val_ref.data = data;
		
	perform_write_word(fd_writing_IP, &val_ref);
	usleep(delay_us);		
	read_word(fd_reading_IP, &val_test);
//========================================================

	if(verbose_mode != 0)	
	{
		printf("====================================================================\n");
		printf("      reference IP     |       test IP             |  correctness  |\n");	
	}
	for(i = 0 ; i < number_of_test; i++)	
	{
		val_test.chan = chan;
		val_ref.chan = chan;
		val_ref.stat = status; /* reading status register */ 
		val_ref.address = address;
		val_ref.data = data;
		
		perform_write_word(fd_writing_IP, &val_ref);
		usleep(delay_us);
		read_word(fd_reading_IP, &val_test);
		err = test_data_correctness(val_ref,val_test);
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
int test_read_IP(int fd_reading_IP,int fd_testing_IP, int chan)
{
	pscip_t val_ref;
	pscip_t val_test;
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

//==============================================================
	val_test.chan = chan;
	val_ref.chan = chan;
	val_ref.stat = status; /* reading status register */ 
	val_ref.address = address;
	val_ref.data = data;
		
	perform_read_word(fd_reading_IP, &val_ref);
	usleep(delay_us);
	read_word(fd_testing_IP, &val_test);
//================================================================
	if(verbose_mode != 0)	
	{
		printf("====================================================================\n");
		printf("      reference IP     |       test IP             |  correctness  |\n");	
	}
	for(i = 0 ; i < number_of_test; i++)	
	{
		val_test.chan = chan;
		val_ref.chan = chan;
		val_ref.stat = status; /* reading status register */ 
		val_ref.address = address;
		val_ref.data = data;
		
		perform_read_word(fd_reading_IP, &val_ref);
		usleep(delay_us);
		read_word(fd_testing_IP, &val_test);
		err = test_data_correctness(val_ref,val_test);
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
		address	= 0xFF - (int)( i * (float)(0xFF/((float)number_of_test+1)));;
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
	int ref_devnumber;
	int test_devnumber;
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
	if (argc == 5)
	{
		if(!strcmp(argv[3],"-n"))
		{
			number_of_tests =  atoi(argv[4]);
			verbose_mode = 0;
		}
		else if(!strcmp(argv[3],"-nv") || !strcmp(argv[3],"-vn"))
		{
			verbose_mode = 1;
			number_of_tests =  atoi(argv[4]);
		}
		else
		{
			wrong_input_arg = 1;
		}		
		ref_devnumber = atoi(argv[1]);
		test_devnumber = atoi(argv[2]);
	} 
	else if (argc == 4)
	{
		if(!strcmp(argv[3],"-v"))
		{
			verbose_mode = 1;
		}
		else
		{
			wrong_input_arg = 1;
		}
		number_of_tests = DEF_TESTS_NUMBER;
		ref_devnumber = atoi(argv[1]);
		test_devnumber = atoi(argv[2]);
		
	}
	else if (argc == 3)
	{
		if(!strcmp(argv[1],"-n"))
		{
			verbose_mode = 0;
			number_of_tests =  atoi(argv[2]);
		}
		else if(!strcmp(argv[1],"-nv") || !strcmp(argv[1],"-vn"))
		{
			verbose_mode = 1;
			number_of_tests =  atoi(argv[2]);
		}
		else
		{
			wrong_input_arg = 1;
		}

		ref_devnumber = DEF_REF_DEVNUMBER;
		test_devnumber = DEF_TEST_DEVNUMBER;
		
	}
	else if (argc == 2)
	{
		if(!strcmp(argv[1],"-h") || !strcmp(argv[1],"-?"))
		{
			info(argc, argv);
			return -1;
		}
		else if(!strcmp(argv[1],"-v"))
		{
			verbose_mode = 1;
		}
		else
		{
			wrong_input_arg = 1;
		}
		number_of_tests = DEF_TESTS_NUMBER;
		ref_devnumber = DEF_REF_DEVNUMBER;
		test_devnumber = DEF_TEST_DEVNUMBER;
	}
	else
	{
		verbose_mode = 0;
		number_of_tests = DEF_TESTS_NUMBER;
		ref_devnumber = DEF_REF_DEVNUMBER;
		test_devnumber = DEF_TEST_DEVNUMBER;
		
	}

	if (wrong_input_arg==1) 
	{

		info(argc, argv);

		return -1;
	}

	sprintf(ref_devname,"/dev/pscip%d",ref_devnumber);
	sprintf(test_devname,"/dev/pscip%d",test_devnumber);
	printf("\n\n\n");
	printf("\t==========================================\n");	
	printf("\tThis is a program to test IP communication\n");
	printf("\t==========================================\n");

	printf("\n\n1) Openning the IPs:\n");		
	if ((fd_ref = open(ref_devname, O_RDWR)) < 0) 
	{
		printf("Cannot open reference /dev/%s\n",ref_devname);
		return -1;
	}	 
	else
	{
		printf("\t%s has been openned sucessfully as a reference IP\n", ref_devname);
		test_time = (number_of_tests * (delay_us + 6000) * 8 )/1000000;
		printf("\t=----------------------------------------=\n");	
		printf("\t= > Estimated time of test: %d seconds < =\n",test_time );
		printf("\t=----------------------------------------=\n");
	}		 

	if ((fd_test = open(test_devname, O_RDWR)) < 0) 
	{
		printf("Cannot open test /dev/%s\n",test_devname);
		return -1;
	}	 
	else
	{
		printf("\t%s has been openned sucessfully as the IP to be teste\n", test_devname);
	}	
	return 0;
}
void info(int argc, char *argv[])
{
	printf("\n");
	printf("Usage: %s [<ref_devidx(0,1..20)> <test_devidx(0,1..20)>] [-v | -n <number>| -vn <number>] \n",argv[0]);
	printf("\n");
	printf("-n <number>             defines number of repetitions of operation of reading/writing\n");
	printf("-v                      defines verbose mode - writen/read data is printed out\n");
	printf("-<def_devidx(0,1..20)>  number of reference IP,ex if 0 chosen, /dev/pscip0 is openned\n");
	printf("-<test_devidx(0,1..20)> number of test IP,ex if 0 chosen, /dev/pscip0 is openned\n");
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
