#pragma once

#include "ray.h"

class camera {
    public:
    camera(int pixel_nx, int pixel_ny)  : pos(0, 0, 0), dir(0, 0, 0), focal_length(1.0f), 
    imageplane_width(0.5f), imageplane_height(0.5f), nx(pixel_nx), ny(pixel_ny) {
        l = -imageplane_width / 2.0f;
        r = imageplane_width / 2.0f;
        b = -imageplane_height / 2.0f;
        t = imageplane_height / 2.0f;
        normalizeDir();
    }

    camera(int pixel_nx, int pixel_ny, const vec3& position, const vec3& direction, float focalLen, float imgPlaneWidth, float imgPlaneHeight)
    : pos(position), dir(direction), focal_length(focalLen), imageplane_width(imgPlaneWidth), imageplane_height(imgPlaneHeight), nx(pixel_nx), ny(pixel_ny) {
        l = -imageplane_width / 2.0f;
        r = imageplane_width / 2.0f;
        b = -imageplane_height / 2.0f;
        t = imageplane_height / 2.0f;
        normalizeDir();
    }

    virtual ~camera() {}

    virtual ray generateRay(float i, float j, ray &R) = 0;

    vec3 getU() const { return U; }
    vec3 getV() const { return V; }
    vec3 getW() const { return W; }
    vec3 getPos() const { return pos; }

    void setPos(const vec3& newPos) { pos = newPos; }

    protected:
    vec3 pos, dir;
    vec3 U, V, W;
    vec3 up = vec3(0, 1, 0);
    float focal_length;
    int nx, ny;
    float imageplane_width;
    float imageplane_height;
    float l, r, b, t;

    void normalizeDir() {
        W = unit_vector(-dir);
        U = unit_vector(cross(up, W));
        V = cross(W, U);
    }
};

