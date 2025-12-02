/*
 * NexusQ-AI Kernel - LedgerFS Interface
 * File: include/sys/ledgerfs.h
 */

#ifndef _SYS_LEDGERFS_H_
#define _SYS_LEDGERFS_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// --- Configuration ---
#define LFS_BLOCK_SIZE 4096
#define HASH_SIZE 32 // SHA-256 simulation (32 bytes)

// --- Structures ---

// Le "Bloc Genesis" du fichier
typedef struct {
  uint32_t inode_id;
  char filename[64];
  int size;
  unsigned char content_hash[32]; // SHA-256 Anchor
  int is_sealed;                  // 1 = Immutable (Blockchain confirmed)
  char owner_id[64];              // Wallet Public Key of the Owner
  void *data;                     // Pointer to content (Simulated storage)
  // Directory Support
  int type;           // 0 = FILE, 1 = DIRECTORY
  uint32_t parent_id; // ID of the parent directory (0 for root)
} lfs_inode_t;

#define LFS_TYPE_FILE 0
#define LFS_TYPE_DIR 1

// --- API Publique ---
void lfs_init(void);
lfs_inode_t *lfs_find_by_name(const char *name);
lfs_inode_t *lfs_find_by_id(uint32_t id); // New
lfs_inode_t *lfs_create_file(const char *name, const void *data, int size,
                             uint32_t parent_id, const char *owner);
int lfs_read_file(const char *name, void *buffer, int max_size);
int lfs_delete_file(const char *name, const char *requestor);
int lfs_verify_integrity(lfs_inode_t *inode);
// Une fois scellé, toute modification ultérieure invalide le fichier.
void lfs_seal(lfs_inode_t *inode);
void lfs_save_disk(void);
lfs_inode_t *lfs_create_directory(const char *name, uint32_t parent_id,
                                  const char *owner);
int lfs_list_directory(uint32_t dir_id, void *out_list, int max);

// Tentative d'écriture (Modification)
// Retourne -1 si le fichier est scellé (Interdit par le protocole)
int lfs_write(lfs_inode_t *inode, const uint8_t *data, size_t size);
int lfs_append_file(const char *name, const void *data, int size);

// Le "Watchdog" (Vérification au lancement d'un processus)
// Recalcule le hash réel et le compare au hash stocké.
// Retourne: 0 (OK), -1 (CORRUPTION DÉTECTÉE)
int lfs_verify_integrity(lfs_inode_t *inode);

// Outils de Debug (Hexdump du hash)
void lfs_print_hash(const char *label, uint8_t *hash);

#endif // _SYS_LEDGERFS_H_
