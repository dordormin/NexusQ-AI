/*
 * NexusQ-AI Smart Contract Interface
 * File: modules/contracts/include/contract.h
 */

#ifndef _CONTRACT_H_
#define _CONTRACT_H_

#include <stdint.h>

// --- VM Opcodes ---
#define OP_EXIT         0x00 // Stop execution
#define OP_PUSH         0x01 // Push value to stack
#define OP_POP          0x02 // Pop value from stack
#define OP_ADD          0x03 // Add top two values
#define OP_SUB          0x04 // Subtract top two values
#define OP_EQ           0x05 // Check equality
#define OP_GET_SENDER   0x10 // Push Sender's Wallet ID Hash to stack
#define OP_CHECK_SIG    0x11 // Verify signature (Simulated)
#define OP_GRANT_ACCESS 0xFF // Success (Return 1)

// --- Contract Structure ---
#define MAX_BYTECODE_SIZE 256

typedef struct {
    uint8_t bytecode[MAX_BYTECODE_SIZE];
    uint32_t size;
    char name[32];
} contract_t;

// --- API ---
// Exécute un contrat dans le contexte du processus appelant
// Retourne 1 si succès (Accès accordé), 0 sinon.
int vm_exec(contract_t* contract, const char* sender_pubkey);

#endif // _CONTRACT_H_
