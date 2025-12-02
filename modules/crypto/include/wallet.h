/*
 * NexusQ-AI Wallet & Identity Interface
 * File: modules/crypto/include/wallet.h
 */

#ifndef _WALLET_H_
#define _WALLET_H_

#include <stdint.h>

// Taille des clés (Simulé pour l'instant, on utilise des strings)
#define PUBKEY_SIZE 64
#define PRIVKEY_SIZE 64
#define SIGNATURE_SIZE 128

typedef struct {
    char public_key[PUBKEY_SIZE];
    char private_key[PRIVKEY_SIZE];
} wallet_t;

// --- API ---

// Crée un nouveau wallet (Génère une paire de clés)
void wallet_create(wallet_t* out_wallet);

// Signe un message (Retourne une signature simulée)
void wallet_sign(wallet_t* wallet, const char* message, char* out_signature);

// Vérifie une signature (Retourne 0 si valide, -1 sinon)
int wallet_verify(const char* public_key, const char* message, const char* signature);

#endif // _WALLET_H_
