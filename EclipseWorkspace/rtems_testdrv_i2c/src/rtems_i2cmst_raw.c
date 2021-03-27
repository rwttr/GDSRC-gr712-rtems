/*
 * rtems_i2cmst_raw.c
 *
 *  Created on: Mar 23, 2021
 *      Author: Rattachai
 */

#include <rtems.h>
#include <assert.h>
#include <string.h>

#define CONFIGURE_INIT
#include <bsp.h>

rtems_task Init( rtems_task_argument argument);

/* configuration information */
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_NULL_DRIVER 1
#define CONFIGURE_MAXIMUM_TASKS             8
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_EXTRA_TASK_STACKS         (3 * RTEMS_MINIMUM_STACK_SIZE)
#define CONFIGURE_MAXIMUM_FILE_DESCRIPTORS 16
#define CONFIGURE_INIT_TASK_PRIORITY	100
#define CONFIGURE_USE_IMFS_AS_BASE_FILESYSTEM

#define CONFIGURE_MAXIMUM_SEMAPHORES 32
#define CONFIGURE_INIT_TASK_ATTRIBUTES RTEMS_FLOATING_POINT

#include <rtems/confdefs.h>

#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GPTIMER
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_APBUART
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_I2CMST

/* Configure Driver Manager */
#include <drvmgr/drvmgr_confdefs.h>

#define printf printk

#include <rtems/libi2c.h>
//#include <libchip/i2c-2b-eeprom.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <grlib/i2cmst.h>

/* **** Test configuration **** */
/* Address of EEPROM present in system: */
#define EEPROM_ADDR 0x54
#define I2CMST_MINOR 1

/* **** End of test configuration **** */

rtems_task task1(rtems_task_argument argument);

rtems_id   Task_id[3];         /* array of task ids */
rtems_name Task_name[3];       /* array of task names */

rtems_task Init(rtems_task_argument ignored)
{
  rtems_status_code status;
  //drvmgr_print_topo();
  //drvmgr_print_devs(PRINT_DEVS_ALL);
  //drvmgr_print_devs(PRINT_DEVS_ALL);
  /*drvmgr_print_topo();	*/

  printf("******** Starting Gaisler I2CMST test ********\n");

	/* I2C MASTER has been initialized by the driver manager */
  Task_name[1] = rtems_build_name( 'T', 'S', 'K', 'A' );

  status = rtems_task_create(
			     Task_name[1], 1, RTEMS_MINIMUM_STACK_SIZE * 2,
			     RTEMS_DEFAULT_MODES,
			     RTEMS_FLOATING_POINT, &Task_id[1]
			     );
  assert(status == RTEMS_SUCCESSFUL);

  status = rtems_task_start(Task_id[1], task1, 1);
  assert(status == RTEMS_SUCCESSFUL);

  status = rtems_task_delete(RTEMS_SELF);
  assert(status == RTEMS_SUCCESSFUL);

  exit(0);
}

/* Helper functions */

/* printbuf: Produces formatted printout of 'len' bytes from buffer 'buf' */
void printbuf(unsigned char *buf, int len, int saddr)
{
  int i;

  for (i = 0; i < len; i++) {
    if (i % 4 == 0)
      printf("0x%02X:", saddr + i);
    printf("\t%x%s", buf[i], (i+1) % 4 == 0 ? "\n" : "");
  }
}

void printbuf_char(unsigned char *buf, int len, int saddr)
{
  int i;

  for (i = 0; i < len; i++) {
    if (i % 4 == 0)
      printf("0x%02X:", saddr + i);
    printf("\t%c%s", buf[i], (i+1) % 4 == 0 ? "\n" : "");
  }
}

/* ver_read: Verbose read, checks return value of read() call */
void ver_read(int fd, void *buf, size_t count)
{
  int status;

  status = read(fd, buf, count);
  if (status < 0) {
    printf("read() call returned with error: %s\n",
	   strerror(errno));

  } else if (status < count) {
    printf("\nread() did not return with all requested bytes\n"
	   "requested bytes: %d, returned bytes: %d\n\n",
	   count, status);
  }
}

void ver_write(int fd, const void *buf, size_t count)
{
  int status;

  status = write(fd, buf, count);
  if (status < 0) {
    printf("write() call returned with error: %s\n", strerror(errno));
  }
  else if (status < count) {
    printf("write() did not output all bytes\n"
	   "requested bytes: %d, returned bytes: %d\n\n", count, status);
  }
}

/* ver_open: Checks return value of open and exits on failure */
int ver_open(const char *pathname, int flags)
{
  int fd;

  fd = open(pathname, O_RDWR);
  if (fd < 0) {
	  printf("Could not open %s: %s\n", pathname, strerror(errno));
	  exit(0);
  }
  return fd;
}

/* Test task */
int test_minor1(void)
{
	int fd;
	int i;
	unsigned char buf[100];
	char messageTx[] = "Hello World";
	int status;
	dev_t raw_dev_t;
	int messageLen = strlen(messageTx)+1;
	/* We will create a /dev entry for the raw driver below and name it: */
	char *raw_dev = "/dev/i2c" "I2CMST_MINOR" "-50";

	printf("Starting test task...\n");

	/* Create node for 'raw' access to EEPROM device */
	raw_dev_t = rtems_filesystem_make_dev_t(rtems_libi2c_major,
				      RTEMS_LIBI2C_MAKE_MINOR(I2CMST_MINOR-1,EEPROM_ADDR));

	status = mknod(raw_dev, S_IFCHR | 0666, raw_dev_t);

	if (status < 0) {
		printf("mknod of %s failed: %s\n", raw_dev, strerror(errno));
		exit(0);
	}

	printf("Reading from EEPROM using 'raw' driver..\n");
	fd = ver_open(raw_dev, O_RDWR);

	/* Set address */
	buf[0] = 0;
	ver_write(fd, buf, 1);

	printf("Reading 20 bytes from EEPROM\n");
	ver_read(fd, buf, 20);
	printf("Contents of first 100 bytes of EEPROM:\n");
	printbuf(buf, 20, 0);

	printf("Writing %d bytes to EEPROM,\n %s, starting at address 0x00: ",messageLen, messageTx);
	buf[0] = 0; /* First will become address */


	for (i = 1; i < messageLen; i++)
		buf[i] = messageTx[i-1];

	ver_write(fd, buf, messageLen);
	printf("done..\n");

	printf("Reading the first %d bytes of the EEPROM:\n",strlen(messageTx));
	buf[0] = 0;
	ver_write(fd, buf, 1);

	ver_read(fd, buf, messageLen);

	printbuf_char(buf, messageLen, 0);

/*	for (i = 0; i < 16; i++)
		if (buf[i] != i)
			printf("ERROR! Mismatch on byte %d\n", i);*/

	close(fd);

  	  printf("Test completed\n");

	return 0;
}



rtems_task task1(rtems_task_argument unused)
{
	if ( test_minor1() ) {
		printf("TEST1 FAILED\n");
		exit(-1);
	}

	printf("TEST OK\n");
	exit(0);
}


