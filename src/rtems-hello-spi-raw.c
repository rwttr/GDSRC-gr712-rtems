/* SPICTRL interface example */
/* Raw Driver Version
  1. Write "Hello World RAW SPI" to slave device;
  2. Read back from slave device and print on console
*/

/* Define this to use GPIO for chip select */
/*#define USE_GPIO_SLVSEL*/

#include <rtems.h>

#define CONFIGURE_INIT
#include <bsp.h> /* for device driver prototypes */

rtems_task Init( rtems_task_argument argument);	/* forward declaration needed */

/* configure RTEMS kernel */
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_MAXIMUM_TASKS             16
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_EXTRA_TASK_STACKS         (24 * RTEMS_MINIMUM_STACK_SIZE)
#define CONFIGURE_MAXIMUM_FILE_DESCRIPTORS 32
#define CONFIGURE_INIT_TASK_PRIORITY	100
#define CONFIGURE_MAXIMUM_DRIVERS 16
#define CONFIGURE_USE_IMFS_AS_BASE_FILESYSTEM
#define CONFIGURE_INIT_TASK_ATTRIBUTES RTEMS_FLOATING_POINT

#include <rtems/confdefs.h>

/* Configure Driver Manager */
#include <drvmgr/drvmgr.h>

 /* Add Timer and UART Driver for this example */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GPTIMER
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_APBUART
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_SPICTRL
/* Option if use GPIO as spi cs pins (chip-select pin) */
#ifdef USE_GPIO_SLVSEL
  #define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRGPIO
#endif
#include <drvmgr/drvmgr_confdefs.h>
/* end of driver config */

#include <grlib/spictrl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

/* No networking */
#undef ENABLE_NETWORK_SMC_LEON3
#undef ENABLE_NETWORK_SMC_LEON2
#undef ENABLE_NETWORK

/* Option if use GPIO as spi cs pins (chip-select pin) */
#ifdef USE_GPIO_SLVSEL
#include <grlib/gpiolib.h>
/* Provide custom slave select function */
void *spi_gpio_port = NULL;
extern int spi_gpio_slvsel(void *regs, int addr, int select);
extern int spi_gpio_slvsel_setup(int port);
#define SPICTRL_SLV_SEL_FUNC spi_gpio_slvsel
#define GPIO_PORT_NR 5
#endif

/* Include driver configurations and system initialization */
#include "config.c"

#include <rtems.h>
#include <rtems/bspIo.h>
#include <rtems/libi2c.h>
#include <rtems/libio.h>

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* Standard Read, Write function for SPI with Verbose on console*/

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
    printf("write() call returned with error: %s\n",
	   strerror(errno));
  } else if (status < count) {
    printf("write() did not output all bytes\n"
	   "requested bytes: %d, returned bytes: %d\n\n",
	   count, status);
  }
}

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

/* =========================================================
   initialisation
*/
rtems_task Init( rtems_task_argument ignored )
{
  int fd;
	unsigned char rxbuf[50], txbuf[50];

	printf("******** Initializing SPICTRL test ********\n");

  /* Initialize Driver manager, in config.c */
	system_init();

#ifdef USE_GPIO_SLVSEL
	spi_gpio_slvsel_setup(GPIO_PORT_NR);
#endif
/*
	drvmgr_print_devs(0xfffff);
	drvmgr_print_topo();
*/

  /* Raw Driver Alternative */
  mknod("/dev/SPIbasicRAW", S_IFIFO | S_IWUSR | S_IRUSR , MKDEV(rtems_libi2c_major, RTEMS_LIBI2C_MAKE_MINOR(0,0x54)));
  char data_to_send[] = "Hello World RAW SPI\n";

  fd = open("/dev/SPIbasicRAW",O_RDWR);

  if ( fd < 0 ) {
    printf("Failed to open RAW SPI\n");
    exit(0);
  }

  ver_write(fd, data_to_send, sizeof(data_to_send));

  rtems_task_wake_after(1000);

  // Read sizeof(data_to_send) bytes
  printf("\n\nREAD address start at 0x0\n");
	memset(rxbuf, 0, sizeof(rxbuf));   // clear rxbuf to all 0
	ver_read(fd, &rxbuf[0], sizeof(data_to_send));
	printf("Read, size: %d bytes .. [0x%x, 0x%x, 0x%x, 0x%x]\n", 15, rxbuf[0], rxbuf[1], rxbuf[2], rxbuf[3]);
	printbuf(rxbuf, sizeof(data_to_send), 0);

  close(fd);

	exit(0);
}

#ifdef USE_GPIO_SLVSEL
int spi_gpio_slvsel_setup(int portnr)
{
	struct gpiolib_config cfg;

	/* The drivers that use the GPIO Libarary have already initialized
	 * the libarary, however if no cores the drivers will not initialize it.
	 */
	if ( gpiolib_initialize() ) {
		printf("Failed to initialize GPIO libarary\n");
		return -1;
	}

	/* Show all GPIO Ports available */
	gpiolib_show(-1, NULL);

	spi_gpio_port = gpiolib_open(portnr);
	if ( spi_gpio_port == NULL ){
		printf("Failed to open gpio port %d\n", portnr);
		return -1;
	}

	/* Mask IRQ, Low level (interrupt not enabled anyway) */
	cfg.mask = 0;
	cfg.irq_level = 1;
	cfg.irq_polarity = 0;
	gpiolib_set_config(spi_gpio_port, &cfg);

	/* Drive pin High */
	gpiolib_set(spi_gpio_port, 1, 1);

	return 0;
}

int spi_gpio_slvsel(void *regs, int addr, int select)
{
	/*printk("SPI ADDR: addr 0x%x [0x%x]\n", addr, regs);*/
	if ( (addr >= 0) && select ) {
		/* Drive low */
		gpiolib_set(spi_gpio_port, 1, 0);
	} else {
		/* Drive high */
		gpiolib_set(spi_gpio_port, 1, 1);
	}
	return 0;
}

#endif
