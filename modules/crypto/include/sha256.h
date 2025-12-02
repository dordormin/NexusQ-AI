/*
 * NexusQ-AI Crypto Module - SHA256 Interface
 * File: modules/crypto/include/sha256.h
 */

#ifndef _NEXUS_SHA256_H_
#define _NEXUS_SHA256_H_

#include <stdint.h>
#include <stddef.h>

#define SHA256_BLOCK_SIZE 32            // SHA256 outputs a 32 byte digest

typedef struct {
	uint8_t data[64];
	uint32_t datalen;
	unsigned long long bitlen;
	uint32_t state[8];
} Nexus_SHA256_CTX;

void sha256_init(Nexus_SHA256_CTX *ctx);
void sha256_update(Nexus_SHA256_CTX *ctx, const uint8_t *data, size_t len);
void sha256_final(Nexus_SHA256_CTX *ctx, uint8_t hash[SHA256_BLOCK_SIZE]);

#endif // _NEXUS_SHA256_H_
