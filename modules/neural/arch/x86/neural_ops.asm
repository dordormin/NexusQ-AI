; NexusQ-AI Neural Engine - Optimized Operations (ASM)
; File: modules/neural/arch/x86/neural_ops.asm

global neural_dot_product_asm

section .text

; float neural_dot_product_asm(float *weights, float *inputs, int count)
; RDI = weights
; RSI = inputs
; RDX = count
; Returns result in XMM0
neural_dot_product_asm:
    push rbp
    mov rbp, rsp
    
    vxorps xmm0, xmm0, xmm0 ; Accumulator = 0.0
    
    test rdx, rdx
    jz .done
    
.loop:
    ; Load float from weights
    vmovss xmm1, [rdi]
    ; Load float from inputs
    vmovss xmm2, [rsi]
    
    ; Multiply: xmm1 = w * i
    vmulss xmm1, xmm1, xmm2
    
    ; Add to accumulator: xmm0 += xmm1
    vaddss xmm0, xmm0, xmm1
    
    ; Advance pointers (4 bytes per float)
    add rdi, 4
    add rsi, 4
    
    dec rdx
    jnz .loop
    
.done:
    mov rsp, rbp
    pop rbp
    ret
