/*
 * snep.h
 *
 *  Created on: 30-okt-2012
 *      Author: sam
 */

#ifndef SNEP_H_
#define SNEP_H_

#include <sys/types.h>

#include <stdint.h>

#define SNEP_VERSION_POS      			   0
#define SNEP_TYPE_POS         			   1
#define SNEP_LENGTH_POS       			   2
#define SNEP_HEADER_LENGTH   			   6

/* Request Field Values */
#define REQUEST_CONTINUE                   0x00
#define REQUEST_GET                        0x01
#define REQUEST_PUT                        0x02
#define REQUEST_REJECT                     0x7F

/* Response Field Values */
#define RESPONSE_CONTINUE                  0x80
#define RESPONSE_SUCCESS                   0x81
#define RESPONSE_NOT_FOUND                 0xC0
#define RESPONSE_EXCESS_DATA               0xC1
#define RESPONSE_BAD_REQUEST               0xC2
#define RESPONSE_NOT_IMPLEMENTED           0xE0
#define RESPONSE_UNSUPPORTED_VERSION       0xE1
#define RESPONSE_REJECT                    0xFF

#define SNEP_VERSION					   0x10

struct snep_message {
	uint8_t major_version;
	uint8_t minor_version;
	uint8_t type_field;
	uint32_t data_length;
	uint8_t *ndef_message;
};


uint32_t ReadUint32FromBuffer(const uint8_t* pBuffer);
struct snep_message *snep_unpack(const uint8_t *buffer, size_t len);
uint8_t *snep_create_success_response(int *length);
static inline uint8_t * memdup (const uint8_t *mem, size_t len);




#endif /* SNEP_H_ */
