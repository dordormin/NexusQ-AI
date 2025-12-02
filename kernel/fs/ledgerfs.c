/*
 * NexusQ-AI Kernel - LedgerFS Implementation
 * File: kernel/fs/ledgerfs.c
 */

#include "../memory/include/sys/ledgerfs.h"
#include "../../lib/include/nexus.h"
#include "../memory/include/sys/kalloc.h" // Utilisation de l'allocateur noyau
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Fonctions Privées (Crypto Simulation) ---

#include "../../modules/crypto/include/sha256.h"

// Helper to wrap SHA256 for a buffer
static void compute_sha256(const uint8_t *data, size_t len, uint8_t *out_hash) {
  Nexus_SHA256_CTX ctx;
  sha256_init(&ctx);
  sha256_update(&ctx, data, len);
  sha256_final(&ctx, out_hash);
}

// --- Implémentation API ---

// --- Global Registry (Simple Array for now) ---
#define MAX_FILES 128
lfs_inode_t *global_file_registry[MAX_FILES];
int global_file_count = 0; // Removed static to expose to syscalls

// --- Implémentation AP I ---

// Forward declaration
void lfs_load_disk(void);

void lfs_init(void) {
  printf("[LFS] LedgerFS Initialized. Blockchain Anchor Ready.\n");
  memset(global_file_registry, 0, sizeof(global_file_registry));
  global_file_count = 0;
  lfs_load_disk();
}

lfs_inode_t *lfs_find_by_name(const char *name) {
  for (int i = 0; i < global_file_count; i++) {
    if (global_file_registry[i] &&
        strcmp(global_file_registry[i]->filename, name) == 0) {
      return global_file_registry[i];
    }
  }
  return NULL;
}

lfs_inode_t *lfs_find_by_id(uint32_t id) {
  for (int i = 0; i < global_file_count; i++) {
    if (global_file_registry[i] && global_file_registry[i]->inode_id == id) {
      return global_file_registry[i];
    }
  }
  return NULL;
}

int lfs_get_file_list(nexus_file_entry_t *out_list, int max) {
  // Legacy wrapper for root (or all files)
  // Actually, we should change this to list ROOT (0) by default or ALL?
  // Let's keep it listing ALL for now to not break existing tests,
  // BUT we will add lfs_list_directory for the shell.
  int count = 0;
  for (int i = 0; i < global_file_count && count < max; i++) {
    if (global_file_registry[i]) {
      strncpy(out_list[count].name, global_file_registry[i]->filename, 63);
      out_list[count].size = global_file_registry[i]->size;
      out_list[count].is_sealed = global_file_registry[i]->is_sealed;
      // Preview hash (first 4 bytes hex)
      sprintf(out_list[count].hash_preview, "%02X%02X%02X%02X",
              global_file_registry[i]->content_hash[0],
              global_file_registry[i]->content_hash[1],
              global_file_registry[i]->content_hash[2],
              global_file_registry[i]->content_hash[3]);
      count++;
    }
  }
  return count;
}

int lfs_list_directory(uint32_t dir_id, void *list_ptr, int max) {
  nexus_file_entry_t *out_list = (nexus_file_entry_t *)list_ptr;
  int count = 0;
  for (int i = 0; i < global_file_count && count < max; i++) {
    lfs_inode_t *node = global_file_registry[i];
    if (node && node->parent_id == dir_id) {
      strncpy(out_list[count].name, node->filename, 63);
      out_list[count].size = node->size;
      out_list[count].is_sealed = node->is_sealed;
      out_list[count].id = node->inode_id; // Populate ID

      // Marker for Directory in Hash Preview
      if (node->type == LFS_TYPE_DIR) {
        strcpy(out_list[count].hash_preview, "<DIR>");
      } else {
        sprintf(out_list[count].hash_preview, "%02X%02X%02X%02X",
                node->content_hash[0], node->content_hash[1],
                node->content_hash[2], node->content_hash[3]);
      }
      count++;
    }
  }
  return count;
}

