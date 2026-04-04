#include <cstdlib>
#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "../src/perspectiveCamera.h"
#include "../src/vec3.h"

#include "GLSL.h"

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
    glClearColor(0.0, 0.0, 0.0, 1.0);

    int fb_width, fb_height;
    glfwGetFramebufferSize(window, &fb_width, &fb_height);
    glViewport(0, 0, fb_width, fb_height);

    // Need to set a projection matrix that fits the aspect ratio set
    // by the window frame.
    //
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
    glm::mat4 M_perspective = glm::perspective(3.14159f/4.0f, 1.0f, 0.1f, 100.0f);
    glm::mat4 projectionMatrix = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, -10.0f, 10.0f);

    GLint major_version;
    glGetIntegerv(GL_MAJOR_VERSION, &major_version);
    std::cout << "GL_MAJOR_VERSION: " << major_version << std::endl;

    //initialize all my data and get it on the GPU(load scene file, get shapes, etc.)
    GLuint m_triangleVBO[1], m_VAO;
    sivelab::GLSLObject shader;

    // create a Vertex Array Buffer to hold our triangle data                                               
    glGenBuffers(1, m_triangleVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_triangleVBO[0]);

    // this is the actual triangle data that will be copied to                                              
    // the GPU memory                                                                                       
    std::vector< float > host_VertexBuffer{ 
        //position              //normals
        -3.0f, -3.0f, 0.0f,     0.0f, 0.0f, 1.0f,                     
        3.0f, -3.0f, 0.0f,      0.0f, 0.0f, 1.0f,                              
        0.0f, 3.0f, 0.0f,       0.0f, 0.0f, 1.0f,
    };                              

    int numBytes = host_VertexBuffer.size() * sizeof(float);

    // copy the numBytes from host_VertexBuffer t the GPU and store in                                      
    // the currently bound VBO                                                                              
    glBufferData(GL_ARRAY_BUFFER, numBytes, host_VertexBuffer.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // once copied, we no longer need the data on the host                                                  
    host_VertexBuffer.clear();

    // create a vertex array object that will map the attributes in                                         
    // our vertex buffer to different location attributes for our                                           
    // shaders                                                                                              
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    // VAO details here - we only have 1 attribute or location                                              
    // (Position of the vertex)                                                                             
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_triangleVBO[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);

    // Create a shader using my GLSLObject class                                                            
    shader.addShader( "vertexShader_PrepForPerFragment.glsl", sivelab::GLSLObject::VERTEX_SHADER );
    shader.addShader( "fragmentShader_Lambertian.glsl", sivelab::GLSLObject::FRAGMENT_SHADER );
    shader.createProgram();
    GLuint projMatrixID, viewMatrixID, modelMatrixID, normalMatrixID;
    projMatrixID = shader.createUniform("projMatrix");
    viewMatrixID = shader.createUniform("viewMatrix");
    modelMatrixID = shader.createUniform("modelMatrix");
    normalMatrixID = shader.createUniform("normalMatrix");

    GLuint diffuseComponentID, lightPosWorldID, IaID, kaID, kdID, ksID, phongExpID, lightPosID, lightIntensityID;
    diffuseComponentID = shader.createUniform("diffuseComponent");
    lightPosWorldID = shader.createUniform("lightPosWorld");
    IaID = shader.createUniform("Ia");
    kaID = shader.createUniform("ka");
    kdID = shader.createUniform("kd");
    ksID = shader.createUniform("ks");
    phongExpID = shader.createUniform("phongExp");
    lightPosID = shader.createUniform("light.position");
    lightIntensityID = shader.createUniform("light.intensity");

    glm::vec3 diffuseComponent(1.0f, 0.0f, 0.5f);
    glm::vec4 lightPosWorld(0.0f, 0.0f, 5.0f, 1.0f);

    glm::mat4 modelNormal = glm::mat4(1.0);
    glm::mat4 modelTransform = glm::mat4(1.0);
    float rot = 0;
    modelTransform = glm::rotate(modelTransform, rot, glm::vec3(0,1,0));

    //glm::vec3 m_pos(0,0,0), m_viewDir(0,0,-1);
    //glm::vec3 m_U(1,0,0), m_V(0,1,0), m_W(0,0,1);

    perspectiveCamera cam(winWidth, winHeight, vec3(0, 0, 5), vec3(0, 0, -1), 1.0f, 10.0f, 10.0f);

    double timeDiff = 0.0, startFrameTime = 0.0, endFrameTime = 0.0;

    float rotAngle = 0;
    
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        endFrameTime = glfwGetTime();
        timeDiff = endFrameTime - startFrameTime;
        startFrameTime = glfwGetTime();

        // Clear the window's buffer (or clear the screen to our
        // background color)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
        shader.activate();

        modelTransform = glm::mat4(1.0f);
        modelTransform = glm::rotate(modelTransform, rotAngle, glm::vec3(0,1,0));
        rotAngle += 0.01;
        if (rotAngle > 2.0*3.14159) rotAngle = 0.0f;

        modelNormal = glm::transpose(glm::inverse(modelTransform));

        glUniformMatrix4fv(projMatrixID, 1, GL_FALSE, glm::value_ptr( M_perspective ));
        glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, glm::value_ptr( M_view )); 
        glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, glm::value_ptr( modelTransform ));
        glUniformMatrix4fv(normalMatrixID, 1, GL_FALSE, glm::value_ptr( modelNormal ));

        glUniform3fv(diffuseComponentID, 1, glm::value_ptr( diffuseComponent ));
        glUniform4fv(lightPosWorldID, 1, glm::value_ptr( lightPosWorld ));

        glUniform3f(IaID, 0.2f, 0.2f, 0.2f);
        glUniform3f(kaID, 1.0f, 1.0f, 1.0f);
        glUniform3f(kdID, 1.0f, 0.0f, 0.5f);
        glUniform3f(ksID, 1.0f, 1.0f, 1.0f);
        glUniform1f(phongExpID, 32.0f);
        glUniform3f(lightPosID, 0.0f, 0.0f, 10.0f);
        glUniform3f(lightIntensityID, 1.0f, 1.0f, 1.0f);

        glBindVertexArray(m_VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        shader.deactivate();

        // Swap the front and back buffers
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();

        float moveRatePerFrame = 0.05;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            cam.setPos(cam.getPos() + -cam.getW() * moveRatePerFrame);
        }
        else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            cam.setPos(cam.getPos() - cam.getU() * moveRatePerFrame);
        }
        else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            cam.setPos(cam.getPos() + cam.getW() * moveRatePerFrame);
        }
        else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            cam.setPos(cam.getPos() + cam.getU() * moveRatePerFrame);
        }

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
  
    glfwTerminate();
    return 0;
}
