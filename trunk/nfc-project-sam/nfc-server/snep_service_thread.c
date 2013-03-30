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
    struct timeval tvBegin, tvEnd, tvDiff;
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
		PingRec->payload = (uint8_t *)"Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping";
		//PingRec->payload = (uint8_t *)"Ping Ping Ping Ping Ping";
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



    /************************* Sending Ping 5 times ***************************************/
	for(teller=0; teller<5; teller++ ) {
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

    /************************* Sending Pong 1 time ***************************************/
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


    /************************* Receiving Plong 5 times ***************************************/

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


    /************************* Sending Ping 5 times ***************************************/
	for(teller=0; teller<5; teller++ ) {
		int len = snep_pack(PingRec, send_buffer);

		llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_FATAL, "[snep_send_thread] Sending data");

		// begin
		gettimeofday(&tvBegin, NULL);
		timeval_print(&tvBegin);

		llc_connection_send (connection, send_buffer, len);

		//Receiving the SNEP Success response!
		if ((len = llc_connection_recv (connection, res_buffer, sizeof (res_buffer), NULL)) < 0)
				return NULL;
		//end
		gettimeofday(&tvEnd, NULL);
		timeval_print(&tvEnd);

		// diff
		timeval_subtract(&tvDiff, &tvEnd, &tvBegin);
		printf("Difference: seconds: %ld . usecs: %06ld\n", tvDiff.tv_sec, tvDiff.tv_usec);

		struct snep_message *msg = snep_unpack(res_buffer, len);
		if(msg->type_field == RESPONSE_SUCCESS) {
			llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_DEBUG, "SNEP Response: Success" );
		}else{
			llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_FATAL, "SNEP Response: Not success!!" );
		}
    }

    /************************* Sending Pong 1 time ***************************************/
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


    /************************* Receiving Plong 5 times ***************************************/

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


	llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_FATAL, "[snep_send_thread] sending disc pdu");

	struct pdu *disc_pdu = pdu_new(connection->remote_sap, PDU_DISC, connection->local_sap, 0, 0, 0, 0);

	llc_connection_send_pdu(connection, disc_pdu);

    return NULL;

}

/* Return 1 if the difference is negative, otherwise 0.  */
int timeval_subtract(struct timeval *result, struct timeval *t2, struct timeval *t1)
{
    long int diff = (t2->tv_usec + 1000000 * t2->tv_sec) - (t1->tv_usec + 1000000 * t1->tv_sec);
    result->tv_sec = diff / 1000000;
    result->tv_usec = diff % 1000000;

    return (diff<0);
}

void timeval_print(struct timeval *tv)
{
    char buffer[30];
    time_t curtime;

    printf("%ld.%06ld", tv->tv_sec, tv->tv_usec);
    curtime = tv->tv_sec;
    strftime(buffer, 30, "%m-%d-%Y  %T", localtime(&curtime));
    printf(" = %s.%06ld\n", buffer, tv->tv_usec);
}
