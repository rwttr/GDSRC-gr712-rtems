/*
 *
 *
 *  Created on: Mar 24, 2021
 *      Author: Rattachai
 *
 *
 * A RTEMS sample application using the GPIO Library and GRGPIO driver
 */

#include <rtems.h>

#define CONFIGURE_INIT
#include <bsp.h>

rtems_task Init( rtems_task_argument argument);

/* rtems configuration information */
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_MAXIMUM_TASKS             4
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_EXTRA_TASK_STACKS         (3 * RTEMS_MINIMUM_STACK_SIZE)
#define CONFIGURE_MAXIMUM_SEMAPHORES        4

#define CONFIGURE_INIT_TASK_ATTRIBUTES RTEMS_FLOATING_POINT

#define RTEMS_PCI_CONFIG_LIB /* load PCI driver*/
#define CONFIGURE_PCI_LIB PCI_LIB_AUTO /* Auto configuration PCI LIB */

#include <rtems/confdefs.h>

/* Configure Driver manager */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GPTIMER
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_APBUART
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRGPIO

#include <drvmgr/drvmgr_confdefs.h>
/* end of driver config*/

#include <stdio.h>
#include <stdlib.h>
#include <rtems/bspIo.h>
#include <grlib/gpiolib.h>

#define GPIO_PORT_NRSET 5	// output pin
#define GPIO_PORT_NRGET 6	// input pin

#define TOGGLE_LIMIT 10

void *port_set;
void *port_get;
int port_stat = 0;
char toggle_count =  0;

void gpio_isr_getnset(int irq, void *arg)
{
	struct gpiolib_config cfg; // config a GPIO port

	/* Mask away GPIO IRQ */
	cfg.mask = 0;
	cfg.irq_level = GPIOLIB_IRQ_LEVEL;
	cfg.irq_polarity = GPIOLIB_IRQ_POL_LOW;
	gpiolib_set_config(port_set, &cfg);

	printf("Interrupt Entered: Toggling Pin %d, ", GPIO_PORT_NRSET);

	port_stat = port_stat^1; // XOR with 1 for toggling
	gpiolib_set(port_set,1, port_stat);

	printf("OK\n");
	rtems_task_wake_after(100);
	toggle_count++;
}

void gpio_isr_setdo(int irq, void *arg)
{
	struct gpiolib_config cfg; // config a GPIO port
	int port_val;
	/* Mask away GPIO IRQ */
	cfg.mask = 0;
	cfg.irq_level = GPIOLIB_IRQ_LEVEL;
	cfg.irq_polarity = GPIOLIB_IRQ_POL_LOW;
	gpiolib_set_config(port_set, &cfg);

	gpiolib_get(port_set,&port_val);
	printf("Current Value on GPIO: %d\n", port_val);
	rtems_task_wake_after(100);

	printf("Current Toggle Attempt: %d\n",toggle_count);

	if(toggle_count >= TOGGLE_LIMIT)
	{
		printf("Toggle limited, exiting program\n");
		cfg.mask = 1;
		gpiolib_close(port_get);
		gpiolib_close(port_set);
		gpiolib_set_config(port_get, &cfg);
		gpiolib_set_config(port_set, &cfg);
		exit(0);
	}

}

rtems_task Init( rtems_task_argument ignored )
{

	if ( gpiolib_initialize() ) {
		printf("Failed to initialize GPIO libarary\n");
		exit(0);
	}
	gpiolib_show(-1, NULL); // -1 argument for all ports
	/* Setup Interrupt on input port */
	/* Mask away GPIO IRQ */

	struct gpiolib_config cfg;
	cfg.mask = 0;
	cfg.irq_level = GPIOLIB_IRQ_LEVEL;
	cfg.irq_polarity = GPIOLIB_IRQ_POL_LOW;

	/* Open input port */
	port_get = gpiolib_open(GPIO_PORT_NRGET);
	if ( port_get == NULL ){
		printf("Failed to open gpio port %d\n", GPIO_PORT_NRGET);
		exit(0);
	}
	gpiolib_set_config(port_get, &cfg);
	gpiolib_irq_disable(port_get);
	gpiolib_irq_clear(port_get);
	gpiolib_irq_register(port_get, gpio_isr_getnset, (void *)GPIO_PORT_NRGET );
	gpiolib_irq_enable(port_get);


	/* Open output port */
	port_set = gpiolib_open(GPIO_PORT_NRSET);
	if ( port_set == NULL ){
		printf("Failed to open gpio port %d\n", GPIO_PORT_NRSET);
		exit(0);
	}
	cfg.mask = 0; /* Mask away GPIO IRQ */
	gpiolib_set_config(port_set, &cfg);
	gpiolib_irq_disable(port_set);
	gpiolib_irq_clear(port_set);
	gpiolib_irq_register(port_set, gpio_isr_setdo, (void *)GPIO_PORT_NRSET );
	gpiolib_irq_enable(port_set);


	puts("Wating for interrupt ..");
	while(1){
		rtems_task_wake_after(1000);
	}
	//exit( 0 );
}

