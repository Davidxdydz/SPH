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
#include "sph.h"

#include <glm/gtc/matrix_transform.hpp>

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

    GLuint MatrixID = glGetUniformLocation(simpleShaderID, "MVP");

    float dt = 0.0f;
    float t = 0.0f;
    // keep track of last frame time
    double lastFrame = glfwGetTime();
    double fpsLastTime = glfwGetTime();
    int frameCount = 0;

    int nx = 20;
    int ny = 20;
    int nz = 1;
    int n = nx * ny * nz;
    std::vector<Transform> transforms;
    std::vector<vec3> colors;
    std::vector<vec3> vs;
    std::vector<float> densities = std::vector<float>(transforms.size(), 0);
    float fixedDt = 0.005f;
    float simulatedVolume = 1.0f;
    float gravity = 1.0f;
    float initialDensity = 200; // nx* ny* nz / simulatedVolume;
    float volume = simulatedVolume / nx / ny / nz;
    float h = 1.0f / pow(initialDensity, 1 / 2) / 2;
    float displayRaius = 0.02f;
    float effectiveRadius = pow(volume * 3 / 4 / 3.1415926f, 1.0f / 3);
    float targetDensity = initialDensity/3;
    float stiffness = 15.0f;

    for (int x = 0; x < nx; x++)
    {
        for (int y = 0; y < ny; y++)
        {
            for (int z = 0; z < nz; z++)
            {
                float x0 = (float)x / nx;
                float y0 = (float)y / ny;
                float z0 = (float)z / nz;
                transforms.push_back(Transform(vec3(x0, y0, z0) - vec3(0.5), vec3(0), vec3(displayRaius)));
                vs.push_back(vec3(0));
                colors.push_back(vec3(x0, y0, z0));
            }
        }
    }

    SpheresRenderer spheresRenderer(instancingShaderID, transforms, colors, 2);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    float degreesPerSecond = 100;
    do
    {
        glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        sphStep(densities, transforms, vs, fixedDt, h, stiffness, targetDensity, gravity, 1, volume);
        applyBoundaries(transforms, vs, 0.1, gravity);

        for (int i = 0; i < densities.size(); i++)
        {
            float normalized = densities[i] / targetDensity;
            colors[i] = vec3(normalized, 1 - normalized, 0);
        }
        spheresRenderer.draw();

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