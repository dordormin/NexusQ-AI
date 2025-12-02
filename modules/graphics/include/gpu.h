/*
 * NexusQ-AI Simulated GPU Interface
 * File: modules/graphics/include/gpu.h
 */

#ifndef _GPU_H_
#define _GPU_H_

#define GPU_WIDTH 60
#define GPU_HEIGHT 20

typedef struct {
    char buffer[GPU_HEIGHT][GPU_WIDTH];
} gpu_buffer_t;

// Initialize the GPU (Clear screen)
void gpu_init(void);

// Clear the buffer
void gpu_clear(void);

// Draw a point in 3D space (Projected to 2D)
// x, y, z range: -1.0 to 1.0
void gpu_draw_point_3d(float x, float y, float z, char c);

// Draw a line in 3D space
void gpu_draw_line_3d(float x1, float y1, float z1, float x2, float y2, float z2, char c);

// Flush buffer to terminal (Render)
void gpu_flush(void);

#endif // _GPU_H_
