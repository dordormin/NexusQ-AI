/*
 * NexusQ-AI Quantum Text Editor (QEdit)
 * File: apps/qedit.c
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINES 100
#define MAX_LEN 80

char buffer[MAX_LINES][MAX_LEN];
int line_count = 0;
char current_file[64];

// Load file into buffer
void qedit_load(const char *filename) {
  strncpy(current_file, filename, 63);
  line_count = 0;

  // In a real OS, we would use fopen/fgets.
  // Here we use our syscall wrapper `nexus_read_file` but it reads the whole
  // blob. For simplicity in this simulation, we assume the file is empty or we
  // start fresh. To support editing existing files, we would need to parse the
  // blob.

  // Attempt to read existing content
  char content[4096];
  extern int nexus_read_file(const char *name, char *buf, int size);
  if (nexus_read_file(filename, content, 4095) > 0) {
    // Parse lines
    char *line = strtok(content, "\n");
    while (line != NULL && line_count < MAX_LINES) {
      strncpy(buffer[line_count++], line, MAX_LEN - 1);
      line = strtok(NULL, "\n");
    }
    printf("[QEdit] Loaded '%s' (%d lines).\n", filename, line_count);
  } else {
    printf("[QEdit] New file: '%s'\n", filename);
  }
}

// Save buffer to file
void qedit_save() {
  char content[4096] = "";
  for (int i = 0; i < line_count; i++) {
    strcat(content, buffer[i]);
    strcat(content, "\n");
  }

  // Use nexus_create_file which calls the syscall with proper parent_id
  extern int nexus_create_file(const char *name, const char *content,
                               int parent_id);
  extern int cwd_id; // Current working directory from shell.c

  if (nexus_create_file(current_file, content, cwd_id) == 0) {
    printf("[QEdit] Saved '%s' (%d bytes).\n", current_file,
           (int)strlen(content));
  } else {
    printf("[QEdit] Error: Failed to save file.\n");
  }
}

// Main Editor Loop
void qedit_main(const char *filename) {
  qedit_load(filename);

  // Print welcome message with clear instructions
  printf("\n=== QEdit - Quantum Text Editor ===\n");
  printf("Commands (type at ':' prompt):\n");
  printf("  h, help       - Show this help\n");
  printf("  p, list       - Print all lines\n");
  printf("  a, add        - Add lines (end with '.')\n");
  printf("  d <num>       - Delete line number\n");
  printf("  w, save       - Save file\n");
  printf("  q, quit       - Quit without saving\n");
  printf("  wq, x         - Save and quit\n");
  printf("  q!            - Force quit without saving\n");
  printf("===================================\n\n");

  char cmd[64];
  while (1) {
    printf(":"); // Prompt
    if (fgets(cmd, sizeof(cmd), stdin) == NULL)
      break;

    // Remove newline
    cmd[strcspn(cmd, "\n")] = 0;

    // Ignore empty commands
    if (strlen(cmd) == 0)
      continue;

    // Help command
    if (strcmp(cmd, "h") == 0 || strcmp(cmd, "help") == 0 ||
        strcmp(cmd, "?") == 0) {
      printf("\n--- QEdit Help ---\n");
      printf("  p, list       - Print all lines with numbers\n");
      printf("  a, add        - Enter insert mode (type text, end with '.' "
             "alone)\n");
      printf("  d <num>       - Delete line <num> (e.g., 'd 5')\n");
      printf("  w, save       - Save file to disk\n");
      printf("  q, quit       - Quit editor (warns if unsaved changes)\n");
      printf("  wq, x         - Save and quit in one command\n");
      printf("  q!            - Force quit without saving\n");
      printf("------------------\n\n");
    }
    // Quit commands
    else if (strcmp(cmd, "q") == 0 || strcmp(cmd, "quit") == 0) {
      printf("Quitting... (use 'wq' to save first, or 'q!' to force quit)\n");
      break;
    } else if (strcmp(cmd, "q!") == 0) {
      printf("Force quit without saving.\n");
      break;
    }
    // Save commands
    else if (strcmp(cmd, "w") == 0 || strcmp(cmd, "save") == 0) {
      qedit_save();
    }
    // Save and quit
    else if (strcmp(cmd, "wq") == 0 || strcmp(cmd, "x") == 0) {
      qedit_save();
      printf("Saved and exiting.\n");
      break;
    }
    // Print/list
    else if (strcmp(cmd, "p") == 0 || strcmp(cmd, "list") == 0) {
      if (line_count == 0) {
        printf("(Empty file)\n");
      } else {
        for (int i = 0; i < line_count; i++) {
          printf("%3d: %s\n", i + 1, buffer[i]);
        }
      }
    }
    // Add/insert
    else if (strcmp(cmd, "a") == 0 || strcmp(cmd, "add") == 0) {
      printf(
          "[Insert Mode] Type your text. Enter '.' on a new line to finish.\n");
      while (line_count < MAX_LINES) {
        char line[MAX_LEN];
        printf("  > ");
        if (fgets(line, sizeof(line), stdin) == NULL)
          break;
        line[strcspn(line, "\n")] = 0;
        if (strcmp(line, ".") == 0)
          break;
        strncpy(buffer[line_count++], line, MAX_LEN - 1);
      }
      printf("[Insert Mode Ended] %d lines in buffer.\n", line_count);
    }
    // Delete line
    else if (strncmp(cmd, "d ", 2) == 0) {
      int ln;
      if (sscanf(cmd + 2, "%d", &ln) == 1 && ln > 0 && ln <= line_count) {
        // Shift lines
        for (int i = ln - 1; i < line_count - 1; i++) {
          strcpy(buffer[i], buffer[i + 1]);
        }
        line_count--;
        printf("Deleted line %d.\n", ln);
      } else {
        printf("Invalid line number. Use 'd <number>' (e.g., 'd 3')\n");
      }
    }
    // Unknown command
    else {
      printf("Unknown command '%s'. Type 'h' for help.\n", cmd);
    }
  }
}
