; kernel/arch/x86/isr.asm (Interrupt Service Routine)

global syscall_handler
extern syscall_table
extern num_syscalls

; C'est ici que le CPU arrive quand une app fait "INT 0x80"
syscall_handler:
    push eax                ; Sauvegarde des registres (Context Switch)
    
    ; 1. Gouvernance : Vérification des Limites
    cmp eax, [num_syscalls] ; Le numéro demandé est-il valide ?
    ja .invalid_syscall     ; Si > max, on rejette (Sécurité)

    ; 2. Gouvernance : Appel de la fonction officielle
    call [syscall_table + eax*4]
    
    ; 3. Retour au citoyen
    mov [esp], eax          ; On place le résultat dans la stack (overwrite saved eax)
    pop eax
    iretd                   ; Retour en Ring 3 (User Space)

.invalid_syscall:
    mov eax, -1             ; Code d'erreur "Operation Interdite"
    iretd