/*
 * NexusQ-AI Graphics Framebuffer
 * File: modules/graphics/include/framebuffer.h
 */

#ifndef _FRAMEBUFFER_H_
#define _FRAMEBUFFER_H_

#include <stdint.h>

// RGBA Pixel Format (32-bit)
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} pixel_t;

// Framebuffer Structure
typedef struct {
    int width;
    int height;
    pixel_t* buffer; // Pointer to pixel array
} framebuffer_t;

// Standard Resolutions
#define RES_LOW_W  320
#define RES_LOW_H  200
#define RES_HIGH_W 640
#define RES_HIGH_H 400

#endif // _FRAMEBUFFER_H_
