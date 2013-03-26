/*
 * ndef.h
 *
 *  Created on: 6-nov-2012
 *      Author: Sam Van Den  Berge
 */

#ifndef NDEF_H_
#define NDEF_H_

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#include <stdint.h>
#define SR_MASK 		0b00010000
#define MB_MASK			0b10000000
#define ME_MASK			0b01000000
#define CF_MASK			0b00100000
#define IL_MASK			0b00001000
#define TNF_MASK		0b00000111

struct ndef_record {
	uint8_t MB;
	uint8_t ME;
	uint8_t CF;
	uint8_t SR;
	uint8_t IL;
	uint8_t TNF;
	uint8_t type_length;
	uint32_t payload_length;
	uint8_t ID_length;
	uint8_t *type;
	uint8_t *ID;
	uint8_t *payload;
};

struct ndef_record *ndef_unpack(const uint8_t *buffer, size_t len);
int ndef_pack(struct ndef_record *record, uint8_t *buffer);
uint32_t ReadUint32FromBuffer(const uint8_t* pBuffer);

static inline uint8_t * memdup (const uint8_t *mem, size_t len)
{
    uint8_t *res = NULL;

    if (mem && (res = malloc (len))) {
	memcpy (res, mem, len);
    }

    return res;
}

#endif /* NDEF_H_ */