int lfs_read_file(const char *name, void *buffer, int max_size) {
  for (int i = 0; i < global_file_count; i++) {
    if (global_file_registry[i] &&
        strcmp(global_file_registry[i]->filename, name) == 0) {
      int copy_size = (global_file_registry[i]->size < max_size)
                          ? global_file_registry[i]->size
                          : max_size;
      memcpy(buffer, global_file_registry[i]->data, copy_size);
      return copy_size;
    }
  }
  return -1; // Not found
}

int lfs_delete_file(const char *name, const char *requestor) {
  for (int i = 0; i < global_file_count; i++) {
    if (global_file_registry[i] &&
        strcmp(global_file_registry[i]->filename, name) == 0) {
      // Governance Check: Ownership
      if (strcmp(global_file_registry[i]->owner_id, requestor) != 0 &&
          strcmp(requestor, "SYSTEM") != 0) {
        printf("[LFS] ACCESS DENIED: '%s' cannot delete '%s'. Owner: %s, "
               "Requestor: %s\n",
               requestor, name, global_file_registry[i]->owner_id, requestor);
        return -2; // Access Denied
      }

      // "Delete" by removing from registry and freeing memory
      kfree(global_file_registry[i]->data);
      kfree(global_file_registry[i]);

      // Compact list: Shift remaining elements left
      for (int j = i; j < global_file_count - 1; j++) {
        global_file_registry[j] = global_file_registry[j + 1];
      }
      global_file_registry[global_file_count - 1] = NULL;
      global_file_count--;

      printf("[LFS] Deleted File '%s' by '%s'.\n", name, requestor);
      return 0;
    }
  }
  printf("[LFS] Error: File '%s' not found for deletion.\n", name);
  return -1; // Not found
}

lfs_inode_t *lfs_create_file(const char *name, const void *data, int size,
                             uint32_t parent_id, const char *owner) {
  // Check if file exists IN THE SAME DIRECTORY
  lfs_inode_t *existing = NULL;
  for (int i = 0; i < global_file_count; i++) {
    if (global_file_registry[i] &&
        strcmp(global_file_registry[i]->filename, name) == 0 &&
        global_file_registry[i]->parent_id == parent_id) {
      existing = global_file_registry[i];
      break;
    }
  }

  if (existing) {
    // Overwrite Logic
    // Check permissions (simplified)
    if (owner && strcmp(existing->owner_id, owner) != 0 &&
        strcmp(owner, "SYSTEM") != 0) {
      printf("[LFS] Error: Permission denied to overwrite '%s'.\n", name);
      return NULL;
    }

    // Update Content
    if (existing->size != size) {
      kfree(existing->data);
      existing->data = kmalloc(size);
      existing->size = size;
    }

    if (existing->data) {
      memcpy(existing->data, data, size);
      compute_sha256((const uint8_t *)existing->data, size,
                     existing->content_hash);
      printf("[LFS] Overwrote File '%s' (ID: %d, Size: %d bytes)\n", name,
             existing->inode_id, size);
      return existing;
    } else {
      return NULL; // Alloc failed
    }
  }

  // Allocation inode via kmalloc (Module 1)
  lfs_inode_t *inode = (lfs_inode_t *)kmalloc(sizeof(lfs_inode_t));
  if (!inode)
    return NULL;

  // Métadonnées
  static uint32_t next_id = 1000;
  inode->inode_id = next_id++;
  strncpy(inode->filename, name, 63);
  inode->size = size;
  inode->is_sealed = 0; // Pas encore sécurisé
  inode->type = LFS_TYPE_FILE;
  inode->parent_id = parent_id; // Use passed parent_id
  if (owner) {
    strncpy(inode->owner_id, owner, 63);
  } else {
    strcpy(inode->owner_id, "SYSTEM");
  }

  // Stockage Données (Allocation brute)
  inode->data = kmalloc(size);
  if (!inode->data)
    return NULL; // Leak inode ici dans un vrai OS
  memcpy(inode->data, data, size);

  // Calcul du Hash initial (Genesis)
  compute_sha256((const uint8_t *)inode->data, size, inode->content_hash);

  // 5. Register
  if (global_file_count < MAX_FILES) {
    global_file_registry[global_file_count++] = inode;
  } else {
    printf("[LFS] Error: Registry full. File created but not tracked.\n");
  }

  printf("[LFS] Created File '%s' (ID: %d, Size: %d bytes, Owner: %s)\n", name,
         inode->inode_id, size, inode->owner_id);
  return inode;
}

