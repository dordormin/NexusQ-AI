/*
 * NexusQ-AI Simulated GPU
 * File: modules/graphics/gpu.c
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "include/gpu.h"

static gpu_buffer_t frame_buffer;

void gpu_init(void) {
    gpu_clear();
}

void gpu_clear(void) {
    for (int y = 0; y < GPU_HEIGHT; y++) {
        for (int x = 0; x < GPU_WIDTH; x++) {
            frame_buffer.buffer[y][x] = ' ';
        }
    }
}

// Simple Weak Perspective Projection
void project(float x, float y, float z, int* out_x, int* out_y) {
    // Camera is at z = -2.0
    float dist = 2.0f;
    float scale_x = 20.0f;
    float scale_y = 10.0f;
    
    float z_factor = 1.0f / (dist + z);
    
    *out_x = (int)(x * z_factor * scale_x) + (GPU_WIDTH / 2);
    *out_y = (int)(y * z_factor * scale_y) + (GPU_HEIGHT / 2);
}

void gpu_draw_point_3d(float x, float y, float z, char c) {
    int px, py;
    project(x, y, z, &px, &py);
    
    if (px >= 0 && px < GPU_WIDTH && py >= 0 && py < GPU_HEIGHT) {
        frame_buffer.buffer[py][px] = c;
    }
}

void gpu_draw_line_3d(float x1, float y1, float z1, float x2, float y2, float z2, char c) {
    // Bresenham-like interpolation in 3D (simplified)
    float dist = sqrtf((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) + (z2-z1)*(z2-z1));
    int steps = (int)(dist * 20.0f); // Resolution
    if (steps < 1) steps = 1;
    
    for (int i = 0; i <= steps; i++) {
        float t = (float)i / steps;
        float x = x1 + (x2 - x1) * t;
        float y = y1 + (y2 - y1) * t;
        float z = z1 + (z2 - z1) * t;
        gpu_draw_point_3d(x, y, z, c);
    }
}

void gpu_flush(void) {
    printf("\n+");
    for (int i=0; i<GPU_WIDTH; i++) printf("-");
    printf("+\n");
    
    for (int y = 0; y < GPU_HEIGHT; y++) {
        printf("|");
        for (int x = 0; x < GPU_WIDTH; x++) {
            putchar(frame_buffer.buffer[y][x]);
        }
        printf("|\n");
    }
    
    printf("+");
    for (int i=0; i<GPU_WIDTH; i++) printf("-");
    printf("+\n");
}
