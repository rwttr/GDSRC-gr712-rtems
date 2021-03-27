/*
Send and Receive String on UART2 port of GR712RC Board

on linux terminal

set serial port baud rate
stty -F /dev/ttyUSB0 38400

send data
echo -e 'hello world' > /dev/ttyUSB3

receive data
cat /dev/ttyUSB3


*/
#include <rtems.h>
/* configuration information */
#define CONFIGURE_INIT

#include <bsp.h> /* for device driver prototypes */

rtems_task Init( rtems_task_argument argument);	/* forward declaration needed */

/* configuration information : config rtems and rtems driver */
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_MAXIMUM_TASKS             4
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_EXTRA_TASK_STACKS         (3 * RTEMS_MINIMUM_STACK_SIZE)
#define CONFIGURE_MAXIMUM_FILE_DESCRIPTORS	32

#include <rtems/confdefs.h>

/* config GRLIB driver */
/* Add Timer and UART Driver */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GPTIMER
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_APBUART

// #include <drvmgr/ambapp_bus.h>
#include <grlib/ambapp_bus.h>

/* APBUART0 */
struct drvmgr_key grlib_drv_res_apbuart0[] =
{
  {"mode",   DRVMGR_KT_INT, {(unsigned int)1}},
  {"syscon", DRVMGR_KT_INT, {(unsigned int)1}},
  DRVMGR_KEY_EMPTY
};
/* APBUART1 */
struct drvmgr_key grlib_drv_res_apbuart1[] =
{
  {"mode",   DRVMGR_KT_INT, {(unsigned int)1}},
  {"syscon", DRVMGR_KT_INT, {(unsigned int)0}},
  DRVMGR_KEY_EMPTY
};

/* APBUART2 : as default configuration */
struct drvmgr_key grlib_drv_res_apbuart2[] =
{
  {"mode",   DRVMGR_KT_INT, {(unsigned int)0}},
  {"syscon", DRVMGR_KT_INT, {(unsigned int)0}},
  DRVMGR_KEY_EMPTY
};

/* Register driver */
struct drvmgr_bus_res grlib_drv_resources =
{
  .next = NULL,
  .resource = {
    {DRIVER_AMBAPP_GAISLER_APBUART_ID, 0, &grlib_drv_res_apbuart0[0]},
    {DRIVER_AMBAPP_GAISLER_APBUART_ID, 1, &grlib_drv_res_apbuart1[0]},
    {DRIVER_AMBAPP_GAISLER_APBUART_ID, 2, &grlib_drv_res_apbuart2[0]},
    DRVMGR_RES_EMPTY
  }
};


#include <drvmgr/drvmgr_confdefs.h>

/*  end of driver config */
#include <rtems/bspIo.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define printf printk

rtems_task Init(  rtems_task_argument ignored )
{
	printk("task init\r\n");

	int fd;
	int result;

	fd = open("/dev/console_b", O_RDWR);
	if(fd<0)
		printk("open port 2 error\n");

	struct termios term;
	tcgetattr(fd, &term);

	/* Set Console baud to 38400, default is 38400 */
	cfsetospeed(&term, B38400);
	cfsetispeed(&term, B38400);


	/* Do not echo chars from input*/
	//term.c_lflag &= ~(ICANON|ECHO|ECHOE|ECHOK|ECHONL|ECHOPRT|ECHOCTL|ECHOKE);

	/* temios c_cflag control field supported by APBUART Console Driver */
	term.c_cflag &= ~CSIZE;
	term.c_cflag |= CS8;
	term.c_cflag &= ~PARENB;

	  /* Update driver's settings */
	tcsetattr(fd, TCSANOW, &term);
	fflush(NULL);

	char buffer[20] = "Hello World\n";
	result = write(fd, buffer, 20); //&buffer[0]
	rtems_task_wake_after(10);
	fflush(NULL); // flush buffer

	if(result<0){
		printk("write fail\n");
	}
	else{
		printk("Write Success with data size %d Byte",result);
	}

	//close(fd); // close file descriptor
	//fflush(NULL);
	tcflush(fd,TCIOFLUSH); // flush serial buffer

	char bufferRx[20];
	memset((void *)bufferRx, 0, 20);

	int readlen = read(fd, bufferRx, 16);
	if(readlen<0){
		printk("read error: %s\n", strerror(errno));
		close(fd);
		exit(0);
	}
	else{
		printk("Read Complete with %d Bytes received\n",readlen);
		printf("Read Data: %s \n",bufferRx);
	}

	tcflush(fd,TCIOFLUSH);


	close(fd);

	exit(0);
}