lfs_inode_t *lfs_create_directory(const char *name, uint32_t parent_id,
                                  const char *owner) {
  if (global_file_count >= MAX_FILES) {
    printf("[LFS] Error: Registry full.\n");
    return NULL;
  }

  lfs_inode_t *node = (lfs_inode_t *)kmalloc(sizeof(lfs_inode_t));
  if (!node)
    return NULL;

  static uint32_t next_id = 2000; // Start dirs at 2000
  node->inode_id = next_id++;
  strncpy(node->filename, name, 63);
  node->size = 0;
  node->is_sealed = 0;
  node->type = LFS_TYPE_DIR;
  node->parent_id = parent_id;
  node->data = NULL; // Dirs don't have data content in this simple model

  if (owner)
    strncpy(node->owner_id, owner, 63);
  else
    strcpy(node->owner_id, "SYSTEM");

  // Register
  global_file_registry[global_file_count++] = node;

  printf("[LFS] Created Directory '%s' (ID: %d, Parent: %d)\n", name,
         node->inode_id, parent_id);
  return node;
}

void lfs_seal(lfs_inode_t *inode) {
  inode->is_sealed = 1;
  printf("[LFS] SEALED File '%s'. Integrity Locked.\n", inode->filename);
  lfs_print_hash(" -> Ledger Hash", inode->content_hash);
}

int lfs_write(lfs_inode_t *inode, const uint8_t *data, size_t size) {
  if (inode->is_sealed) {
    // Dans NexusQ, on ne modifie pas un binaire scellé. On doit créer une
    // nouvelle version.
    printf("[LFS] ACCESS DENIED: Cannot write to Sealed File '%s' "
           "(Immutability Violation).\n",
           inode->filename);
    return -1;
  }

  // Réallocation simplifiée (suppose que ça rentre ou écrase)
  if ((int)size > inode->size) {
    // Dans ce proto simple, on refuse l'extension pour éviter complexité
    // realloc
    printf("[LFS] Error: New size too large for static block.\n");
    return -1;
  }

  memcpy(inode->data, data, size);
  inode->size = size;

  // Recalcul du Hash automatique (Chain Update)
  compute_sha256((const uint8_t *)inode->data, size, inode->content_hash);

  printf("[LFS] Wrote to '%s'. Hash Updated.\n", inode->filename);
  return 0;
}

int lfs_append_file(const char *name, const void *data, int size) {
  lfs_inode_t *inode = lfs_find_by_name(name);
  if (!inode)
    return -1;

  if (inode->is_sealed) {
    printf("[LFS] Error: Cannot append to sealed file '%s'.\n", name);
    return -1;
  }

  int new_size = inode->size + size;
  uint8_t *new_data = kmalloc(new_size);
  if (!new_data)
    return -1;

  if (inode->data) {
    memcpy(new_data, inode->data, inode->size);
    kfree(inode->data);
  }
  memcpy(new_data + inode->size, data, size);

  inode->data = new_data;
  inode->size = new_size;

  compute_sha256((const uint8_t *)inode->data, new_size, inode->content_hash);
  // printf("[LFS] Appended %d bytes to '%s'. New Size: %d\n", size, name,
  // new_size);
  return 0;
}

