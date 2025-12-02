; NexusQ-AI Crypto Module - Fast Hash (ASM)
; Used for high-speed block verification.
; File: modules/crypto/arch/x86/sha256_core.asm

global nexus_fast_hash_asm

section .text

; uint32_t nexus_fast_hash_asm(const uint8_t *data, int len)
; RDI = data
; RSI = len
nexus_fast_hash_asm:
    push rbp
    mov rbp, rsp
    
    mov eax, 0xCAFEBABE ; Initial Seed
    
    test rsi, rsi
    jz .done
    
.loop:
    movzx ecx, byte [rdi] ; Load byte
    
    ; Mix function: (hash << 5) - hash + byte
    mov edx, eax
    shl edx, 5
    sub edx, eax
    add edx, ecx
    mov eax, edx
    
    inc rdi
    dec rsi
    jnz .loop
    
.done:
    mov rsp, rbp
    pop rbp
    ret
