/*
 * snep.c
 *
 *  Created on: 30-okt-2012
 *      Author: Sam Van Den  Berge
 */

#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snep.h"

//To enable the llcp_log_log function.
//Must be defined before the include of llcp_log.h !!!!!!!!!!!!
#define DEBUG 1

#include "llcp_log.h"

struct snep_message *snep_unpack(const uint8_t *buffer, size_t len) {
	struct snep_message *msg;

	if ((msg = malloc (sizeof *msg))) {
		msg->major_version = (buffer[SNEP_VERSION_POS] & 0xF0) >> 4;
		msg->minor_version = buffer[SNEP_VERSION_POS] & 0x0F;
		msg->type_field = buffer[SNEP_TYPE_POS];
		msg->data_length = ReadUint32FromBuffer(&buffer[SNEP_LENGTH_POS]);
		msg->ndef_message = memdup (&buffer[SNEP_HEADER_LENGTH], msg->data_length);
	}

    llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_INFO, "[snep.c] snep version: %i.%i", msg->major_version, msg->minor_version);
    llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_INFO, "[snep.c] snep request type: %i", msg->type_field);
    llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_INFO, "[snep.c] data length: %i bytes", msg->data_length);

	return msg;
}

int snep_pack(struct ndef_record *record, uint8_t *buffer) {
	buffer[0] = SNEP_VERSION;
	buffer[1] = REQUEST_PUT;
	buffer[2] = 0x00;
	buffer[3] = 0x00;
	buffer[4] = 0x00;
	uint8_t length = ndef_pack(record, &buffer[6]);
	buffer[5] = length;
	return (length + 6);
}

uint8_t *snep_create_success_response(int *length) {
	uint8_t *buffer = malloc(6);

	buffer[0] = SNEP_VERSION;
	buffer[1] = RESPONSE_SUCCESS;
	buffer[2] = 0x00;
	buffer[3] = 0x00;
	buffer[4] = 0x00;
	buffer[5] = 0x00;

	*length = 6;
	return buffer;
}



uint32_t ReadUint32FromBuffer(
         const uint8_t* pBuffer)
{
   return (uint32_t)((((uint32_t)pBuffer[0]) << 24)
   | (((uint32_t)pBuffer[1]) << 16)
   | (((uint32_t)pBuffer[2]) << 8)
   | ((uint32_t)pBuffer[3]));
}
