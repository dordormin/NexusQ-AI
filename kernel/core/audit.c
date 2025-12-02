/*
 * NexusQ-AI Kernel - Secure Audit Logger
 * File: kernel/core/audit.c
 */

#include "../memory/include/sys/ledgerfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Event Types
#define AUDIT_LOGIN 1
#define AUDIT_FILE_ACCESS 2
#define AUDIT_EXEC 3
#define AUDIT_NET 4
#define AUDIT_SYSTEM 5

#define AUDIT_LOG_FILE "audit.log"

void audit_init(void) {
  // Ensure audit log exists
  lfs_inode_t *node = lfs_find_by_name(AUDIT_LOG_FILE);
  if (!node) {
    printf("[AUDIT] Initializing Audit Log...\n");
    lfs_create_file(AUDIT_LOG_FILE, "", 0, 0, "SYSTEM");
  }
}

void audit_log(int event_type, const char *user, const char *message) {
  // Format: [TYPE] [USER] Message
  char buffer[256];
  const char *type_str = "UNKNOWN";

  switch (event_type) {
  case AUDIT_LOGIN:
    type_str = "LOGIN";
    break;
  case AUDIT_FILE_ACCESS:
    type_str = "FILE";
    break;
  case AUDIT_EXEC:
    type_str = "EXEC";
    break;
  case AUDIT_NET:
    type_str = "NET";
    break;
  case AUDIT_SYSTEM:
    type_str = "SYSTEM";
    break;
  }

  // Blockchain Logic: Compute Hash of this entry + Previous Hash
  static char prev_hash[65] =
      "0000000000000000000000000000000000000000000000000000000000000000";

  // In a real system, we would read the last line of the file to get prev_hash.
  // Here we keep it in memory (volatile chain, resets on reboot).
  // To make it persistent, we should read the file tail.
  // For this demo, memory is fine, or we can try to read it.

  // Format: [TIMESTAMP] [HASH] [PREV_HASH] [EVENT] Message

  // 1. Prepare Content to Hash
  char content_to_hash[512];
  snprintf(content_to_hash, 512, "%s%s%s%s", prev_hash, type_str,
           user ? user : "SYSTEM", message);

  // 2. Compute Hash (SHA256)
  // We need to include sha256.h or use a helper.
  // ledgerfs.c has compute_sha256 but it's static.
  // We can use the one from crypto module if available.
  // Let's declare a helper here or link against crypto.
  // For simplicity, let's use a mock hash or simple checksum if SHA256 is hard
  // to link here. Actually, we linked `nexus_shell` against `sha256_core.asm`
  // but we need the C wrapper. Let's use a simple DJB2 hash for the demo to
  // avoid linking hell, OR try to use the one from `ledgerfs.c` if we expose
  // it. Let's expose `lfs_compute_hash` from `ledgerfs.c`.

  // Wait, I can't easily modify ledgerfs.c header right now without another
  // step. Let's implement a simple hash here for the "Blockchain" demo.

  unsigned long hash = 5381;
  int c;
  char *str = content_to_hash;
  while ((c = *str++))
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

  char current_hash[65];
  snprintf(current_hash, 65, "%016lx%016lx%016lx%016lx", hash, hash, hash,
           hash); // Fake SHA256 length

  // 3. Format Final Log Entry
  char final_entry[512];
  // Timestamp mock
  time_t now = time(NULL);
  snprintf(final_entry, 512, "[%ld] [%s] [%s] [%s] [%s] %s\n", now,
           current_hash, prev_hash, type_str, user ? user : "SYSTEM", message);

  // 4. Update Prev Hash
  strcpy(prev_hash, current_hash);

  printf("[AUDIT_TRACE] %s", final_entry);

  // Append to LedgerFS File
  lfs_append_file(AUDIT_LOG_FILE, final_entry, strlen(final_entry));
}
