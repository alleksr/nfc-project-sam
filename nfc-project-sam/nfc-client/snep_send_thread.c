#include <err.h>
#include <getopt.h>
#include <libgen.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "llcp_pdu.h"
#include "llc_link.h"
#include "llc_service.h"
#include "mac.h"
#include "llcp_parameters.h"
#include "llc_connection.h"

#include "snep.h"
#include "ndef.h"
#include "snep_send_thread.h"

//To enable the llcp_log_log function.
//Must be defined before the include of llcp_log.h !!!!!!!!!!!!
#define DEBUG 1

#include "llcp_log.h"

void *
snep_send_thread (void *arg) {

	llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_FATAL, "[snep_send_thread] started!");
    struct llc_connection *connection = (struct llc_connection *) arg;

    struct ndef_record *PingRec;
    if((PingRec = malloc(sizeof *PingRec))) {
    	PingRec->MB = 1;
    	PingRec->ME = 1;
    	PingRec->CF = 0;
    	PingRec->IL = 0;
    	PingRec->TNF = 2;
    	PingRec->SR = 1;
    	PingRec->type = (uint8_t *)"application/com.example.android.beam";
    	PingRec->type_length = strlen((char *)PingRec->type);
    	PingRec->payload = (uint8_t *)"Ping - ";
    	PingRec->payload_length = strlen((char *)PingRec->payload);
    }

    struct ndef_record *PongRec;
	if((PongRec = malloc(sizeof *PongRec))) {
		PongRec->MB = 1;
		PongRec->ME = 1;
		PongRec->CF = 0;
		PongRec->IL = 0;
		PongRec->TNF = 2;
		PongRec->SR = 1;
		PongRec->type = (uint8_t *)"application/com.example.android.beam";
		PongRec->type_length = strlen((char *)PongRec->type);
		PongRec->payload = (uint8_t *)"Pong - ";
		PongRec->payload_length = strlen((char *)PongRec->payload);
	}

    struct ndef_record *ytRec;
	if((ytRec = malloc(sizeof *ytRec))) {
		ytRec->MB = 1;
		ytRec->ME = 1;
		ytRec->CF = 0;
		ytRec->IL = 0;
		ytRec->TNF = 3;
		ytRec->SR = 1;
		ytRec->type = (uint8_t *)"http://www.youtube.com/watch?v=nCgQDjiotG0#t=1.0";
		ytRec->type_length = strlen((char *)ytRec->type);
		ytRec->payload = (uint8_t *)"";
		ytRec->payload_length = strlen((char *)ytRec->payload);
	}

    uint8_t res_buffer[1024];
    uint8_t out_buffer[1024];

    int i=0;
    for(i=0; i<5; i++) {

    	/************************* Sending Ping ***************************************/

		int len = snep_pack(PingRec, res_buffer);

		llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_FATAL, "[snep_send_thread] Sending Ping!");
		llc_connection_send (connection, res_buffer, len);

		//Receiving the SNEP Success response!
		if ((len = llc_connection_recv (connection, res_buffer, sizeof (res_buffer), NULL)) < 0)
				return NULL;
		struct snep_message *msg = snep_unpack(res_buffer, len);
		if(msg->type_field == RESPONSE_SUCCESS) {
			llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_DEBUG, "SNEP Response: Success" );
		}else{
			llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_FATAL, "SNEP Response: Not success!!" );
		}





		/************************* Receiving Pong ***************************************/

		llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_INFO, "[snep_service_thread] Receiving from connection");

		if ((len = llc_connection_recv (connection, out_buffer, sizeof (out_buffer), NULL)) < 0)
			return NULL;

		llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_INFO, "[snep_service_thread] Received %d bytes", len);


		struct snep_message *received_msg = snep_unpack(out_buffer, sizeof(out_buffer));

		struct ndef_record *record = ndef_unpack(received_msg->ndef_message, received_msg->data_length);

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

		if(received_msg->type_field == REQUEST_PUT ) {
			int length = -1;
			uint8_t *success_response = snep_create_success_response(&length);

			llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_INFO, "[snep_service_thread] Sending success response via SNEP");
			llc_connection_send(connection, success_response, 6);
		}

		llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_FATAL, "[snep_send_thread] Success response sent");


		usleep(25000);

    } //end for i=0..5


    llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_FATAL, "[snep_send_thread] sending disc pdu");

    struct pdu *dm_pdu = pdu_new(connection->remote_sap, PDU_DISC, connection->local_sap, 0, 0, 0, 0);

    llc_connection_send_pdu(connection, dm_pdu);

    usleep(250000);

    llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_FATAL, "[snep_send_thread] Stopping connection");


    return NULL;

}

