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
#define LOOP_COUNT 10

#include "llcp_log.h"

void *
snep_service_thread (void *arg) {

	llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_INFO, "[snep_service_thread] started!");

    struct llc_connection *connection = (struct llc_connection *) arg;
    uint8_t buffer[1024];
    uint8_t send_buffer[1024];
    uint8_t res_buffer[1024];
    int teller, loop, len;
    struct snep_message *msg;
    struct timeval timelogs[LOOP_COUNT];
    struct timeval t1, t2;
    long averageDelay;


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
		//PingRec->payload = (uint8_t *)"Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping";
		//PingRec->payload = (uint8_t *)"Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping Ping";
		//PingRec->payload = (uint8_t *)"25 bytes Ping -----------";
		//PingRec->payload = (uint8_t *)"50 bytes Ping ----------- 50 bytes Ping ----------";
		//PingRec->payload = (uint8_t *)"75 bytes Ping ----------- 75 bytes Ping ---------- 75 bytes Ping ----------";
		//PingRec->payload = (uint8_t *)"100 bytes Ping ---------- 100 bytes Ping --------- 100 bytes Ping --------- 100 bytes Ping ---------";
		//PingRec->payload = (uint8_t *)"150 bytes Ping ----------- 150 bytes Ping --------- 150 bytes Ping --------- 150 bytes Ping --------- 150 bytes Ping --------- 150 bytes Ping --------";
		//PingRec->payload = (uint8_t *)"200 bytes Ping ---------- 200 bytes Ping --------- 200 bytes Ping --------- 200 bytes Ping --------- 200 bytes Ping --------- 200 bytes Ping --------- 200 bytes Ping --------- 200 bytes Ping ---------";
		PingRec->payload = (uint8_t *)"MAX bytes Ping ---------- MAX bytes Ping --------- MAX bytes Ping --------- MAX bytes Ping --------- MAX bytes Ping --------- MAX bytes Ping --------- MAX bytes Ping --------- MAX bytes Ping --------- MAX bytes Ping -------------";
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

	for(loop=0; loop<LOOP_COUNT; loop++) {
		/************************* Sending Ping 1 times ***************************************/
		for(teller=0; teller<1; teller++ ) {
			int len = snep_pack(PingRec, send_buffer);

			llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_FATAL, "[snep_send_thread] Sending data");

		gettimeofday(&t1, NULL);
			llc_connection_send (connection, send_buffer, len);

			//Receiving the SNEP Success response!
			if ((len = llc_connection_recv (connection, res_buffer, sizeof (res_buffer), NULL)) < 0)
					return NULL;
		gettimeofday(&t2, NULL);
		timeval_subtract(&timelogs[loop], &t2, &t1);
			struct snep_message *msg = snep_unpack(res_buffer, len);
			if(msg->type_field == RESPONSE_SUCCESS) {
				llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_DEBUG, "SNEP Response: Success" );
			}else{
				llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_FATAL, "SNEP Response: Not success!!" );
			}
		}

		/************************* Receiving Pong 1 times ***************************************/

		for(teller=0; teller<1; teller++) {
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
	}

	llcp_log_log("\n[TIME]", LLC_PRIORITY_NOTICE, "Printing time logs:");
	for(loop=0; loop<LOOP_COUNT; loop++) {
		timeval_print(&timelogs[loop]);
		averageDelay += timelogs[loop].tv_usec;
	}
	llcp_log_log("\n[TIME]", LLC_PRIORITY_NOTICE, "Average delay = %lu µs", averageDelay/LOOP_COUNT);
	llcp_log_log("[TIME]", LLC_PRIORITY_NOTICE, "Size payload = %u bytes\n", PingRec->payload_length);

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

    return 0;
}

void timeval_print(struct timeval *tv)
{
    char buffer[30];
    time_t curtime;

    curtime = tv->tv_sec;
    strftime(buffer, 30, "%m-%d-%Y  %T", localtime(&curtime));
    llcp_log_log("[TIME]", LLC_PRIORITY_NOTICE, "%s.%06ld", buffer, tv->tv_usec);
}
