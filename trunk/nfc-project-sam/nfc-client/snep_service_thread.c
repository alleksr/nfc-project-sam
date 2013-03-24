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
    uint8_t send_buffer[1024];
    uint8_t res_buffer[1024];
    int teller;

    struct ndef_record *PingRec;
	if((PingRec = malloc(sizeof *PingRec))) {
		PingRec->MB = 1;
		PingRec->ME = 1;
		PingRec->CF = 0;
		PingRec->IL = 0;
		PingRec->TNF = 2;
		PingRec->SR = 1;
		PingRec->type = (uint8_t *)"text/plain";
		PingRec->type_length = strlen((char *)PingRec->type);
		PingRec->payload = (uint8_t *)"Ping";
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
			PongRec->type = (uint8_t *)"text/plain";
			PongRec->type_length = strlen((char *)PongRec->type);
			PongRec->payload = (uint8_t *)"Pong";
			PongRec->payload_length = strlen((char *)PongRec->payload);
		}


    for(teller=0; teller<5; teller++ ) {
    /************************* Sending Ping ***************************************/
		int len = snep_pack(PingRec, send_buffer);

		llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_FATAL, "[snep_send_thread] Sending data");
		llc_connection_send (connection, send_buffer, len);

		//Receiving the SNEP Success response!
		if ((len = llc_connection_recv (connection, res_buffer, sizeof (res_buffer), NULL)) < 0)
				return NULL;
		struct snep_message *msg = snep_unpack(res_buffer, len);
		if(msg->type_field == RESPONSE_SUCCESS) {
			llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_DEBUG, "SNEP Response: Success" );
		}else{
			llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_FATAL, "SNEP Response: Not success!!" );
		}
    }

    /************************* Sending Pong ***************************************/
	int len = snep_pack(PongRec, send_buffer);

	llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_FATAL, "[snep_send_thread] Sending data");
	llc_connection_send (connection, send_buffer, len);

	//Receiving the SNEP Success response!
	if ((len = llc_connection_recv (connection, res_buffer, sizeof (res_buffer), NULL)) < 0)
			return NULL;
	struct snep_message *msg = snep_unpack(res_buffer, len);
	if(msg->type_field == RESPONSE_SUCCESS) {
		llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_DEBUG, "SNEP Response: Success" );
	}else{
		llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_FATAL, "SNEP Response: Not success!!" );
	}


    /************************* Receiving Plong ***************************************/

	for(teller=0; teller<5; teller++) {
		//int len;
		llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_INFO, "[snep_service_thread] Receiving from connection");
		if ((len = llc_connection_recv (connection, buffer, sizeof (buffer), NULL)) < 0)
			return NULL;

		llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_INFO, "[snep_service_thread] Received %d bytes", len);

		msg = snep_unpack(buffer, sizeof(buffer));

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
			llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_FATAL, "[snep_send_thread] Success response sent");
		}
	}

	//Next sleep is necessary. If it wouldn't be here, then the data is sent too soon to the Nexus S.
	//As a result, the phone will answer with a frame rejected. At 20msec, such a frame is received rarely.
	//At 30msec, no errors were ever received.
	usleep(30000);

    for(teller=0; teller<5; teller++ ) {
    /************************* Sending Ping ***************************************/
		int len = snep_pack(PingRec, send_buffer);

		llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_FATAL, "[snep_send_thread] Sending data");
		llc_connection_send (connection, send_buffer, len);

		//Receiving the SNEP Success response!
		if ((len = llc_connection_recv (connection, res_buffer, sizeof (res_buffer), NULL)) < 0)
				return NULL;
		struct snep_message *msg = snep_unpack(res_buffer, len);
		if(msg->type_field == RESPONSE_SUCCESS) {
			llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_DEBUG, "SNEP Response: Success" );
		}else{
			llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_FATAL, "SNEP Response: Not success!!" );
		}
    }

    /************************* Sending Pong ***************************************/
	len = snep_pack(PongRec, send_buffer);

	llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_FATAL, "[snep_send_thread] Sending data");
	llc_connection_send (connection, send_buffer, len);

	//Receiving the SNEP Success response!
	if ((len = llc_connection_recv (connection, res_buffer, sizeof (res_buffer), NULL)) < 0)
			return NULL;
	msg = snep_unpack(res_buffer, len);
	if(msg->type_field == RESPONSE_SUCCESS) {
		llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_DEBUG, "SNEP Response: Success" );
	}else{
		llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_FATAL, "SNEP Response: Not success!!" );
	}


    /************************* Receiving Plong ***************************************/

	for(teller=0; teller<5; teller++) {
		//int len;
		llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_INFO, "[snep_service_thread] Receiving from connection");
		if ((len = llc_connection_recv (connection, buffer, sizeof (buffer), NULL)) < 0)
			return NULL;

		llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_INFO, "[snep_service_thread] Received %d bytes", len);

		msg = snep_unpack(buffer, sizeof(buffer));

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
			llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_FATAL, "[snep_send_thread] Success response sent");
		}
	}



    //struct llc_link *my_llc_link = connection->link;

    //llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_DEBUG, "[snep_service_thread] Stopping llc_connection");
    //llc_connection_stop (connection);

    //llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_DEBUG, "[snep_service_thread] Deactivating llc_link");
    //llc_link_deactivate(my_llc_link);

    return NULL;

}

