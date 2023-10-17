#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
using namespace glm;

#include "RenderObject.h"
#include "loadShader.h"
#include "shapes.h"
#include "fluid.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>

int main()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

    GLFWwindow *window;
    window = glfwCreateWindow(Camera::mainCamera.width, Camera::mainCamera.height, "Test", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to open GLFW window." << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window); // Initialize GLEW
    glewExperimental = true;        // Needed in core profile
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // disable vsync
    glfwSwapInterval(0);

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    float fpsAverageTime = 1.0f;

    GLuint simpleShaderID = LoadShaders("shaders/localPosition");
    GLuint instancingShaderID = LoadShaders("shaders/instancing");

    float dt = 0.0f;
    float t = 0.0f;
    // keep track of last frame time
    double lastFrame = glfwGetTime();
    double fpsLastTime = glfwGetTime();
    int frameCount = 0;

    Fluid fluid(instancingShaderID);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    float degreesPerSecond = 100;
    do
    {
        glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        fluid.step();
        fluid.draw();

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
        double currentFrame = glfwGetTime();
        dt = (float)(currentFrame - lastFrame);
        t += dt;
        lastFrame = currentFrame;
        frameCount++;
        if (currentFrame - fpsLastTime > fpsAverageTime)
        {
            std::cout << frameCount / (currentFrame - fpsLastTime) << "fps\n";
            fpsLastTime = currentFrame;
            frameCount = 0;
        }

    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}