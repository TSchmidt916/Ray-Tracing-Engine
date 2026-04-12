#pragma once

#include "camera.h"
#include "ray.h"

class perspectiveCamera : public camera
{
    public:
    perspectiveCamera(int pixel_nx, int pixel_ny) : camera(pixel_nx, pixel_ny) {}

    perspectiveCamera(int pixel_nx, int pixel_ny, const vec3& position, const vec3& direction, float focalLen, float imgPlaneWidth, float imgPlaneHeight)
    : camera(pixel_nx, pixel_ny, position, direction, focalLen, imgPlaneWidth, imgPlaneHeight) {}

    ray generateRay(float i, float j, ray &R) override {
        float u, v;

        u = l + (r-l) * (i+0.5)/(float)nx;
        v = b + (t-b) * (j+0.5)/(float)ny;
        vec3 dir = u*U + v*V + focal_length*(-W);
        R = ray(pos, unit_vector(dir));
        return R;
    }

    
};