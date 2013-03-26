/*
 * ndef.c
 *
 *  Created on: 6-nov-2012
 *      Author: Sam Van Den  Berge
 */

#include "ndef.h"
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//To enable the llcp_log_log function.
//Must be defined before the include of llcp_log.h !!!!!!!!!!!!
#define DEBUG 1

#include "llcp_log.h"

struct ndef_record *ndef_unpack(const uint8_t *buffer, size_t len) {

	struct ndef_record *record;
	if((record = malloc(sizeof *record))) {
		record->MB = (buffer[0] & MB_MASK) >> 7;
		llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_INFO, "[ndef.c] Message Begin: %i", record->MB);
		record->ME = (buffer[0] & ME_MASK) >> 6;
		llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_INFO, "[ndef.c] Message End: %i", record->ME);
		record->CF = (buffer[0] & CF_MASK) >> 5;
		llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_INFO, "[ndef.c] Chunk flag: %i", record->CF);
		record->SR = (buffer[0] & SR_MASK) >> 4;
		llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_INFO, "[ndef.c] Short record: %i", record->SR);
		record->IL = (buffer[0] & IL_MASK) >> 3;
		llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_INFO, "[ndef.c] ID Length present: %i", record->IL);
		record->TNF = (buffer[0] & TNF_MASK);
		llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_INFO, "[ndef.c] Type Name Format: %i", record->TNF);
		record->type_length = buffer[1];
		llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_INFO, "[ndef.c] Type length: %i", record->type_length);

		int payload_length_size = 1;

		if(record->SR == 0 ) {
			//This is not a short record => payload length = 32bit!
			record->payload_length = ReadUint32FromBuffer(buffer[2]);
			llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_INFO, "[ndef.c] Payload length (normal record): %i", record->payload_length);
			payload_length_size = 4;
		}
		else{
			//This is a short record => payload length = 8bit
			record->payload_length = buffer[2];
			llcp_log_log("[nfc-p2p-demo.c]", LLC_PRIORITY_INFO, "[ndef.c] Payload length (short record): %i", record->payload_length);
			payload_length_size = 1;
		}

		if(record->IL == 0 ) {
			//ID length not present. After payload length follows the type
			record->ID_length = -1;
			record->type = memdup(&buffer[2 + payload_length_size], record->type_length);
			record->payload = memdup(&buffer[2+payload_length_size+record->type_length], record->payload_length);
		}
		else{
			//ID length present ( 1 byte after payload length )
			record->ID_length = buffer[2 + payload_length_size];
			record->type = memdup(&buffer[3 + payload_length_size], record->type_length);
			record->payload = memdup(&buffer[3+payload_length_size+record->type_length], record->payload_length);
		}
	}

	return record;
}


int ndef_pack(struct ndef_record *record, uint8_t *buffer) {

	buffer[0] |= ((record->MB << 7) & MB_MASK) | ((record->ME << 6) & ME_MASK) | ((record->CF << 5) & CF_MASK) | ((record->SR << 4)& SR_MASK) | (record->TNF & TNF_MASK);
	buffer[1] = record->type_length;

	if(record->SR == 1 ) {
		buffer[2] = record->payload_length;
	}
	else{
		//TODO: implement if SR = 0 ( payload_length = 4 bytes ) NOTE: add ofset to all the rest
	}

	memcpy(&buffer[3], record->type, record->type_length);
	memcpy(&buffer[3+record->type_length], record->payload, record->payload_length);

	int len = (3 + record->type_length + record->payload_length);

	return len;
}

