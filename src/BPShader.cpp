#include "BPShader.h"
#include "hitRecord.h"
#include "light.h"
#include <algorithm>

vec3 blinnPhong::ray_color(const hitRecord& rec, const light& pointLight, const hittableList& world, const vec3& backgroundColor, int depth) const {
    vec3 ambient = ka * base_color * vec3(0.2f, 0.2f, 0.2f);
    
    if (shadow_ray(rec, pointLight, world)) {
        return clamp(ambient, 0.0f, 1.0f);
    }

    vec3 light_dir = unit_vector(pointLight.position - rec.p);
    
    float diff = std::max(0.0f, dot(rec.normal, light_dir));
    vec3 diffuse = kd * diff * base_color * pointLight.color;
    
    vec3 view_dir = unit_vector(rec.incoming_ray.origin() - rec.p);

    vec3 halfway = unit_vector(light_dir + view_dir);
    
    float spec = std::pow(std::max(0.0f, dot(rec.normal, halfway)), shininess);
    vec3 specular = ks * spec * specular_color * pointLight.color;
    
    return clamp(ambient + diffuse + specular, 0.0f, 1.0f);
}

bool blinnPhong::shadow_ray(const hitRecord& rec, const light& pointLight, const hittableList& world) const {
    vec3 light_dir = unit_vector(pointLight.position - rec.p);
    ray shadowRay(rec.p + rec.normal * 0.001f, light_dir);
    float tmin = 0.001f;
    float tmax = (pointLight.position - rec.p).length();
    hitRecord shadowRec;
    if (world.intersect(shadowRay, tmin, tmax, shadowRec)) {
        return true;
    }
    return false;
}