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

    llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_INFO, "[snep_service_thread] Received %s bytes", len);

    if (len < 10) // NPP's header (5 bytes)  and NDEF entry header (5 bytes)
        return NULL;

    size_t n = 0;

    // Header
    fprintf (info_stream, "NDEF Push Protocol version: %02x\n", buffer[n]);
    if (buffer[n++] != 0x01) // Protocol version
        return NULL; // Protocol not version supported

    uint32_t ndef_entries_count = be32toh (*((uint32_t *)(buffer + n))); // Number of NDEF entries
    fprintf (info_stream, "NDEF entries count: %u\n", ndef_entries_count);
    if (ndef_entries_count != 1) // In version 0x01 of the specification, this value will always be 0x00, 0x00, 0x00, 0x01.
        return NULL;
    n += 4;

    // NDEF Entry
    if (buffer[n++] != 0x01) // Action code
        return NULL; // Action code not supported

    uint32_t ndef_length = be32toh (*((uint32_t *)(buffer + n))); // NDEF length
    n += 4;

    if ((len - n) < ndef_length)
        return NULL; // Less received bytes than expected ?

    char ndef_msg[1024];
    //shexdump (ndef_msg, buffer + n, ndef_length);
    fprintf (info_stream, "NDEF entry received (%u bytes): %s\n", ndef_length, ndef_msg);

    if (ndef_stream) {
        if (fwrite (buffer + n, 1, ndef_length, ndef_stream) != ndef_length) {
            fprintf (stderr, "Could not write to file.\n");
            fclose (ndef_stream);
            ndef_stream = NULL;
        } else {
            fclose (ndef_stream);
            ndef_stream = NULL;
        }
    }
    // TODO Stop the LLCP when this is reached
    llc_connection_stop (connection);

    return NULL;
}

