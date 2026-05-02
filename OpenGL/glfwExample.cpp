#include <cstdlib>
#include <iostream>
#include <vector>
#include <png++/png.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "../src/perspectiveCamera.h"
#include "../src/vec3.h"

#include "GLSL.h"
#include "mouseControls.cpp"
#include "../src/model_obj.h"

//Ocean stuff
struct GridVertex {
    float position[3];
    float normal[3];
};

std::vector<GridVertex> gridVerts;
std::vector<unsigned int> gridIndices;

void generateGrid(int width, int depth, float spacing) {
    gridVerts.clear();
    gridIndices.clear();

    for (int z = 0; z <= depth; z++) {
        for (int x = 0; x <= width; x++) {
            GridVertex v;
            v.position[0] = (x - width/2.0f) * spacing;
            v.position[1] = 0.0f;
            v.position[2] = (z - depth/2.0f) * spacing;
            v.normal[0] = 0.0f;
            v.normal[1] = 1.0f;
            v.normal[2] = 0.0f;
            gridVerts.push_back(v);
        }
    }

    for (int z = 0; z < depth; z++) {
        for (int x = 0; x < width; x++) {
            int wave_top_left = z * (width + 1) + x;
            int wave_top_right = wave_top_left + 1;
            int wave_bottom_left = wave_top_left + (width + 1);
            int wave_bottom_right = wave_bottom_left + 1;
            
            gridIndices.push_back(wave_top_left);
            gridIndices.push_back(wave_bottom_left);
            gridIndices.push_back(wave_top_right);

            gridIndices.push_back(wave_top_right);
            gridIndices.push_back(wave_bottom_left);
            gridIndices.push_back(wave_bottom_right);
        }
    }
}

//sphere stuff that should be moved
struct SphereVertex {
    float position[3];
    float normal[3];
};

std::vector<SphereVertex> sphereVerts;
std::vector<unsigned int> sphereIndices;

void generateSphere(float radius, int stacks, int slices) {
    sphereVerts.clear();
    sphereIndices.clear();

    for (int i = 0; i <= stacks; i++) {
        float phi = M_PI * i / stacks;
        for (int j = 0; j <= slices; j++) {
            float theta = 2.0f * M_PI * j / slices;

            SphereVertex v;
            v.position[0] = radius * sin(phi) * cos(theta);
            v.position[1] = radius * cos(phi);
            v.position[2] = radius * sin(phi) * sin(theta);
            v.normal[0] = sin(phi) * cos(theta);
            v.normal[1] = cos(phi);
            v.normal[2] = sin(phi) * sin(theta);
            sphereVerts.push_back(v);
        }
    }

    for (int i = 0; i < stacks; i++) {
        for (int j = 0; j < slices; j++) {
            int top_left = i * (slices + 1) + j;
            int top_right = top_left + 1;
            int bottom_left = top_left + (slices + 1);
            int bottom_right = bottom_left + 1;

            sphereIndices.push_back(top_left);
            sphereIndices.push_back(bottom_left);
            sphereIndices.push_back(top_right);

            sphereIndices.push_back(top_right);
            sphereIndices.push_back(bottom_left);
            sphereIndices.push_back(bottom_right);
        }
    }
}

