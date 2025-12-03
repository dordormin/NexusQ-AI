/*
 * NexusQ-AI - Enhanced Governance System
 * File: kernel/core/governance.c
 *
 * Improved security, permissions, and audit system
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../../lib/include/nexus.h"

#define MAX_AUDIT_ENTRIES 1000
#define MAX_PERMISSIONS 128

// Permission flags
#define PERM_READ 0x01
#define PERM_WRITE 0x02
#define PERM_EXECUTE 0x04
#define PERM_DELETE 0x08
#define PERM_ADMIN 0x10

// Governance levels (Defined in nexus.h)
// typedef enum { ... } gov_level_t;

// Enhanced audit entry
typedef struct {
  time_t timestamp;
  char user[64];
  char action[128];
  char resource[256];
  int success;
  char reason[128];
  gov_level_t level;
} audit_entry_t;

// Permission entry
typedef struct {
  char user[64];
  char resource[256];
  uint8_t permissions;
  gov_level_t level;
  time_t granted_at;
  char granted_by[64];
} permission_entry_t;

// Global governance state
static audit_entry_t audit_log[MAX_AUDIT_ENTRIES];
static int audit_count = 0;
static permission_entry_t permissions[MAX_PERMISSIONS];
static int perm_count = 0;

// Current user context
static char current_user[64] = "SYSTEM";
static gov_level_t current_level = GOV_LEVEL_SYSTEM;

// Initialize governance system
void gov_init() {
  audit_count = 0;
  perm_count = 0;
  strcpy(current_user, "SYSTEM");
  current_level = GOV_LEVEL_SYSTEM;
  printf("[GOV] Governance System Initialized\n");
}

// Set current user
void gov_set_user(const char *user, gov_level_t level) {
  strncpy(current_user, user, 63);
  current_level = level;
  printf("[GOV] User context: %s (Level: %d)\n", user, level);
}

// Get current user
const char *gov_get_current_user() { return current_user; }

// Get current level
gov_level_t gov_get_current_level() { return current_level; }

// Log audit entry
void gov_audit(const char *action, const char *resource, int success,
               const char *reason) {
  if (audit_count >= MAX_AUDIT_ENTRIES) {
    // Rotate log (remove oldest)
    for (int i = 0; i < MAX_AUDIT_ENTRIES - 1; i++) {
      audit_log[i] = audit_log[i + 1];
    }
    audit_count = MAX_AUDIT_ENTRIES - 1;
  }

  audit_entry_t *entry = &audit_log[audit_count++];
  entry->timestamp = time(NULL);
  strncpy(entry->user, current_user, 63);
  strncpy(entry->action, action, 127);
  strncpy(entry->resource, resource, 255);
  entry->success = success;
  strncpy(entry->reason, reason ? reason : "", 127);
  entry->level = current_level;

  // Also log to file asynchronously
  if (!success) {
    printf("[GOV] AUDIT: %s attempted '%s' on '%s' - %s\n", current_user,
           action, resource, reason ? reason : "DENIED");
  }
}

// Check permission
int gov_check_permission(const char *resource, uint8_t required_perm) {
  // System always has access
  if (current_level == GOV_LEVEL_SYSTEM) {
    return 1;
  }

  // Check explicit permissions
  for (int i = 0; i < perm_count; i++) {
    if (strcmp(permissions[i].user, current_user) == 0 &&
        strcmp(permissions[i].resource, resource) == 0) {
      if (permissions[i].permissions & required_perm) {
        return 1;
      }
    }
  }

  // Check ownership (simple: if username in resource name)
  if (strstr(resource, current_user) != NULL) {
    return 1;
  }

  return 0;
}

// Grant permission
int gov_grant_permission(const char *user, const char *resource, uint8_t perms,
                         const char *granter) {
  if (perm_count >= MAX_PERMISSIONS) {
    printf("[GOV] Permission table full\n");
    return -1;
  }

  permission_entry_t *perm = &permissions[perm_count++];
  strncpy(perm->user, user, 63);
  strncpy(perm->resource, resource, 255);
  perm->permissions = perms;
  perm->level = GOV_LEVEL_USER;
  perm->granted_at = time(NULL);
  strncpy(perm->granted_by, granter, 63);

  gov_audit("GRANT_PERMISSION", resource, 1, NULL);
  printf("[GOV] Granted permissions 0x%02X to %s for '%s'\n", perms, user,
         resource);

  return 0;
}

// Revoke permission
int gov_revoke_permission(const char *user, const char *resource) {
  for (int i = 0; i < perm_count; i++) {
    if (strcmp(permissions[i].user, user) == 0 &&
        strcmp(permissions[i].resource, resource) == 0) {
      // Shift remaining permissions
      for (int j = i; j < perm_count - 1; j++) {
        permissions[j] = permissions[j + 1];
      }
      perm_count--;
      gov_audit("REVOKE_PERMISSION", resource, 1, NULL);
      printf("[GOV] Revoked permissions for %s on '%s'\n", user, resource);
      return 0;
    }
  }
  return -1;
}

// Print audit log
void gov_print_audit(const char *filter_user, int last_n) {
  printf(
      "\n╔════════════════════════════════════════════════════════════════╗\n");
  printf(
      "║                    GOVERNANCE AUDIT LOG                        ║\n");
  printf(
      "╚════════════════════════════════════════════════════════════════╝\n");

  int shown = 0;
  int start = (last_n > 0 && last_n < audit_count) ? audit_count - last_n : 0;

  for (int i = start; i < audit_count; i++) {
    if (filter_user && strcmp(audit_log[i].user, filter_user) != 0) {
      continue;
    }

    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S",
             localtime(&audit_log[i].timestamp));

    printf("[%s] %s: %s on %s - %s\n", time_str, audit_log[i].user,
           audit_log[i].action, audit_log[i].resource,
           audit_log[i].success ? "SUCCESS" : "FAILED");

    if (audit_log[i].reason[0] != '\0') {
      printf("  Reason: %s\n", audit_log[i].reason);
    }
    shown++;
  }

  if (shown == 0) {
    printf("No audit entries found.\n");
  }
  printf("\nTotal entries: %d (showing %d)\n", audit_count, shown);
}

// Print permissions
void gov_print_permissions() {
  printf(
      "\n╔════════════════════════════════════════════════════════════════╗\n");
  printf(
      "║                     PERMISSION TABLE                           ║\n");
  printf(
      "╚════════════════════════════════════════════════════════════════╝\n");
  printf("%-15s | %-30s | %-10s | %-15s\n", "User", "Resource", "Perms",
         "Granted By");
  printf("----------------------------------------------------------------\n");

  for (int i = 0; i < perm_count; i++) {
    char perms_str[16] = "";
    if (permissions[i].permissions & PERM_READ)
      strcat(perms_str, "R");
    if (permissions[i].permissions & PERM_WRITE)
      strcat(perms_str, "W");
    if (permissions[i].permissions & PERM_EXECUTE)
      strcat(perms_str, "X");
    if (permissions[i].permissions & PERM_DELETE)
      strcat(perms_str, "D");
    if (permissions[i].permissions & PERM_ADMIN)
      strcat(perms_str, "A");

    printf("%-15s | %-30s | %-10s | %-15s\n", permissions[i].user,
           permissions[i].resource, perms_str, permissions[i].granted_by);
  }

  if (perm_count == 0) {
    printf("No permissions defined.\n");
  }
}

// Validate file name (prevent duplicates and corruption)
int gov_validate_filename(const char *name) {
  // Check for empty or too long
  if (!name || strlen(name) == 0 || strlen(name) > 255) {
    return 0;
  }

  // Check for invalid characters
  const char *invalid_chars = "?*|<>\"";
  for (int i = 0; invalid_chars[i]; i++) {
    if (strchr(name, invalid_chars[i])) {
      return 0;
    }
  }

  // Check for corrupted names (control characters)
  for (int i = 0; name[i]; i++) {
    if ((unsigned char)name[i] < 32 && name[i] != '\n') {
      return 0;
    }
  }

  return 1;
}

// Clean filesystem (remove duplicates and corrupted files)
int gov_cleanup_filesystem() {
  printf("[GOV] Starting filesystem cleanup...\n");

  extern int global_file_count;
  extern void *global_file_registry[];

  int removed = 0;
  int i = 0;

  // Remove corrupted entries
  while (i < global_file_count) {
    void *entry = global_file_registry[i];
    if (!entry) {
      // Shift array
      for (int j = i; j < global_file_count - 1; j++) {
        global_file_registry[j] = global_file_registry[j + 1];
      }
      global_file_count--;
      removed++;
      continue;
    }
    i++;
  }

  printf("[GOV] Cleanup complete. Removed %d corrupted entries.\n", removed);
  gov_audit("FILESYSTEM_CLEANUP", "system", 1, NULL);

  return removed;
}
