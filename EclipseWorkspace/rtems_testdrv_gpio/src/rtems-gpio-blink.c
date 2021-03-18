/*
 * A RTEMS sample application using the GPIO Library and GRGPIO driver
 */

#include <rtems.h>

/* init configuration information */
#define CONFIGURE_INIT

#include <bsp.h> /* for device driver prototypes */

rtems_task Init( rtems_task_argument argument);	/* forward declaration needed */

/* rtems configuration information */
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_MAXIMUM_TASKS             4
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_EXTRA_TASK_STACKS         (3 * RTEMS_MINIMUM_STACK_SIZE)
#define CONFIGURE_MAXIMUM_SEMAPHORES        4

#define RTEMS_PCI_CONFIG_LIB /* load PCI driver*/
#define CONFIGURE_PCI_LIB PCI_LIB_AUTO /* Auto configuration PCI LIB */

#include <rtems/confdefs.h>

/* Configure Driver manager */
/* Add Timer and UART Driver for this example */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GPTIMER
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_APBUART
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRGPIO  /* GRGPIO driver */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_PCIF    /* PCI is for RASTA-IO GRGPIO */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRPCI   /* PCI is for RASTA-IO GRGPIO */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRPCI2  /* PCI is for RASTA-IO GRGPIO */
#define CONFIGURE_DRIVER_PCI_GR_RASTA_IO        /* GR-RASTA-IO PCI Target Driver */

#include <drvmgr/drvmgr_confdefs.h>
/* end of driver config*/

#include <rtems/bspIo.h>
#include <stdio.h>
#include <stdlib.h>

#include <grlib/gpiolib.h>

#define GPIO_PORT_NR (4) /* GPIO port/pin number*/

void *port;

void gpio_isr(int irq, void *arg)
{
	struct gpiolib_config cfg; // config a GPIO port

	/* Mask away GPIO IRQ */
	cfg.mask = 0;
	cfg.irq_level = GPIOLIB_IRQ_LEVEL;
	cfg.irq_polarity = GPIOLIB_IRQ_POL_LOW;

	if ( gpiolib_set_config(port, &cfg) ){
		printf("Failed to configure gpio port %d\n", GPIO_PORT_NR);
		exit(0);
	}
	printk("GPIO_ISR: %d, GOT %d\n", irq, (int)arg);
}

rtems_task Init( rtems_task_argument ignored )
{
	struct gpiolib_config cfg;
	int val;
	int timeout_limit;

	/* The drivers that use the GPIO Libarary have already initialized the libarary,
   * however if no cores the drivers will not initialize it.
	 */
	if ( gpiolib_initialize() ) {
		printf("Failed to initialize GPIO libarary\n");
		exit(0);
	}

	/* Show all GPIO Ports available */
	//gpiolib_show(-1, NULL); // -1 argument for all ports

	port = gpiolib_open(GPIO_PORT_NR);
	if ( port == NULL ){
		printf("Failed to open gpio port %d\n", GPIO_PORT_NR);
		exit(0);
	}

	/* Mask away GPIO IRQ */
	cfg.mask = 0;
	cfg.irq_level = GPIOLIB_IRQ_LEVEL;
	cfg.irq_polarity = GPIOLIB_IRQ_POL_LOW;

	if ( gpiolib_set_config(port, &cfg) ){
		printf("Failed to configure gpio port %d\n", GPIO_PORT_NR);
		exit(0);
	}

	if ( gpiolib_irq_disable(port)){
		printf("Failed to disable IRQ on gpio port %d, disabling port "
		       "while not enabled should fail. OK\n", GPIO_PORT_NR);
	} else {
		printf("Disable IRQ on port %d should fail\n", GPIO_PORT_NR);
		exit(0);
	}

	if ( gpiolib_irq_clear(port) ){
		printf("Failed to clear IRQ on gpio port %d\n", GPIO_PORT_NR);
		exit(0);
	}

	if ( gpiolib_irq_register(port, gpio_isr, (void *)GPIO_PORT_NR) ){
		printf("Failed to register IRQ on gpio port %d\n", GPIO_PORT_NR);
		exit(0);
	}

	if ( gpiolib_irq_enable(port) ){
		printf("Failed to enable IRQ on gpio port %d\n", GPIO_PORT_NR);
		exit(0);
	}

  /* Read Current Value on GPIO */
	if ( gpiolib_get(port, &val) ){
		printf("Failed to get value of gpio port %d\n", GPIO_PORT_NR);
		exit(0);
	}
	printf("Current Value on GPIO: %d\n", val);

  rtems_task_wake_after(100);
  puts("Start Blinking\n");
  rtems_task_wake_after(100);

  timeout_limit = 10;
  while(timeout_limit>0)
  {
    gpiolib_set(port,1, 0);
    rtems_task_wake_after(1000);
    gpiolib_set(port,1, 1);
    rtems_task_wake_after(1000);
    timeout_limit--;
  }

  puts("Exiting Blinking Section\n");

	/* Enable IRQ by unmasking IRQ */
	cfg.mask = 1;
	if ( gpiolib_set_config(port, &cfg) ){
		printf("Failed to configure gpio port %d\n", GPIO_PORT_NR);
		exit(0);
	}

	/* Print the same */
	gpiolib_show(GPIO_PORT_NR, NULL);
	gpiolib_show(0, port);

	gpiolib_close(port);
	puts("TEST END");

	exit( 0 );
}
