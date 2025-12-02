/*
 * NexusQ-AI Kernel - Kernel Logging System
 * File: kernel/core/klog.c
 */

#include "include/klog.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define KLOG_BUFFER_SIZE 4096
#define KLOG_ENTRY_SIZE 256

typedef struct {
  int level;
  char message[KLOG_ENTRY_SIZE];
  time_t timestamp;
} klog_entry_t;

static klog_entry_t klog_buffer[KLOG_BUFFER_SIZE];
static int klog_head = 0;
static int klog_tail = 0;
static int klog_count = 0;

static const char *level_strings[] = {"[DEBUG]", "[INFO] ", "[WARN] ",
                                      "[ERROR]"};

void klog_init(void) {
  klog_head = 0;
  klog_tail = 0;
  klog_count = 0;
  memset(klog_buffer, 0, sizeof(klog_buffer));
}

void klog_write(int level, const char *fmt, ...) {
  if (level < KLOG_DEBUG || level > KLOG_ERROR) {
    level = KLOG_INFO;
  }

  klog_entry_t *entry = &klog_buffer[klog_head];
  entry->level = level;
  entry->timestamp = time(NULL);

  // Format message
  va_list args;
  va_start(args, fmt);
  vsnprintf(entry->message, KLOG_ENTRY_SIZE - 1, fmt, args);
  va_end(args);

  // Advance head
  klog_head = (klog_head + 1) % KLOG_BUFFER_SIZE;

  if (klog_count < KLOG_BUFFER_SIZE) {
    klog_count++;
  } else {
    // Buffer full, overwrite oldest
    klog_tail = (klog_tail + 1) % KLOG_BUFFER_SIZE;
  }

  // Also print to stdout for immediate visibility
  printf("%s %s\n", level_strings[level], entry->message);
}

int klog_read(char *buffer, int max_size) {
  if (!buffer || max_size <= 0) {
    return -1;
  }

  int written = 0;
  int idx = klog_tail;

  for (int i = 0; i < klog_count && written < max_size - 1; i++) {
    klog_entry_t *entry = &klog_buffer[idx];

    // Format: [LEVEL] timestamp: message
    int len =
        snprintf(buffer + written, max_size - written, "%s [%ld] %s\n",
                 level_strings[entry->level], entry->timestamp, entry->message);

    if (len < 0 || written + len >= max_size) {
      break;
    }

    written += len;
    idx = (idx + 1) % KLOG_BUFFER_SIZE;
  }

  buffer[written] = '\0';
  return written;
}
