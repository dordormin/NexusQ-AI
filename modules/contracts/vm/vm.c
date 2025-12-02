/*
 * NexusQ-AI Smart Contract VM
 * File: modules/contracts/vm/vm.c
 */

#include <stdio.h>
#include <string.h>
#include "../include/contract.h"

#define STACK_SIZE 64

int vm_exec(contract_t* contract, const char* sender_pubkey) {
    int stack[STACK_SIZE];
    int sp = -1; // Stack Pointer
    uint32_t ip = 0; // Instruction Pointer
    
    printf("[VM] Executing Contract '%s' (Sender: %.8s...)\n", contract->name, sender_pubkey);
    
    while (ip < contract->size) {
        uint8_t opcode = contract->bytecode[ip++];
        
        switch (opcode) {
            case OP_EXIT:
                printf("[VM] OP_EXIT\n");
                return 0; // Default fail
                
            case OP_PUSH: {
                // Simple: Push next byte as value
                int val = contract->bytecode[ip++];
                stack[++sp] = val;
                // printf("[VM] OP_PUSH %d\n", val);
                break;
            }
            
            case OP_GET_SENDER: {
                // Pour ce prototype, on push un hash simpliste du sender
                // Si sender commence par 'S' (SYSTEM), on push 1, sinon 0
                int val = (sender_pubkey[0] == 'S') ? 1 : 0;
                stack[++sp] = val;
                printf("[VM] OP_GET_SENDER -> %d\n", val);
                break;
            }
            
            case OP_EQ: {
                int a = stack[sp--];
                int b = stack[sp--];
                stack[++sp] = (a == b) ? 1 : 0;
                printf("[VM] OP_EQ (%d == %d) -> %d\n", a, b, stack[sp]);
                break;
            }
            
            case OP_GRANT_ACCESS:
                printf("[VM] OP_GRANT_ACCESS triggered.\n");
                return 1; // Success
                
            default:
                printf("[VM] Unknown Opcode: 0x%02X\n", opcode);
                return 0;
        }
    }
    
    return 0; // End of code without Grant
}
