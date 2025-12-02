/*
 * NexusQ-AI Wallet Implementation
 * File: modules/crypto/wallet/wallet.c
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/wallet.h"

// Helper pour générer des strings aléatoires
static void random_string(char* str, int len) {
    const char charset[] = "0123456789ABCDEF";
    for (int i = 0; i < len - 1; i++) {
        int key = rand() % (int)(sizeof(charset) - 1);
        str[i] = charset[key];
    }
    str[len - 1] = '\0';
}

void wallet_create(wallet_t* out_wallet) {
    // Génération de clés simulées (Hex strings)
    random_string(out_wallet->public_key, PUBKEY_SIZE);
    random_string(out_wallet->private_key, PRIVKEY_SIZE);
    
    printf("[WALLET] Created New Identity.\n");
    printf(" -> PubKey: %.16s...\n", out_wallet->public_key);
}

void wallet_sign(wallet_t* wallet, const char* message, char* out_signature) {
    // Simulation de signature : SIGNED[Message]BY[PrivKey]
    // Dans un vrai système, ce serait RSA ou ECDSA.
    snprintf(out_signature, SIGNATURE_SIZE, "SIG_%.8s_BY_%.8s", message, wallet->private_key);
}

int wallet_verify(const char* public_key, const char* message, const char* signature) {
    // Vérification simulée
    // On vérifie juste que la signature commence par "SIG_"
    // Dans un vrai système, on utiliserait la clé publique pour décrypter/vérifier le hash.
    
    if (strncmp(signature, "SIG_", 4) == 0) {
        return 0; // Valide
    }
    return -1; // Invalide
}
