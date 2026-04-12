#include "framebuffer.h"
#include "perspectiveCamera.h"
#include "sphere.h"
#include "triangle.h"
#include "lambertianShader.h"
#include "BPShader.h"
#include "normalShader.h"
#include "mirrorShader.h"
#include "light.h"
#include "hitRecord.h"
#include "hittableList.h"
#include "scene.h"
#include "handleGraphicsArgs.h"
#include <memory>
#include "SceneContainer.h"
#include "SceneLoader.h"
#include "SceneParser_JSON.h"

using namespace sivelab;

float tmin = 0.001f;
float tmax = std::numeric_limits<float>::infinity();

int main(int argc, char *argv[])
{
    /*
    SceneContainer parsedScene;

    std::shared_ptr<ISceneLoader> loader = std::make_shared<SceneLoader>(parsedScene);
    SceneParser_JSON parser(loader);

    std::string filename = argv[1];
    std::string outputFile = (argc >= 3) ? argv[2] : "output.png";
    parser.parseFileData(filename);


    // ── 2. Camera ────────────────────────────────────────────────────────
    auto cam = parsedScene.getCamera();
    if (!cam) {
        std::cerr << "ERROR: No camera in scene.\n";
        return 1;
    }

    int imgW = 200, imgH = 200;
    Framebuffer fb(imgW, imgH);

    perspectiveCamera pcam(
        imgW, imgH,
        vec3(cam->position[0], cam->position[1], cam->position[2]),
        vec3(cam->viewDir[0],  cam->viewDir[1],  cam->viewDir[2]),
        cam->focalLength,
        cam->imagePlaneWidth,
        cam->imagePlaneWidth
    );

    // ── 3. Shapes ────────────────────────────────────────────────────────
    hittableList world;

    auto makeMatFromShader = [&](const std::shared_ptr<Shader>& s)
        -> std::shared_ptr<shader>
    {
        if (!s) return std::make_shared<lambertian>(vec3(0.8f, 0.8f, 0.8f));

        if (s->type == "Lambertian" || s->type == "Diffuse" || s->type == "CoolToWarm") {
            return std::make_shared<lambertian>(
                vec3(s->diffuse[0], s->diffuse[1], s->diffuse[2]));
        }
        else if (s->type == "BlinnPhong" || s->type == "Phong" ||
                 s->type == "BlinnPhongMirrored") {
            return std::make_shared<blinnPhong>(
                vec3(s->diffuse[0], s->diffuse[1], s->diffuse[2]),
                0.1f,
                0.8f,
                0.5f,
                s->shininess,
                vec3(s->specular[0], s->specular[1], s->specular[2])
            );
        }
        else if (s->type == "Mirror") {
            return std::make_shared<mirrorShader>();
        }
        else {
            return std::make_shared<normalShader>();
        }
    };

    for (auto& shapePtr : parsedScene.getShapes()) {
        if (shapePtr->type == "sphere") {
            auto s   = std::static_pointer_cast<Sphere>(shapePtr);
            auto mat = makeMatFromShader(s->shader);
            world.add(std::make_shared<sphere>(
                vec3(s->center[0], s->center[1], s->center[2]),
                s->radius, mat));
        }
        else if (shapePtr->type == "triangle") {
            auto t   = std::static_pointer_cast<Triangle>(shapePtr);
            auto mat = makeMatFromShader(t->shader);
            world.add(std::make_shared<triangle>(
                vec3(t->v0[0], t->v0[1], t->v0[2]),
                vec3(t->v1[0], t->v1[1], t->v1[2]),
                vec3(t->v2[0], t->v2[1], t->v2[2]),
                mat));
        }
        else {
            std::cerr << "Skipping unsupported shape type: " << shapePtr->type << "\n";
        }
    }

    // ── 4. Lights ────────────────────────────────────────────────────────
    vec3 lightPos(2, 2, 2);
    vec3 lightColor(1, 1, 1);

    if (!parsedScene.getLights().empty()) {
        auto l = std::dynamic_pointer_cast<PointLight>(parsedScene.getLights()[0]);
        if (l) {
            lightPos   = vec3(l->position[0],  l->position[1],  l->position[2]);
            lightColor = vec3(l->intensity[0], l->intensity[1], l->intensity[2]);
        }
    }

    light sceneLight(lightPos, lightColor);

    // ── 5. Render ────────────────────────────────────────────────────────
    vec3 bgColor(0.5f, 0.5f, 0.5f);
    scene sc(&fb, &pcam, &world, &sceneLight, outputFile, bgColor);  // FIX: sc not scene
    sc.generateScene();

    std::cout << "Rendered to: " << outputFile << "\n";

    GraphicsArgs args;
    args.process(argc, argv);

    Framebuffer fb(args.width, args.height);
    perspectiveCamera cam(args.width, args.height, vec3(0,0,0), vec3(0,0,-1), 1.0f, 0.5f, 0.5f);
    hittableList world;
    auto lambertMat = std::make_shared<lambertian>(vec3(1.0f, 0.0f, 0.0f));
    auto bpMat = std::make_shared<blinnPhong>(vec3(0.0f, 0.0f, 1.0f));
    auto normalMat = std::make_shared<normalShader>();
    auto mirrorMat = std::make_shared<mirrorShader>();
    world.add(std::make_shared<sphere>(vec3(0, 1, -3), 0.5f, lambertMat));
    world.add(std::make_shared<sphere>(vec3(1, 0, -3), 0.5f, bpMat));
    world.add(std::make_shared<sphere>(vec3(-1, 0, -3), 0.5f, normalMat));  
    world.add(std::make_shared<sphere>(vec3(0, -100.5, -3), 100.0f, std::make_shared<lambertian>(vec3(0.1f, 0.5f, 0.1f))));
    world.add(std::make_shared<sphere>(vec3(1.5, 0, -3), 0.5f, std::make_shared<lambertian>(vec3(1.0f, 0.2f, 0.2f))));
    world.add(std::make_shared<sphere>(vec3(0, 0, -3), 0.5f, mirrorMat));
    light light(vec3(2, 2, 2), vec3(1.0f, 1.0f, 1.0f));
    scene sc(&fb, &cam, &world, &light, args.outputFileName, vec3(0.5f, 0.5f, 0.5f));
    sc.generateScene();

    Framebuffer fb1;
    fb1.clearToColor(vec3(0.5, 0.0, 1.0));
    fb1.exportPNG("output.png");

    Framebuffer fb2;
    fb2.clearToColor(vec3(1.0, 0.0, 0.0), vec3(0.0, 0.0, 1.0));
    fb2.exportPNG("output_gradient.png");

    Framebuffer fb3(200, 200);
    perspectiveCamera cam1(200, 200);
    for (int j = 0; j < 200; j++) {
        for (int i = 0; i < 200; i++) {
            ray R;
            cam1.generateRay(i, j, R);
            vec3 color = vec3((R.direction().x() + 1.0f) / 2.0f, (R.direction().y() + 1.0f) / 2.0f, (R.direction().z() + 1.0f) / 2.0f);
            fb3.setPixelColor(i, j, color);
        }
    }

    fb3.exportPNG("output_camera.png");

    Framebuffer fb4(800, 800);
    perspectiveCamera cam2(800, 800, vec3(0,0,0), vec3(0,0,-1), 1.0f, 2.0f, 2.0f);
    sphere sph(vec3(0,0,-2), 1.0f, nullptr);
    for (int j = 0; j < 800; j++) {
        for (int i = 0; i < 800; i++) {
            ray R;
            cam2.generateRay(i, j, R);
            vec3 color;
            float tmax = std::numeric_limits<float>::infinity();

            hitRecord rec1;
            if (sph.intersect(R, tmin, tmax, rec1)) {
                color = vec3((R.direction().x() + 1.0f) / 2.0f, (R.direction().y() + 1.0f) / 2.0f, (R.direction().z() + 1.0f) / 2.0f);
            } else {
                color = vec3(0.5 , 0.0 , 1.0f);
            }
            fb4.setPixelColor(i, j, color);
        }
    }
    fb4.exportPNG("output_sphere.png");

    Framebuffer fb5(200, 200);
    perspectiveCamera cam3(200, 200, vec3(0,0,0), vec3(0,0,-1), 0.7f, 0.5f, 0.5f);
    sphere sph1(vec3(0,-1.25, -7), 0.3f, nullptr);
    sphere sph2(vec3(0,0, -10), 3.0f, nullptr);
    sphere sph3(vec3(0, 0, -15), 5.0f, nullptr);
    for (int j = 0; j < 200; j++) {
        for (int i = 0; i < 200; i++) {
            ray R;
            cam3.generateRay(i, j, R);
            vec3 yellow = vec3(0.992, 0.863, 0.239);
            vec3 blue = vec3(0.149, 0.451, 0.698);
            vec3 background = vec3(0.5, 0.5, 0.5f);
            vec3 color = background;
            hitRecord rec2;
            float tmax = std::numeric_limits<float>::infinity();


            if (sph1.intersect(R, tmin, tmax, rec2)) {
                color = yellow;
            }
            if (sph2.intersect(R, tmin, tmax, rec2)) {
                color = blue;
            }
            if (sph3.intersect(R, tmin, tmax, rec2)) {
                color = yellow;
            }
            fb5.setPixelColor(i, j, color);
        }
    }
    fb5.exportPNG("output_multiple_spheres.png");

    Framebuffer fb6(200, 200);
    perspectiveCamera cam4(200, 200, vec3(0,0,0), vec3(0,0,-1), 1.0f, 0.5f, 0.5f);
    triangle tri1(vec3(-1.2, -0.2, -7), vec3(0.8, -0.5, -5), vec3(0.9, 0, -5), nullptr);
    triangle tri2(vec3(0.773205, -0.93923, -7), vec3(0.0330127, 0.94282, -5), vec3(-0.45, 0.779423, -5), nullptr);
    triangle tri3(vec3(0.426795, 1.13923, -7), vec3(-0.833013, -0.44282, -5), vec3(-0.45, -0.779423, -5), nullptr);
    for (int j = 0; j < 200; j++) {
        for (int i = 0; i < 200; i++) {
            ray R;
            cam4.generateRay(i, j, R);
            vec3 red(1.0f, 0.0f, 0.0f);
            vec3 green(0.0f, 1.0f, 0.0f);
            vec3 blue(0.0f, 0.0f, 1.0f);
            vec3 background(0.5f, 0.5f, 0.5f);
            float tmax = std::numeric_limits<float>::infinity();

            vec3 color = background;
            hitRecord rec3;
            if (tri1.intersect(R, tmin, tmax, rec3)) {
                color = red;
            }
            if (tri2.intersect(R, tmin, tmax, rec3)) {
                color = green;
            }
            if (tri3.intersect(R, tmin, tmax, rec3)) {
                color = blue;
            }
            fb6.setPixelColor(i, j, color);
        }
    }
    fb6.exportPNG("output_triangles.png");

    Framebuffer fb7(200, 200);
    perspectiveCamera cam5(200, 200, vec3(0,0.2,3.5), vec3(0,0,-1), 1.0f, 0.5f, 0.5f);
    sphere sph4(vec3(0, 0, -2), 1.0f, nullptr);
    //ears
    triangle tri4(vec3(-1, 0, -2), vec3(0, 1, -2), vec3(-0.9, 1.5, -2), nullptr);
    triangle tri5(vec3(1, 0, -2), vec3(0, 1, -2), vec3(0.9, 1.5, -2), nullptr);
    //eyes
    triangle tri6(vec3(-0.6, 0.05, -1), vec3(-0.3, 0.05, -1), vec3(-0.45, 0.3, -1), nullptr);
    triangle tri7(vec3(0.6, 0.05, -1), vec3(0.3, 0.05, -1), vec3(0.45, 0.3, -1), nullptr);
    //nose
    triangle tri8(vec3(0, 0.1, -1), vec3(-0.1, -0.1, -1), vec3(0.1, -0.1, -1), nullptr);
    //mouth
    triangle tri9(vec3(-0.25, -0.25, -1), vec3(0.25, -0.25, -1), vec3(0, -0.35, -1), nullptr);
    for (int j = 0; j < 200; j++) {
        for (int i = 0; i < 200; i++) {
            ray R;
            cam5.generateRay(i, j, R);
            vec3 white = vec3(1.0, 1.0, 1.0);
            vec3 black = vec3(0, 0, 0);
            vec3 background = vec3(0.5, 0.2, 0.5f);
            vec3 color = background;
            hitRecord rec4;
            float tmax = std::numeric_limits<float>::infinity();

            if (sph4.intersect(R, tmin, tmax, rec4) || tri4.intersect(R, tmin, tmax, rec4) || tri5.intersect(R, tmin, tmax, rec4)) {
                color = white;
            }
            if (tri6.intersect(R, tmin, tmax, rec4) || tri7.intersect(R, tmin, tmax, rec4) || tri8.intersect(R, tmin, tmax, rec4) || tri9.intersect(R, tmin, tmax, rec4)) {
                color = black;
            }
            fb7.setPixelColor(i, j, color);
        }
    }
    fb7.exportPNG("cat.png"); 

    
    Framebuffer fb8(200, 200);
    perspectiveCamera cam6(200, 200, vec3(0, 0, 0), vec3(0,0,-1), 1.0f, 0.5f, 0.5f);
    hittableList world;
    auto mat1 = std::make_shared<lambertian>(vec3(0.7f, 0.3f, 0.3f));
    world.add(std::make_shared<sphere>(vec3(0, 0, -3), 0.5f, mat1));
    light light1(vec3(2, 2, 2), vec3(1.0f, 1.0f, 1.0f));
    scene sc(&fb8, &cam6, &world, &light1, "output_lambertian.png", vec3(0.5f, 0.5f, 0.5f));
    sc.generateScene();

    Framebuffer fb9(200, 200);
    perspectiveCamera cam7(200, 200, vec3(0, 0.5, 3.25), vec3(0,0,-1), 1.0f, 0.5f, 0.5f);
    hittableList world2;
    auto lambertMat = std::make_shared<lambertian>(vec3(1.0f, 0.0f, 0.0f));
    auto bpMat = std::make_shared<blinnPhong>(vec3(0.0f, 0.0f, 1.0f));
    auto normalMat = std::make_shared<normalShader>();
    world2.add(std::make_shared<sphere>(vec3(0, 1, -3), 0.5f, lambertMat));
    world2.add(std::make_shared<sphere>(vec3(1, 0, -3), 0.5f, bpMat));
    world2.add(std::make_shared<sphere>(vec3(-1, 0, -3), 0.5f, normalMat));
    light light2(vec3(1, 2, -2.5), vec3(1.0f, 1.0f, 1.0f));
    scene sc2(&fb9, &cam7, &world2, &light2, "output_shaders.png", vec3(0.5f, 0.5f, 0.5f));
    sc2.generateScene();

    Framebuffer fb10(200, 200);
    perspectiveCamera cam8(200, 200, vec3(0, 0.0, 0.0), vec3(0,0,-1), 1.0f, 0.5f, 0.5f);
    hittableList world3;
    auto bpMat2 = std::make_shared<blinnPhong>(vec3(0.3f, 0.3f, 1.0f));
    world3.add(std::make_shared<sphere>(vec3(0, 0, -3), 0.5f, bpMat2));
    light light3(vec3(2, 2, 2), vec3(1.0f, 1.0f, 1.0f));
    scene sc3(&fb10, &cam8, &world3, &light3, "output_blinnPhong.png", vec3(0.5f, 0.5f, 0.5f));
    sc3.generateScene();

    Framebuffer fb11(200, 200);
    perspectiveCamera cam9(200, 200, vec3(0, 0.0, 0.0), vec3(0,0,-1), 1.0f, 0.5f, 0.5f);
    hittableList world4;
    auto mirrorMat = std::make_shared<mirrorShader>();
    world4.add(std::make_shared<sphere>(vec3(0, 0, -3), 0.5f, mirrorMat));
    world4.add(std::make_shared<sphere>(vec3(0, -100.5, -3), 100.0f, std::make_shared<lambertian>(vec3(0.1f, 0.5f, 0.1f))));
    world4.add(std::make_shared<sphere>(vec3(1.5, 0, -3), 0.5f, std::make_shared<lambertian>(vec3(1.0f, 0.2f, 0.2f))));
    light light4(vec3(2, 2, 2), vec3(1.0f, 1.0f, 1.0f));
    scene sc4(&fb11, &cam9, &world4, &light4, "output_mirror.png", vec3(0.5f, 0.5f, 0.5f));
    sc4.generateScene();
    return 0;
    */

    Framebuffer fb12(1000, 1000);
    perspectiveCamera cam10(1000, 1000, vec3(0, 0, 12), vec3(0, 0, -1), 1.0, 0.5f, 0.5f);
    hittableList world5;
    auto bpMat3 = std::make_shared<blinnPhong>(vec3(1.0f, 0.0f, 0.0f));
    auto bpMat4 = std::make_shared<blinnPhong>(vec3(0.0f, 0.0f, 1.0f));
    world5.add(std::make_shared<sphere>(vec3(0, 0, 0), 1.0f, bpMat4));
    world5.add(std::make_shared<sphere>(vec3(0, -11, 0), 10.0f, bpMat3));
    light light5(vec3(0, 5, 10), vec3(1.0f, 1.0f, 1.0f));
    scene sc5(&fb12, &cam10, &world5, &light5, "fbMainComparison.png", vec3(0.5, 0.5, 0.5));
    sc5.generateScene();
    return 0;
    
}