//
//  base64.c
//  xdag
//
//  Created by Rui Xie on 11/16/18.
//  Copyright © 2018 xrdavies. All rights reserved.
//
#include <stdint.h>
#include "base64.h"
#include <stdlib.h>
#include <string.h>



static const uint8_t bits2mime[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const uint8_t mime2bits[256] = {
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x3e,0xff,0xff,0xff,0x3f,
	0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,
	0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0xff,0xff,0xff,0xff,0xff,
	0xff,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,
	0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x30,0x31,0x32,0x33,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
};

int encode(const uint8_t *in, size_t inlen, char *out);
int decode(const uint8_t *in, size_t inlen, uint8_t *out, size_t *outlen);

int encode(const uint8_t *in, size_t inlen, char *out)
{
	while(inlen) {
		*out++ = bits2mime[in[0] >> 2];
		*out++ = bits2mime[(in[0] << 4 & 0x3f) | (--inlen ? in[1] >> 4 : 0)];
		if (!inlen) {
			*out++ = '=';
			*out++ = '=';
			break;
		}
		*out++ = bits2mime[(in[1] << 2 & 0x3f) | (--inlen ? in[2] >> 6 : 0)];
		if(!inlen) {
			*out++ = '=';
			break;
		}
		*out++ = bits2mime[in[2] & 0x3f];
		--inlen;

		in += 3;
	}
	*out = '\0';

	return 0;
}

int decode(const uint8_t *in, size_t inlen, uint8_t *out, size_t *outlen)
{
	while(inlen) {
		if(mime2bits[in[0]] == 0xFF || mime2bits[in[1]] == 0xFF) {
			return -1;
		}
		*out++ = mime2bits[in[0]] << 2 | mime2bits[in[1]] >> 4;
		(*outlen)++;
		if(in[2] == '=') {
			break;
		}
		*out++ = mime2bits[in[1]] << 4 | mime2bits[in[2]] >> 2;
		(*outlen)++;

		if(in[3] == '=') {
			break;
		}
		*out++ = mime2bits[in[2]] << 6 | mime2bits[in[3]];
		(*outlen)++;
		in += 4;
		inlen -= 4;
	}

	return 0;
}

int base64_encode(const uint8_t *in, size_t inlen, char **out, size_t *outlen)
{
	*outlen = BASE64_LENGTH(inlen) + 1; /* extra byte for '\0' */
	*out = (char *)malloc(*outlen);
	memset((void*)(*out), 0, *outlen);
	int ret = encode(in, inlen, *out);
	if(ret != 0) {
		free(*out);
		*out = NULL;
		return ret;
	}
	return ret;
}

int base64_decode(const uint8_t *in, size_t inlen, uint8_t **out, size_t *outlen)
{
	if(inlen % 4) { // wrong inlen
		*outlen = 0;
		return -1;
	}

	/* This may allocate a few bytes too much, depending on input,
	 but it's not worth the extra CPU time to compute the exact amount.
	 The exact amount is 3 * inlen / 4, minus 1 if the input ends
	 with "=" and minus another 1 if the input ends with "==".
	 Dividing before multiplying avoids the possibility of overflow. */

	*out = malloc(3 * (inlen / 4) + 2);
	memset(*out, 0, 3 * (inlen / 4) + 2);
	int ret = decode(in, inlen, *out, outlen);
	if(ret != 0) {
		free(*out);
		*out = NULL;
	}
	return ret;
}
