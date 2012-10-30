/*
 * snep.c
 *
 *  Created on: 30-okt-2012
 *      Author: sam
 */

#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snep.h"


struct snep_message *snep_unpack(const uint8_t *buffer, size_t len) {
	struct snep_message *msg;

	if ((msg = malloc (sizeof *msg))) {
		msg->major_version = (buffer[SNEP_VERSION_POS] & 0xF0) >> 4;
		msg->minor_version = buffer[SNEP_VERSION_POS] & 0x0F;
		msg->type_field = buffer[SNEP_TYPE_POS];
		msg->data_length = ReadUint32FromBuffer(&buffer[SNEP_LENGTH_POS]);
		msg->ndef_message = memdup (&buffer[SNEP_HEADER_LENGTH], msg->data_length);
	}

	return msg;
}

uint8_t *snep_create_success_response(int *length) {

	//buffer = { SNEP_VERSION, RESPONSE_SUCCESS, 0x00, 0x00, 0x00, 0x00 };
	//buffer = malloc(6 * sizeof(uint8_t));

	uint8_t *buffer = malloc(6);

	buffer[0] = SNEP_VERSION;
	buffer[1] = RESPONSE_SUCCESS;
	buffer[2] = 0x00;
	buffer[3] = 0x00;
	buffer[4] = 0x00;
	buffer[5] = 0x00;

	length = 6;
	return buffer;
}

static inline uint8_t * memdup (const uint8_t *mem, size_t len)
{
    uint8_t *res = NULL;

    if (mem && (res = malloc (len))) {
	memcpy (res, mem, len);
    }

    return res;
}


uint32_t ReadUint32FromBuffer(
         const uint8_t* pBuffer)
{
   return (uint32_t)((((uint32_t)pBuffer[0]) << 24)
   | (((uint32_t)pBuffer[1]) << 16)
   | (((uint32_t)pBuffer[2]) << 8)
   | ((uint32_t)pBuffer[3]));
}
