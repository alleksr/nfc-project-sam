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
#include "snep_service_thread.h"

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

    llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_FATAL, "[snep_send_thread] Success response sent");

    struct llc_link *my_llc_link = connection->link;

    /*

	uint8_t answer[] = {0x10, 0x02, 0x00, 0x00, 0x00, 0x47, 0xd2, 0x24, 0x20, 0x61, 0x70,
			0x70, 0x6c, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x2f, 0x63, 0x6f, 0x6d, 0x2e, 0x65, 0x78, 0x61, 0x6d,
			0x70, 0x6c, 0x65, 0x2e, 0x61, 0x6e, 0x64, 0x72, 0x6f, 0x69, 0x64, 0x2e, 0x62, 0x65, 0x61, 0x6d, 0x42, 0x65,
			0x61, 0x6d, 0x20, 0x6d, 0x65, 0x20, 0x75, 0x70, 0x21, 0x0a, 0x0a, 0x42, 0x65, 0x61, 0x6d, 0x20, 0x54, 0x69,
			0x6d, 0x65, 0x3a, 0x20, 0x31, 0x31, 0x3a, 0x34, 0x33, 0x3a, 0x32, 0x30
	};


	llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_FATAL, "[snep_send_thread] Sending data");
	llc_connection_send (connection, answer, sizeof (answer));

    */


    llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_DEBUG, "[snep_service_thread] Stopping llc_connection");
    llc_connection_stop (connection);

    llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_DEBUG, "[snep_service_thread] Deactivating llc_link");
    llc_link_deactivate(my_llc_link);

    return NULL;

}

