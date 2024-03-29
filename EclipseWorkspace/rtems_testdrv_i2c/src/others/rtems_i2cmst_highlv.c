/*
 * rtems_i2cmst_highlv.c
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
#include <libchip/i2c-2b-eeprom.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <grlib/i2cmst.h>

/* **** Test configuration **** */
/*
   Both DO_I2C_RAWDRIVER_TEST and DO_I2C_HIGHLEVEL_TEST may be defined.
   The i2cmst driver has a DEBUG define that enables print outs of what
   is happening in the driver
*/

/* Address of EEPROM present in system: */
#define EEPROM_ADDR 0x50

/* Select I2C Master core used in example (the minor), valid range is 1..N,
 * where N is the number of cores in the system
 *
 * I2CMST_MINOR is always used
 * I2CMST_MINOR2 is used when two cores are used
 */
#define I2CMST_MINOR 1
#define I2CMST_MINOR2 2

/* **** End of test configuration **** */

rtems_task task1(rtems_task_argument argument);

rtems_id   Task_id[3];         /* array of task ids */
rtems_name Task_name[3];       /* array of task names */

rtems_task Init(rtems_task_argument ignored)
{
  rtems_status_code status;

  /*drvmgr_print_topo();
	drvmgr_print_devs(PRINT_DEVS_ALL);*/

  printf("** Starting Gaisler I2CMST test **\n");

	/* I2C MASTER has been initialized by the driver manager */

  printf("Registering EEPROM driver: ");
  status = rtems_libi2c_register_drv("eeprom", i2c_2b_eeprom_driver_descriptor,
				     I2CMST_MINOR-1, EEPROM_ADDR);
  if (status < 0) {
    printf("ERROR: Could not register EEPROM driver\n");
    exit(0);
  }else
  {
	  printf("OK\n");
  }
/*
  status = rtems_libi2c_register_drv("eeprom", i2c_2b_eeprom_driver_descriptor, I2CMST_MINOR2-1, EEPROM_ADDR);
  if (status < 0) {
    printf("ERROR: Could not register EEPROM driver\n");
    exit(0);
  }
  printf("driver registered successfully\n");
*/

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
	//rtems_status_code chdebug;

	/* The registered bus is named i2c1 in the I2CMST driver and we named
     the eeprom driver "eeprom" above. The eeprom /dev name becomes: */
	//char *eeprom_dev = "/dev/i2c" "I2CMST_MINOR" ".eeprom";
	char *eeprom_dev = "/dev/i2c1" ".eeprom";

	//printf("Changing Permission for %s\n",eeprom_dev);
	//chdebug = chmod( eeprom_dev, S_IRUSR | S_IWUSR );
	//chdebug = access( eeprom_dev, F_OK );
	//printf("chmod return code : %d\n", chdebug);

  	 printf("Starting test task...\n");

  	 /* Test with 'high level' interface */
  	 printf("Reading EEPROM using high level driver..\n");

  	 fd = ver_open(eeprom_dev, O_RDWR);
  	 //ioctl(fd, IOC_INOUT);
  	 //fchmod( fd, S_IRUSR | S_IWUSR );

  	 /* Read 100 bytes from EEPROM */
  	 printf("Reading 100 bytes from EEPROM\n");
  	 ver_read(fd, buf, 100);
  	 printf("Contents of first 100 bytes of EEPROM:\n");
  	 printbuf(buf, 100, 0);

  	 /* Write values 0 - 99 to byte positions 0 - 99 */
  	 printf("Writing the first 100 bytes with the range 0 to 99: ");
  	 for (i = 0; i < 15; i++) {
  		 buf[i] = i;
  	 }
  	 /* Reset pointer offset, EEPROM driver sets the offset to 0 at open */
  	 close(fd);

  	 fd = ver_open(eeprom_dev, O_RDWR);

  	 ver_write(fd, buf, 16);
  	 printf("done!\n");
  	 printf("Reading the first 40 bytes..\n");

  	 close(fd);

  	 fd = ver_open(eeprom_dev, O_RDWR);

  	 ver_read(fd, buf, 40);
  	 printf("Current contents of the first 40 bytes:\n");
  	 printbuf(buf, 40, 0);
  	 printf("Reading 60 bytes more..\n"); /* File position serves as address */
  	 ver_read(fd, buf, 60);
  	 printf("Contents of bytes 40 to 60:\n");
  	 printbuf(buf, 60, 40);

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


