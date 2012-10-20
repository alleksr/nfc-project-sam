#include <err.h>
#include <getopt.h>
#include <libgen.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

#include "llc_link.h"
#include "llc_service.h"
#include "mac.h"
#include "llcp_parameters.h"

//To enable the llcp_log_log function.
//Must be defined before the include of llcp_log.h !!!!!!!!!!!!
#define DEBUG 1

#include "llcp_log.h"


void *
server_accept_thread (void *arg)
{
    llcp_log_log("[nfc-project.c]", LLC_PRIORITY_WARN, "In server_accept_thread");

    struct llc_connection *connection = (struct llc_connection *) arg;
    sleep (1);
    llc_connection_accept (connection);
    return NULL;
}

void *
server_thread (void *arg)
{

    llcp_log_log("[nfc-project.c]", LLC_PRIORITY_WARN, "In server_thread");

    struct llc_connection *connection = (struct llc_connection *) arg;
    uint8_t buffer[1024];

    int len;
    if ((len = llc_connection_recv (connection, buffer, sizeof (buffer), NULL)) < 0)
        return NULL;


    llc_connection_stop (connection);
    return NULL;
}

int main (int argc, char *argv[])
{
	nfc_init(NULL);

	if (llcp_init () < 0)
	errx (EXIT_FAILURE, "llcp_init()");

	llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_WARN, "This is a test warning message");
	llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_FATAL, "This is a test fatal message");
	llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_TRACE, "This is a test trace message");
	llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_DEBUG, "This is a test debug message");
	llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_ERROR, "This is a test error message");


	nfc_connstring device_connstring[1];

	int res;
	res = nfc_list_devices (NULL, device_connstring, 1);

	if (res < 1) errx (EXIT_FAILURE, "Sorry, no NFC device found");

	//Create nfc_device
	nfc_device *device;
	if (!(device = nfc_open (NULL, device_connstring[0]))) {
		errx (EXIT_FAILURE, "Cannot connect to NFC device");
	}


	//Create llc_link
	struct llc_link *my_llc_link = llc_link_new ();

	//Create llc_service
	//struct llc_service *my_llc_service = llc_service_new (server_accept_thread, server_thread, NULL);


	//Bind llc_service to llc_link
	//if (llc_link_service_bind (my_llc_link, my_llc_service, -1) < 0) {
	//	errx (EXIT_FAILURE, "llc_service_new_with_uri()");
	//}


	//Create mac_link
	struct mac_link *my_mac_link = mac_link_new (device, my_llc_link);

	if (!my_mac_link) errx (EXIT_FAILURE, "Cannot establish MAC link");


	//Active mac_link as target
	res = mac_link_activate_as_target(my_mac_link);
	if (res <= 0) errx (EXIT_FAILURE, "Cannot activate link");


	//const uint8_t *myLTO = 0x010104;	//2000msec(C8) 1byte(01) LTO(04)
	//llc_link_configure(my_llc_link, &myLTO, 3);



	//Wait for mac_link to finish
	void *status;
		mac_link_wait (my_mac_link, &status);

		printf ("STATUS = %p\n", status);

	mac_link_free (my_mac_link);
	llc_link_free (my_llc_link);

	nfc_close (device);

	llcp_fini ();
	nfc_exit(NULL);
	exit(EXIT_SUCCESS);

}
