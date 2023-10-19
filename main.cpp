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

using namespace std;

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

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    GLuint simpleShaderID = LoadShaders("shaders/localPosition");
    GLuint instancingShaderID = LoadShaders("shaders/instancing");

    float dt = 0.0f;
    float t = 0.0f;
    // keep track of last frame time
    float fpsAverageTime = 1.0f;
    double lastFrame = glfwGetTime();
    double fpsLastTime = glfwGetTime();
    int frameCount = 0;

    Fluid fluid(instancingShaderID);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    float phi = 0;
    float theta = 0;
    float r = 5;
    float rSpeed = 1;
    float phiSpeed = 100;
    float thetaSpeed = 100;
    float rotateSpeed = 100;
    Camera::mainCamera.position = vec3(0, 0, 5);
    Camera::mainCamera.target = vec3(0, 0, 0);
    dvec2 cursorPos;
    dvec2 prevCursorPos;
    dvec2 deltaCursor;
    do
    {
        deltaCursor = cursorPos - prevCursorPos;
        prevCursorPos = cursorPos;
        glfwGetCursorPos(window, &cursorPos[0], &cursorPos[1]);

        glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            phi -= deltaCursor.x * phiSpeed * dt;
            theta += deltaCursor.y * thetaSpeed * dt;
        }
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        {
            r = 5;
            phi = 0;
            theta = 0;
        }
        phi = fmod(phi, 360);
        theta = clamp(theta, -80.0f, 80.0f);

        Camera::mainCamera.position = vec3(
            r * cos(radians(theta)) * sin(radians(phi)),
            r * sin(radians(theta)),
            r * cos(radians(theta)) * cos(radians(phi)));

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
            cout << frameCount / (currentFrame - fpsLastTime) << "fps" << endl;
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