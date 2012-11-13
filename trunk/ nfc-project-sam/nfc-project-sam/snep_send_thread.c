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

    struct ndef_record *beamRec;
    if((beamRec = malloc(sizeof *beamRec))) {
    	beamRec->MB = 1;
    	beamRec->ME = 1;
    	beamRec->CF = 0;
    	beamRec->IL = 0;
    	beamRec->TNF = 2;
    	beamRec->SR = 1;
    	beamRec->type = (uint8_t *)"application/com.example.android.beam";
    	beamRec->type_length = strlen((char *)beamRec->type);
    	beamRec->payload = (uint8_t *)"Hello World to Sam :-)";
    	beamRec->payload_length = strlen((char *)beamRec->payload);
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

    int len = snep_pack(ytRec, res_buffer);

    sleep (1);

    llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_FATAL, "[snep_send_thread] Sending data");
    llc_connection_send (connection, res_buffer, len);

    llc_connection_stop (connection);

    return NULL;

}

