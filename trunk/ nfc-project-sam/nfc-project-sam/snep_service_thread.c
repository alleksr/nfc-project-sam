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


//To enable the llcp_log_log function.
//Must be defined before the include of llcp_log.h !!!!!!!!!!!!
#define DEBUG 1

#include "llcp_log.h"

void *
snep_service_thread (void *arg) {

	llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_INFO, "[snep_service_thread] started!");
	FILE* info_stream = NULL;
	FILE* ndef_stream = NULL;

    struct llc_connection *connection = (struct llc_connection *) arg;
    uint8_t buffer[1024];

    int len;
    llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_INFO, "[snep_service_thread] Receiving from connection");
    if ((len = llc_connection_recv (connection, buffer, sizeof (buffer), NULL)) < 0)
        return NULL;

    llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_INFO, "[snep_service_thread] Received %d bytes", len);


    int i = 0;
    int offset = 9;
    llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_INFO, "[snep_service_thread] Printing data");
    for(i=0; i<68; i++) {
        printf("%c", buffer[offset+i]);
    }
    printf("\n");

    uint8_t success_response[6] = { 0x10, 0x81, 0x00, 0x00, 0x00, 0x00 };	//Version - Success - Data Length = 0

    llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_INFO, "[snep_service_thread] Sending success response via SNEP");
    llc_connection_send(connection, success_response, sizeof(success_response));

    llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_DEBUG, "[snep_service_thread] Deactivating llc_link");
    struct llc_link *my_llc_link = connection->link;
    //llc_link_deactivate(my_llc_link);

    llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_DEBUG, "[snep_service_thread] Stopping llc_connection");
    llc_connection_stop (connection);

    return NULL;

}

