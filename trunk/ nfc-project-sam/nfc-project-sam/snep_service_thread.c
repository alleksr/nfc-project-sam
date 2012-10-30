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

#include "snep.h"

//To enable the llcp_log_log function.
//Must be defined before the include of llcp_log.h !!!!!!!!!!!!
#define DEBUG 1

#include "llcp_log.h"

void *
snep_service_thread (void *arg) {

	llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_INFO, "[snep_service_thread] started!");

    struct llc_connection *connection = (struct llc_connection *) arg;
    uint8_t buffer[1024];

    int len;
    llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_INFO, "[snep_service_thread] Receiving from connection");
    if ((len = llc_connection_recv (connection, buffer, sizeof (buffer), NULL)) < 0)
        return NULL;

    llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_INFO, "[snep_service_thread] Received %d bytes", len);



    struct snep_message *msg = snep_unpack(buffer, sizeof(buffer));

    llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_INFO, "[snep_service_thread] snep version: %i.%i", msg->major_version, msg->minor_version);
    llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_INFO, "[snep_service_thread] snep request type: %i", msg->type_field);
    llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_INFO, "[snep_service_thread] data length: %i bytes", msg->data_length);

    int i = 0;
    llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_INFO, "[snep_service_thread] Printing data");
    for(i=0; i<msg->data_length; i++) {
        printf("%c", msg->ndef_message[i]);
    }
    printf("\n");




    uint8_t success_response[6]; 	//Version - Success - Data Length = 0
    snep_create_success_response(success_response);


    llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_INFO, "[snep_service_thread] Sending success response via SNEP");
    llc_connection_send(connection, success_response, sizeof(success_response));


    struct llc_link *my_llc_link = connection->link;

    llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_DEBUG, "[snep_service_thread] Stopping llc_connection");
    llc_connection_stop (connection);

    llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_DEBUG, "[snep_service_thread] Deactivating llc_link");
    llc_link_deactivate(my_llc_link);

    return NULL;

}