int lfs_verify_integrity(lfs_inode_t *inode) {
  uint8_t current_hash[HASH_SIZE];

  // 1. On scanne le contenu physique actuel sur le disque/RAM

  compute_sha256((const uint8_t *)inode->data, inode->size, current_hash);

  // 2. On compare avec le Hash Scellé dans l'Inode
  if (memcmp(current_hash, inode->content_hash, HASH_SIZE) == 0) {
    printf("[LFS] Integrity OK for '%s'\n", inode->filename);
    return 0; // Intègre
  } else {
    printf("[LFS] !!! SECURITY ALERT !!! Integrity Mismatch on '%s'\n",
           inode->filename);
    lfs_print_hash(" -> Expected", inode->content_hash);
    lfs_print_hash(" -> Computed", current_hash);
    return -1; // Corrompu
  }
}

void lfs_print_hash(const char *label, uint8_t *hash) {
  printf("%s: [", label);
  for (int i = 0; i < 8; i++)
    printf("%02X", hash[i]); // Affiche juste les 8 premiers octets
  printf("...]\n");
}

// --- Persistence (Disk Backend) ---
#define DISK_FILE "nexus_disk.dat"

void lfs_save_disk(void) {
  FILE *fp = fopen(DISK_FILE, "wb");
  if (!fp) {
    printf("[LFS] Error: Cannot write to disk '%s'\n", DISK_FILE);
    return;
  }

  // Write File Count
  fwrite(&global_file_count, sizeof(int), 1, fp);

  // Write Each File
  for (int i = 0; i < global_file_count; i++) {
    lfs_inode_t *node = global_file_registry[i];
    if (node) {
      // Write Inode Metadata
      fwrite(node, sizeof(lfs_inode_t), 1, fp);
      // Write Data
      if (node->size > 0 && node->data) {
        fwrite(node->data, node->size, 1, fp);
      }
    }
  }

  fclose(fp);
  printf("[LFS] System State Saved to '%s'.\n", DISK_FILE);
}

void lfs_load_disk(void) {
  FILE *fp = fopen(DISK_FILE, "rb");
  if (!fp) {
    printf("[LFS] No previous state found. Starting fresh.\n");
    return;
  }

  // Read File Count
  int count = 0;
  fread(&count, sizeof(int), 1, fp);

  printf("[LFS] Loading %d files from disk...\n", count);

  for (int i = 0; i < count; i++) {
    if (global_file_count >= MAX_FILES)
      break;

    lfs_inode_t *node = (lfs_inode_t *)kmalloc(sizeof(lfs_inode_t));
    if (!node)
      break;

    // Read Inode Metadata
    fread(node, sizeof(lfs_inode_t), 1, fp);

    // Allocate and Read Data
    if (node->size > 0) {
      node->data = (uint8_t *)kmalloc(node->size);
      if (node->data) {
        fread(node->data, node->size, 1, fp);
      } else {
        node->size = 0; // Allocation failed
      }
    } else {
      node->data = NULL;
    }

    global_file_registry[global_file_count++] = node;
  }

  fclose(fp);
  printf("[LFS] State Loaded. Registry restored.\n");
}

int lfs_rename_file(const char *old_name, const char *new_name,
                    const char *requestor) {
  lfs_inode_t *inode = lfs_find_by_name(old_name);
  if (!inode) {
    printf("[LFS] Error: File '%s' not found for rename.\n", old_name);
    return -1;
  }

  // Check if new name already exists
  if (lfs_find_by_name(new_name)) {
    printf("[LFS] Error: Destination '%s' already exists.\n", new_name);
    return -1;
  }

  // Governance Check
  if (strcmp(inode->owner_id, requestor) != 0 &&
      strcmp(requestor, "SYSTEM") != 0) {
    printf("[LFS] ACCESS DENIED: '%s' cannot rename '%s'. Owner: %s\n",
           requestor, old_name, inode->owner_id);
    return -2;
  }

  if (inode->is_sealed) {
    printf("[LFS] Error: Cannot rename sealed file '%s'.\n", old_name);
    return -1;
  }

  strncpy(inode->filename, new_name, 63);
  printf("[LFS] Renamed '%s' to '%s'.\n", old_name, new_name);
  return 0;
}