void setUniforms(
    sivelab::GLSLObject& shader,
    const glm::mat4& proj,
    const glm::mat4& view,
    const glm::mat4& model,
    const glm::mat4& normal)
{
    glUniformMatrix4fv(shader.createUniform("projMatrix"), 1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(shader.createUniform("viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(shader.createUniform("modelMatrix"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(shader.createUniform("normalMatrix"), 1, GL_FALSE, glm::value_ptr(normal));
}

int CheckGLErrors(const char *s)
{
    int errCount = 0;
    return errCount;
}

int main(void)
{
    /* Initialize the library */
    if (!glfwInit()) {
        exit (-1);
    }
    // throw std::runtime_error("Error! initialization of glfw failed!");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    /* Create a windowed mode window and its OpenGL context */
    int winWidth = 1000;
    float aspectRatio = 1.0; // 16.0 / 9.0; // winWidth / (float)winHeight;
    int winHeight = winWidth / aspectRatio;

    GLFWwindow* window = glfwCreateWindow(winWidth, winHeight, "GLFW Example", NULL, NULL);
    if (!window) {
        std::cerr << "GLFW did not create a window!" << std::endl;
        
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    GLenum err=glewInit();
    if(err != GLEW_OK) {
        std::cerr <<"GLEW Error! glewInit failed, exiting."<< std::endl;
        exit(EXIT_FAILURE);
    }

    const GLubyte* renderer = glGetString (GL_RENDERER);
    const GLubyte* version = glGetString (GL_VERSION);
    std::cout << "Renderer: " << renderer << std::endl;
    std::cout << "OpenGL version supported: " << version << std::endl;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(0.99f, 0.57f, 0.28f, 1.0);

    int fb_width, fb_height;
    glfwGetFramebufferSize(window, &fb_width, &fb_height);
    glViewport(0, 0, fb_width, fb_height);

    glm::mat4 M_perspective = glm::perspective(glm::radians(45.0f), (float)winWidth / (float)winHeight, 0.1f, 100.0f);

    // The ortho parameters, in order: left, right, bottom, top, zNear, zFar
    float halfWidth = 15.0 / 2.0;
    float halfHeight = halfWidth;

    float left = -halfWidth;
    float right = halfWidth;

    float bottom = -halfHeight;
    float top = halfHeight;

    float near = 5.0f;
    float far = -5.0f;

    glm::mat4 M_ortho = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, near, far);
    glm::mat4 projectionMatrix = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, -10.0f, 10.0f);

    //Textures
    std::string texFilename = "../Textures/textureAtlas.png";
    png::image<png::rgb_pixel> texPNGImage;
    texPNGImage.read(texFilename);

    int pngWidth = texPNGImage.get_width();
    int pngHeight = texPNGImage.get_height();

    std::vector<float> texData(pngHeight * pngWidth * 3);

    size_t idx = 0;
    for (size_t row = 0; row < pngHeight; ++row) {
        for (size_t col = 0; col < pngWidth; ++col) {
            png::rgb_pixel pixel = texPNGImage[pngHeight - row - 1][col];
            texData[idx++] = pixel.red / 255.0f;
            texData[idx++] = pixel.green /255.0f;
            texData[idx++] = pixel.blue / 255.0f;
        }
    }

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,  pngWidth, pngHeight, 0, GL_RGB, GL_FLOAT, texData.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    //obj model
    ModelOBJ model;
    if (!model.import("../OBJs/ship.obj")) {
        std::cerr << "Failed to load OBJ file!" << std::endl;
        glfwTerminate();
        return -1;
    }

    model.normalize();

    GLuint objVBO, objIBO, objVAO;
    glGenVertexArrays(1, &objVAO);
    glBindVertexArray(objVAO);

    glGenBuffers(1, &objVBO);
    glBindBuffer(GL_ARRAY_BUFFER, objVBO);
    glBufferData(GL_ARRAY_BUFFER, model.getNumberOfVertices() * model.getVertexSize(), model.getVertexBuffer(), GL_STATIC_DRAW);

    glGenBuffers(1, &objIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.getNumberOfIndices() * sizeof(int), model.getIndexBuffer(), GL_STATIC_DRAW);

    int objStride = model.getVertexSize();

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, objStride, (void*)offsetof(ModelOBJ::Vertex, position));
    //normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, objStride, (void*)offsetof(ModelOBJ::Vertex, normal));
    //texcoords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, objStride, (void*)offsetof(ModelOBJ::Vertex, texCoord));

    glBindVertexArray(0);

    //sphere generation
    generateSphere(1.0f, 32, 32);

    GLuint sphereVBO, sphereIBO, sphereVAO;
    glGenVertexArrays(1, &sphereVAO);
    glBindVertexArray(sphereVAO);

    glGenBuffers(1, &sphereVBO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVerts.size() * sizeof(SphereVertex), sphereVerts.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &sphereIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), sphereIndices.data(), GL_STATIC_DRAW);
    
    int sphereStride = sizeof(SphereVertex);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sphereStride, (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sphereStride, (void*)12);

    glBindVertexArray(0);

    //ocean generation
    generateGrid(200, 200, 0.1f);

    GLuint gridVBO, gridIBO, gridVAO;
    glGenVertexArrays(1, &gridVAO);
    glBindVertexArray(gridVAO);

    glGenBuffers(1, &gridVBO);
    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    glBufferData(GL_ARRAY_BUFFER, gridVerts.size() * sizeof(GridVertex), gridVerts.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &gridIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gridIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, gridIndices.size() * sizeof(unsigned int), gridIndices.data(), GL_STATIC_DRAW);

    int gridStride = sizeof(GridVertex);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, gridStride, (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, gridStride, (void*)12);

    glBindVertexArray(0);



    //Triangles
    std::vector<float> triangle_VertexBuffer{
        //square 1
        -0.5f, -0.5f, 0.0f,     0.0f, 0.0f, 1.0f,   0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,      0.0f, 0.0f, 1.0f,   1.0f, 0.0f,
        0.5f, 0.5f, 0.0f,       0.0f, 0.0f, 1.0f,   1.0f, 0.5f,

        -0.5f, -0.5f, 0.0f,     0.0f, 0.0f, 1.0f,   0.5f, 0.0f,
        0.5f, 0.5f, 0.0f,       0.0f, 0.0f, 1.0f,   1.0f, 0.5f,
        -0.5f, 0.5f, 0.0f,      0.0f, 0.0f, 1.0f,   0.5f, 0.5f,

        //square 2
        -0.5f, -0.5f, -1.0f,    0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
        -0.5f, -0.5f, 0.0f,     0.0f, 0.0f, 1.0f,   0.5f, 0.0f,
        -0.5f, 0.5f, 0.0f,      0.0f, 0.0f, 1.0f,   0.5f, 0.5f,

        -0.5f, -0.5f, -1.0f,    0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
        -0.5f, 0.5f, 0.0f,      0.0f, 0.0f, 1.0f,   0.5f, 0.5f,
        -0.5f, 0.5f, -1.0f,      0.0f, 0.0f, 1.0f,   0.0f, 0.5f,
    };

    int numBytes = triangle_VertexBuffer.size() * sizeof(float);

    GLuint triangleVBO, triangleVAO;
    glGenVertexArrays(1, &triangleVAO);
    glBindVertexArray(triangleVAO);

    glGenBuffers(1, &triangleVBO);
    glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
    glBufferData(GL_ARRAY_BUFFER, numBytes, triangle_VertexBuffer.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)12);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)24);

    glBindVertexArray(0);

    //Shaders
    sivelab::GLSLObject shaderLambertian, shaderBlinnPhong, shaderOcean;

    shaderLambertian.addShader("vertexShader_PrepForPerFragment.glsl", sivelab::GLSLObject::VERTEX_SHADER);
    shaderLambertian.addShader("fragmentShader_Lambertian.glsl", sivelab::GLSLObject::FRAGMENT_SHADER);
    shaderLambertian.createProgram();

    shaderBlinnPhong.addShader("vertexShader_blinnPhongTex.glsl", sivelab::GLSLObject::VERTEX_SHADER);
    shaderBlinnPhong.addShader("fragmentShader_blinnPhongTex.glsl", sivelab::GLSLObject::FRAGMENT_SHADER);
    shaderBlinnPhong.createProgram();

    shaderOcean.addShader("vertexShader_ocean.glsl", sivelab::GLSLObject::VERTEX_SHADER);
    shaderOcean.addShader("fragmentShader_ocean.glsl", sivelab::GLSLObject::FRAGMENT_SHADER);
    shaderOcean.createProgram();

    GLuint lamb_proj = shaderLambertian.createUniform("projMatrix");
    GLuint lamb_view = shaderLambertian.createUniform("viewMatrix");
    GLuint lamb_model = shaderLambertian.createUniform("modelMatrix");
    GLuint lamb_normal = shaderLambertian.createUniform("normalMatrix");
    GLuint lamb_lpos = shaderLambertian.createUniform("lightPosWorld");
    GLuint lamb_Ia = shaderLambertian.createUniform("Ia");
    GLuint lamb_kd = shaderLambertian.createUniform("kd");
    GLuint lamb_lint = shaderLambertian.createUniform("lightIntensity");

    GLuint bp_proj = shaderBlinnPhong.createUniform("projMatrix");
    GLuint bp_view = shaderBlinnPhong.createUniform("viewMatrix");
    GLuint bp_model = shaderBlinnPhong.createUniform("modelMatrix");
    GLuint bp_normal = shaderBlinnPhong.createUniform("normalMatrix");
    GLuint bp_Ia = shaderBlinnPhong.createUniform("Ia");
    GLuint bp_ka = shaderBlinnPhong.createUniform("ka");
    GLuint bp_kd = shaderBlinnPhong.createUniform("kd");
    GLuint bp_ks = shaderBlinnPhong.createUniform("ks");
    GLuint bp_phong = shaderBlinnPhong.createUniform("phongExp");
    GLuint bp_lpos = shaderBlinnPhong.createUniform("light.position");
    GLuint bp_lint = shaderBlinnPhong.createUniform("light.intensity");
    GLuint bp_texUnit = shaderBlinnPhong.createUniform("texUnit");

    GLuint ocean_proj = shaderOcean.createUniform("projMatrix");
    GLuint ocean_view = shaderOcean.createUniform("viewMatrix");
    GLuint ocean_model = shaderOcean.createUniform("modelMatrix");
    GLuint ocean_time = shaderOcean.createUniform("time");

    perspectiveCamera cam(winWidth, winHeight, vec3(0, 0, 5), vec3(0, 0, -1), 1.0f, 10.0f, 10.0f);

    Mouse cameraRotation;
    glfwSetWindowUserPointer(window, &cameraRotation);
    glfwSetCursorPosCallback(window, Mouse::callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    double timeDiff = 0.0, startFrameTime = 0.0, endFrameTime = 0.0;
    float rotAngle = 0.0f;
    bool useBlinnPhong = false;

    // =====================================================================
    // FBO SETUP: Create framebuffer with color texture and depth renderbuffer
    // =====================================================================
    GLuint fboID, fboTextureID, fboRBOID;

    // Generate FBO
    glGenFramebuffers(1, &fboID);
    glBindFramebuffer(GL_FRAMEBUFFER, fboID);

    // Create color texture attachment
    glGenTextures(1, &fboTextureID);
    glBindTexture(GL_TEXTURE_2D, fboTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fb_width, fb_height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTextureID, 0);

    // Create depth renderbuffer
    glGenRenderbuffers(1, &fboRBOID);
    glBindRenderbuffer(GL_RENDERBUFFER, fboRBOID);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, fb_width, fb_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fboRBOID);

    // Check FBO completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer is not complete!" << std::endl;
        exit(EXIT_FAILURE);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // =====================================================================
    // SCREEN-FILLING QUAD: For post-processing pass
    // =====================================================================
    GLuint screenQuadVBO, screenQuadVAO;
  
    // Screen quad vertices: (position xy, texcoord xy)
    std::vector<float> screenQuadVertices = {
    // positions        // texCoords
    -1.0f,  1.0f,        0.0f, 1.0f,  // Top Left (V0)
    -1.0f, -1.0f,        0.0f, 0.0f,  // Bottom Left (V1)
    1.0f,  1.0f,        1.0f, 1.0f,  // Top Right (V2)
    1.0f, -1.0f,        1.0f, 0.0f   // Bottom Right (V3)
    };

    glGenBuffers(1, &screenQuadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, screenQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, screenQuadVertices.size() * sizeof(float), screenQuadVertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenVertexArrays(1, &screenQuadVAO);
    glBindVertexArray(screenQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, screenQuadVBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (const GLvoid *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (const GLvoid *)(2 * sizeof(float)));
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // =====================================================================
    // GAMMA CORRECTION POST-PROCESSING SHADER
    // =====================================================================
    sivelab::GLSLObject gammaShader;
    gammaShader.addShader("vertexShader_screenQuad.glsl", sivelab::GLSLObject::VERTEX_SHADER);
    gammaShader.addShader("fragmentShader_gammaCorrection.glsl", sivelab::GLSLObject::FRAGMENT_SHADER);
    gammaShader.createProgram();

    GLuint gammaTextureID = gammaShader.createUniform("fboTexture");
    GLuint gammaGammaID = gammaShader.createUniform("gamma");

    float gammaValue = 2.2f;  // Standard gamma value    

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        endFrameTime = glfwGetTime();
        timeDiff = endFrameTime - startFrameTime;
        startFrameTime = glfwGetTime();

        // Get current framebuffer size for FBO viewport
        glfwGetFramebufferSize(window, &fb_width, &fb_height);

        // =====================================================================
        // PASS 1: Render scene to FBO
        // =====================================================================
        glEnable(GL_DEPTH_TEST);
        glBindFramebuffer(GL_FRAMEBUFFER, fboID);  // <<<<<-----------
        glViewport(0, 0, fb_width, fb_height);

        // Clear the window's buffer (or clear the screen to our
        // background color)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //update camera
        glm::vec3 front = cameraRotation.getCameraFront();
        cam.setDir(vec3(front.x, front.y, front.z));

        // create the view matrix from our camera data                                                                                                   
        //glm::mat4 M_view = glm::lookAt( m_pos, m_pos - m_W, m_V );
        vec3 p = cam.getPos();
        vec3 u = cam.getU();
        vec3 v = cam.getV();
        vec3 w = cam.getW();
        glm::vec3 camPos(p.x(), p.y(), p.z());
        glm::vec3 camU(u.x(), u.y(), u.z());
        glm::vec3 camV(v.x(), v.y(), v.z());
        glm::vec3 camW(w.x(), w.y(), w.z());
        glm::mat4 M_view = glm::lookAt(camPos, camPos - camW, camV);

        /* Render your objects here */

        rotAngle += 0.01;
        if (rotAngle > 2.0*3.14159) rotAngle = 0.0f;

        sivelab::GLSLObject& activeShader = useBlinnPhong ? shaderBlinnPhong : shaderLambertian;

        GLuint u_proj = useBlinnPhong ? bp_proj : lamb_proj;
        GLuint u_view = useBlinnPhong ? bp_view : lamb_view;
        GLuint u_model = useBlinnPhong ? bp_model : lamb_model;
        GLuint u_normal = useBlinnPhong ? bp_normal : lamb_normal;
        GLuint u_Ia = useBlinnPhong ? bp_Ia : lamb_Ia;
        GLuint u_ka = useBlinnPhong ? bp_ka : 0;
        GLuint u_kd = useBlinnPhong ? bp_kd : lamb_kd;
        GLuint u_ks = useBlinnPhong ? bp_ks : 0;
        GLuint u_phong = useBlinnPhong ? bp_phong : 0;
        GLuint u_lpos = useBlinnPhong ? bp_lpos : lamb_lpos;
        GLuint u_lint = useBlinnPhong ? bp_lint : lamb_lint;
    
        activeShader.activate();

        glUniformMatrix4fv(u_proj, 1, GL_FALSE, glm::value_ptr(M_perspective));
        glUniformMatrix4fv(u_view, 1, GL_FALSE, glm::value_ptr(M_view));

        glUniform3f(u_Ia, 0.2f, 0.2f, 0.2f);
        glUniform3f(u_ka, 1.0f, 1.0f, 1.0f);
        glUniform3f(u_ks, 1.0f, 1.0f, 1.0f);
        glUniform1f(u_phong, 32.0f);
        glUniform3f(u_lint, 1.0f, 1.0f, 1.0f);

        if (useBlinnPhong) {
            glUniform3f(bp_lpos, 0.0f, 5.0f, 10.0f);
        } else {
            glUniform4f(lamb_lpos, 0.0f, 5.0f, 10.0f, 1.0f);
        }

        //obj
        glm::mat4 objTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.75f, 0.0f));
        objTransform = glm::scale(objTransform, glm::vec3(3.0f, 3.0f, 3.0f));
        glm::mat4 objNormal = glm::transpose(glm::inverse(objTransform));

        glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(objTransform));
        glUniformMatrix4fv(u_normal, 1, GL_FALSE, glm::value_ptr(objNormal));
        glUniform3f(u_kd, 0.48f, 0.33f, 0.18f);

        glBindVertexArray(objVAO);
        glDrawElements(GL_TRIANGLES, model.getNumberOfIndices(), GL_UNSIGNED_INT, (void*)0);
        glBindVertexArray(0);
        
        /*
        //sphere 1
        glm::mat4 sphereTransform = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 5.0f, 0.0f));
        sphereTransform = glm::scale(sphereTransform, glm::vec3(3.0f, 3.0f, 3.0f));
        glm::mat4 sphereNormal = glm::transpose(glm::inverse(sphereTransform));

        glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(sphereTransform));
        glUniformMatrix4fv(u_normal, 1, GL_FALSE, glm::value_ptr(sphereNormal));
        glUniform3f(u_kd, 0.96f, 0.91f, 0.61f);

        glBindVertexArray(sphereVAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)sphereIndices.size(), GL_UNSIGNED_INT, (void*)0);
        glBindVertexArray(0);
        
        //sphere 2
        glm::mat4 sphere2Transform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -11.0f, 0.0f));
        sphere2Transform = glm::scale(sphere2Transform, glm::vec3(10.0f, 10.0f, 10.0f));
        glm::mat4 sphere2RS = glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 10.0f, 10.0f));
        glm::mat4 sphere2Normal = glm::transpose(glm::inverse(sphere2RS));

        glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(sphere2Transform));
        glUniformMatrix4fv(u_normal, 1, GL_FALSE, glm::value_ptr(sphere2Normal));
        glUniform3f(u_kd, 1.0f, 0.0f, 0.0f);

        glBindVertexArray(sphereVAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)sphereIndices.size(), GL_UNSIGNED_INT, (void*)0);
        glBindVertexArray(0);

        //sphere 3
        glm::mat4 sphere3Transform = glm::rotate(glm::mat4(1.0f), rotAngle, glm::vec3(0, 1, 0));
        sphere3Transform = glm::translate(sphere3Transform, glm::vec3(0.0f, 0.0f, -4.0f));
        glm::mat4 sphere3Normal = glm::transpose(glm::inverse(sphere3Transform));

        glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(sphere3Transform));
        glUniformMatrix4fv(u_normal, 1, GL_FALSE, glm::value_ptr(sphere3Normal));
        glUniform3f(u_kd, 0.0f, 1.0f, 0.0f);

        glBindVertexArray(sphereVAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)sphereIndices.size(), GL_UNSIGNED_INT, (void*)0);
        glBindVertexArray(0);

        //Square
        glm::mat4 squareTransform = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.0f, 0.0f));
        glm::mat4 squareNormal = glm::transpose(glm::inverse(squareTransform));

        glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(squareTransform));
        glUniformMatrix4fv(u_normal, 1, GL_FALSE, glm::value_ptr(squareNormal));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texID);
        if (useBlinnPhong)
            glUniform1i(bp_texUnit, 0);

        glBindVertexArray(triangleVAO);
        glDrawArrays(GL_TRIANGLES, 0, 12);
        glBindVertexArray(0);

        glBindTexture(GL_TEXTURE_2D, 0);
        */

        activeShader.deactivate();

        //Ocean
        shaderOcean.activate();
        glUniformMatrix4fv(ocean_proj, 1, GL_FALSE, glm::value_ptr(M_perspective));
        glUniformMatrix4fv(ocean_view, 1, GL_FALSE, glm::value_ptr(M_view));

        glm::mat4 oceanTransform = glm::mat4(1.0f);
        glUniformMatrix4fv(ocean_model, 1, GL_FALSE, glm::value_ptr(oceanTransform));
        glUniform1f(ocean_time, (float)glfwGetTime());

        glBindVertexArray(gridVAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)gridIndices.size(), GL_UNSIGNED_INT, (void*)0);
        glBindVertexArray(0);
        shaderOcean.deactivate();

        // =====================================================================
        // PASS 2: Render FBO texture to back buffer with gamma correction
        // =====================================================================
        glBindFramebuffer(GL_FRAMEBUFFER, 0);  // Bind default framebuffer (back buffer)
        glViewport(0, 0, fb_width, fb_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
        // Disable depth testing for the post-processing pass to ensure the
        // screen quad renders completely without depth conflicts with the
        // depth buffer from Pass 1 scene rendering
        glDisable(GL_DEPTH_TEST);
    
        gammaShader.activate();

        // Bind FBO color texture to texture unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fboTextureID);
        glUniform1i(gammaTextureID, 0);

        // Set gamma value
        glUniform1f(gammaGammaID, gammaValue);

        // Draw screen-filling quad
        glBindVertexArray(screenQuadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);

        glBindTexture(GL_TEXTURE_2D, 0);

        gammaShader.deactivate();

        // Swap the front and back buffers
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();

        float moveRatePerFrame = 0.05f;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            cam.setPos(cam.getPos() + -cam.getW() * moveRatePerFrame);        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            cam.setPos(cam.getPos() - cam.getU() * moveRatePerFrame);        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            cam.setPos(cam.getPos() + cam.getW() * moveRatePerFrame);        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            cam.setPos(cam.getPos() + cam.getU() * moveRatePerFrame);        }

        //bind blinn phong and lambertian keys
        if(glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) useBlinnPhong = true;
        if(glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) useBlinnPhong = false;

        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
            std::cout << "fps: " << 1.0 / timeDiff << std::endl;
        }

        if (glfwGetKey( window, GLFW_KEY_T ) == GLFW_PRESS) {
            std::cout << "fps: " << 1.0/timeDiff << std::endl;
        }
        if (glfwGetKey( window, GLFW_KEY_ESCAPE ) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, 1);
        }
    }

    glDeleteBuffers(1, &objVBO);
    glDeleteBuffers(1, &objIBO);
    glDeleteVertexArrays(1, &objVAO);
    glDeleteBuffers(1, &sphereVBO);
    glDeleteBuffers(1, &sphereIBO);
    glDeleteVertexArrays(1, &sphereVAO);
    glDeleteBuffers(1, &triangleVBO);
    glDeleteVertexArrays(1, & triangleVAO);
    glDeleteBuffers(1, &gridVBO);
    glDeleteBuffers(1, &gridIBO);
    glDeleteVertexArrays(1, &gridVAO);
  
    glfwTerminate();
    return 0;
}
