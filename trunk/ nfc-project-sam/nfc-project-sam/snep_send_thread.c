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
#include "snep_send_thread.h"

//To enable the llcp_log_log function.
//Must be defined before the include of llcp_log.h !!!!!!!!!!!!
#define DEBUG 1

#include "llcp_log.h"

void *
snep_send_thread (void *arg) {

	llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_FATAL, "[snep_send_thread] started!");
    struct llc_connection *connection = (struct llc_connection *) arg;
    uint8_t beam_example[] = {0x10, 0x02, 0x00, 0x00, 0x00, 0x47, 0xd2, 0x24, 0x20, 0x61, 0x70,
    		0x70, 0x6c, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x2f, 0x63, 0x6f, 0x6d, 0x2e, 0x65, 0x78, 0x61, 0x6d,
    		0x70, 0x6c, 0x65, 0x2e, 0x61, 0x6e, 0x64, 0x72, 0x6f, 0x69, 0x64, 0x2e, 0x62, 0x65, 0x61, 0x6d, 0x42, 0x65,
    		0x61, 0x6d, 0x20, 0x6d, 0x65, 0x20, 0x75, 0x70, 0x21, 0x0a, 0x0a, 0x42, 0x65, 0x61, 0x6d, 0x20, 0x54, 0x69,
    		0x6d, 0x65, 0x3a, 0x20, 0x31, 0x31, 0x3a, 0x34, 0x33, 0x3a, 0x32, 0x30
    };

    uint8_t youtube_link[] = {0x10, 0x02, 0x00, 0x00, 0x00, 0x33, 0xd3, 0x30, 0x00,
		0x68, 0x74, 0x74, 0x70, 0x3a, 0x2f, 0x2f, 0x77, 0x77, 0x77, 0x2e, 0x79, 0x6f,
		0x75, 0x74, 0x75, 0x62, 0x65, 0x2e, 0x63, 0x6f, 0x6d, 0x2f, 0x77, 0x61, 0x74,
		0x63, 0x68, 0x3f, 0x76, 0x3d, 0x6e, 0x43, 0x67, 0x51, 0x44, 0x6a, 0x69, 0x6f,
		0x74, 0x47, 0x30, 0x23, 0x74, 0x3d, 0x32, 0x2e, 0x30
    };

    struct ndef_record *beamRec;
    if(beamRec = malloc(sizeof *beamRec)) {
    	beamRec->MB = 1;
    	beamRec->ME = 1;
    	beamRec->CF = 0;
    	beamRec->IL = 0;
    	beamRec->TNF = 2;
    	beamRec->SR = 1;
    	beamRec->type = "application/com.example.android.beam";
    	beamRec->type_length = strlen(beamRec->type);
    	beamRec->payload = "Hello World to Sam :-)";
    	beamRec->payload_length = strlen(beamRec->payload);
    }

    struct ndef_record *ytRec;
	if(ytRec = malloc(sizeof *ytRec)) {
		ytRec->MB = 1;
		ytRec->ME = 1;
		ytRec->CF = 0;
		ytRec->IL = 0;
		ytRec->TNF = 3;
		ytRec->SR = 1;
		ytRec->type = "http://www.youtube.com/watch?v=nCgQDjiotG0#t=1.0";
		ytRec->type_length = strlen(ytRec->type);
		ytRec->payload = "";
		ytRec->payload_length = strlen(ytRec->payload);
	}

    uint8_t res_buffer[1024];

    int len = snep_pack(ytRec, res_buffer);

    sleep (1);

    llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_FATAL, "[snep_send_thread] Sending data");
    llc_connection_send (connection, res_buffer, len);

    llc_connection_stop (connection);

    return NULL;

}

