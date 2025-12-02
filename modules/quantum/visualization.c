/*
 * NexusQ-AI Quantum Visualization
 * File: modules/quantum/visualization.c
 */

#include <math.h>
#include "../../modules/graphics/include/gpu.h"

#define PI 3.14159265f

// Render a Bloch Sphere representing the state |psi> = cos(theta/2)|0> + e^(i*phi)sin(theta/2)|1>
// For simplicity, we take alpha/beta amplitudes directly or just angles.
// Let's use angles: theta (0 to PI), phi (0 to 2PI)
void qvis_bloch_sphere(float theta, float phi) {
    gpu_clear();
    
    // Draw Sphere Wireframe (Equator and Meridians)
    // Equator
    for (float a = 0; a < 2*PI; a += 0.1f) {
        gpu_draw_point_3d(cosf(a), sinf(a), 0.0f, '.');
    }
    // Meridian 1
    for (float a = 0; a < 2*PI; a += 0.1f) {
        gpu_draw_point_3d(cosf(a), 0.0f, sinf(a), '.');
    }
    // Meridian 2
    for (float a = 0; a < 2*PI; a += 0.1f) {
        gpu_draw_point_3d(0.0f, cosf(a), sinf(a), '.');
    }
    
    // Draw Axes
    gpu_draw_line_3d(-1.2f, 0, 0, 1.2f, 0, 0, '-'); // X
    gpu_draw_line_3d(0, -1.2f, 0, 0, 1.2f, 0, '|'); // Y
    gpu_draw_line_3d(0, 0, -1.2f, 0, 0, 1.2f, '/'); // Z
    
    // Calculate State Vector
    // x = sin(theta) * cos(phi)
    // y = sin(theta) * sin(phi)
    // z = cos(theta)
    float x = sinf(theta) * cosf(phi);
    float y = sinf(theta) * sinf(phi);
    float z = cosf(theta);
    
    // Draw Vector
    gpu_draw_line_3d(0, 0, 0, x, y, z, '*');
    gpu_draw_point_3d(x, y, z, 'Q'); // Qubit Head
    
    gpu_flush();
}
