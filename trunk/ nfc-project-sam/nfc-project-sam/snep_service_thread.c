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
#include "ndef.h"

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

    struct ndef_record *record = ndef_unpack(msg->ndef_message, msg->data_length);

    llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_INFO, "[snep_service_thread] NDEF Type:");
    int i=0;
    for(i=0; i<record->type_length; i++) {
    	printf("%c", record->type[i]);
    }
    printf("\n");

    llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_INFO, "[snep_service_thread] NDEF Payload:");
	for(i=0; i<record->payload_length; i++) {
		printf("%c", record->payload[i]);
	}
	printf("\n");

    if(msg->type_field == REQUEST_PUT ) {
    	int length = -1;
		uint8_t *success_response = snep_create_success_response(&length);

		llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_INFO, "[snep_service_thread] Sending success response via SNEP");
		llc_connection_send(connection, success_response, 6);
    }


    struct llc_link *my_llc_link = connection->link;

    llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_DEBUG, "[snep_service_thread] Stopping llc_connection");
    llc_connection_stop (connection);

    llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_DEBUG, "[snep_service_thread] Deactivating llc_link");
    llc_link_deactivate(my_llc_link);

    return NULL;

}

