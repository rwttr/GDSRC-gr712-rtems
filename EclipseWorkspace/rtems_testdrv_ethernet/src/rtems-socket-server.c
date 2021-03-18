#include <rtems.h>
#include <rtems/untar.h>

#define CONFIGURE_INIT
#include <bsp.h>

#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_MAXIMUM_FILE_DESCRIPTORS	20
#define CONFIGURE_USE_IMFS_AS_BASE_FILESYSTEM

#define CONFIGURE_EXECUTIVE_RAM_SIZE	(1024*1024)
#define CONFIGURE_MAXIMUM_SEMAPHORES	20
#define CONFIGURE_MAXIMUM_TASKS		20

#define CONFIGURE_MICROSECONDS_PER_TICK	10000

#define CONFIGURE_INIT_TASK_STACK_SIZE	(10*1024)
#define CONFIGURE_INIT_TASK_PRIORITY	120
#define CONFIGURE_INIT_TASK_INITIAL_MODES (RTEMS_PREEMPT | \
                                           RTEMS_NO_TIMESLICE | \
                                           RTEMS_NO_ASR | \
                                           RTEMS_INTERRUPT_LEVEL(0))
#define CONFIGURE_INIT_TASK_ATTRIBUTES RTEMS_FLOATING_POINT

rtems_task Init( rtems_task_argument argument); /*forward declaration */

/* configuration information */
#include <rtems/confdefs.h>

#include <drvmgr/drvmgr.h>

/* Configure Driver manager */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GPTIMER
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_APBUART
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRETH

#include <drvmgr/drvmgr_confdefs.h>


/* Application */
#include <errno.h>
#include <time.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <rtems/bspIo.h>
#include <rtems/rtems_bsdnet.h>
#include <rtems/ftpd.h>
     
#include <rtems/error.h>
#include <rpc/rpc.h>
#include <netinet/in.h>
#include <time.h>


/* Include driver configrations and system initialization */
#include <grlib/network_interface_add.h>
#include "networkconfig.h"

#define PORT 8888

rtems_task Init( rtems_task_argument argument)
{
  //rtems_status_code status;
	printf("\nInit Task Entered \n");

	struct rtems_bsdnet_ifconfig ethif;
	ethif.name = RTEMS_BSP_NETWORK_DRIVER_NAME_OPENETH;
	ethif.drv_ctrl = NULL;
	ethif.attach = (void *)RTEMS_BSP_NETWORK_DRIVER_ATTACH_OPENETH;
	network_interface_add(&ethif);

	/*print registered device driver*/
	//drvmgr_print_devs(0xfffff);

	/* Init network */
	printf("Initializing network\n\n");
	rtems_bsdnet_initialize_network();
	printf("\nInitializing network DONE\n\n");
	rtems_bsdnet_show_inet_routes();
	printf("\n");
	rtems_bsdnet_show_if_stats();
	printf("\n\n");

	int server_fd, new_socket, valread;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	char buffer[1024] = {0};
	char *hello = "Hello World from GR712";

	// Creating socket file descriptor
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd  < 0)
	{
		printf("socket create failed\n");
		exit(0);
	}
	else
	{
		printf("TCP Socket Create : OK\n");
	}

	// Forcefully attaching socket to the port 8080
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		printf("setsockopt error, terminated\n");
		printf("errno: %s\n", strerror(errno));
		exit(0);
	}
	else
	{
	  printf("setsockopt : OK\n");
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( PORT );

	// Forcefully attaching socket to the port PORT
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
	{
		printf("bind failed, ");
		printf("errno: %s\n", strerror(errno));
		exit(0);
	}
	else
	{
		printf("port bind : OK\n");
	}

	if (listen(server_fd, 3) < 0)
	{
		printf("error listen, ");
		printf("errno: %s\n", strerror(errno));
		exit(0);
	}
	else
	{
		printf("Listening ..\n");
	}


	if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
	{
		perror("accept");
		exit(EXIT_FAILURE);
	}

	valread = read( new_socket , buffer, 1024);
	printf("%s\n",buffer );

	send(new_socket , hello , strlen(hello) , 0 );
	printf("Message sent\n");
   
	exit(0);

}




