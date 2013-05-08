#include <err.h>
#include <getopt.h>
#include <libgen.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

#include "llcp_pdu.h"
#include "llc_link.h"
#include "llc_service.h"
#include "mac.h"
#include "llcp_parameters.h"
#include "llc_connection.h"
#include "snep_service_thread.h"

#include "chips/pn53x.h"

//To enable the llcp_log_log function.
//Must be defined before the include of llcp_log.h !!!!!!!!!!!!
#define DEBUG 1

#include "llcp_log.h"

static nfc_context *context;

int main (int argc, char *argv[])
{
	nfc_init(&context);
	  if (context == NULL) {
		printf("Unable to init libnfc (malloc)");
		exit(EXIT_FAILURE);
	  }


	if (llcp_init () < 0)
	errx (EXIT_FAILURE, "llcp_init()");

	llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_WARN, "This is a test warning message");
	llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_FATAL, "This is a test fatal message");
	llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_TRACE, "This is a test trace message");
	llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_DEBUG, "This is a test debug message");
	llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_ERROR, "This is a test error message");


	int res;

	//Auto probe for nfc device
	/*
	nfc_connstring device_connstring[1];

	res = nfc_list_devices (NULL, device_connstring, 1);
	if (res < 1) errx (EXIT_FAILURE, "Sorry, no NFC device found");

	//Create nfc_device
	nfc_device *device;

	if (!(device = nfc_open (NULL, device_connstring[0]))) {
		errx (EXIT_FAILURE, "Cannot connect to NFC device");
	}
	*/


	//Create nfc_device
	nfc_device *device;

	//if (!(device = nfc_open (NULL, device_connstring[0]))) {
	//if (!(device = nfc_open (NULL, "pn532_uart:/dev/ttyUSB0:115200"))) {
	if (!(device = nfc_open (context, NULL))) {
			errx (EXIT_FAILURE, "Cannot connect to NFC device");
	}

	//Create llc_link
	struct llc_link *my_llc_link = llc_link_new ();

	struct llc_service *snep_service;


	//Create receiving service!
	if (!(snep_service = llc_service_new_with_uri (NULL, snep_service_thread, LLCP_SNEP_URI, NULL))) errx (EXIT_FAILURE, "Cannot create snep service");
	//Bind llc_service to llc_link
	if (llc_link_service_bind (my_llc_link, snep_service, LLCP_SNEP_SAP) < 0) {
		errx (EXIT_FAILURE, "llc_service_new_with_uri()");
	}

	//Create mac_link
	struct mac_link *my_mac_link = mac_link_new (device, my_llc_link);

	if (!my_mac_link) errx (EXIT_FAILURE, "Cannot establish MAC link");

	//Active mac_link as initiator
	res = mac_link_activate_as_initiator(my_mac_link);
	if (res <= 0) errx (EXIT_FAILURE, "Cannot activate link");


	//Wait for mac_link to finish
	void *status;
		mac_link_wait (my_mac_link, &status);

		printf ("STATUS = %p\n", status);

	mac_link_free (my_mac_link);
	llc_link_free (my_llc_link);

	nfc_close (device);

	llcp_fini ();
	nfc_exit(context);
	exit(EXIT_SUCCESS);

}
