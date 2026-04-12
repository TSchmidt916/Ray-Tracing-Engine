#pragma once 
#include "shader.h"
#include "hitRecord.h"
#include "light.h"

class blinnPhong : public shader {
public:
    blinnPhong(const vec3& color, float ka = 1.0f, float kd = 1.0f, float ks = 1.0f, float shininess = 32.0f, const vec3& specColor = vec3(1.0f, 1.0f, 1.0f)) 
        : shader(color), ka(ka), kd(kd), ks(ks), shininess(shininess), specular_color(specColor) {}
    
    vec3 ray_color(const hitRecord& rec, const light& pointLight, const hittableList& world, const vec3& backgroundColor, int depth) const override;

    bool shadow_ray(const hitRecord& rec, const light& pointLight, const hittableList& world) const override;

private:
    float ka;        
    float kd;        
    float ks;       
    float shininess;
    vec3 specular_color;
};