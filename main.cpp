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
    GLuint cubeVertexBuffer;
    glGenBuffers(1, &cubeVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    BufferAttribute cubeVertexLocations = BufferAttribute(cubeVertexBuffer, 0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    GLuint triangleVertexBuffer;
    glGenBuffers(1, &triangleVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, triangleVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);
    BufferAttribute triangleVertexLocations = BufferAttribute(triangleVertexBuffer, 0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    GLuint simpleShaderID = LoadShaders("shaders/localPosition");
    GLuint instancingShaderID = LoadShaders("shaders/instancing");

    GLuint MatrixID = glGetUniformLocation(simpleShaderID, "MVP");

    float dt = 0.0f;
    float t = 0.0f;
    // keep track of last frame time
    double lastFrame = glfwGetTime();
    double fpsLastTime = glfwGetTime();
    int frameCount = 0;

    // create a list of two transforms to draw
    std::vector<Transform> transforms;
    for (int i = 0; i < 100; i++)
    {
        transforms.push_back(Transform(vec3((float)i / 20.0f, 0, 0), vec3(0, 0, 0), vec3(0.01f)));
    }

    SpheresRenderer spheresRenderer(instancingShaderID, {}, {}, transforms, 1);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    RenderObject cube = RenderObject(
        simpleShaderID,
        sizeof(cubeVertices) / 3,
        {cubeVertexLocations},
        MatrixID);

    RenderObject cube2 = RenderObject(
        simpleShaderID,
        sizeof(cubeVertices) / 3,
        {cubeVertexLocations},
        MatrixID,
        vec3(1, 0, 0),
        vec3(0, 0, 0),
        vec3(0.5, 1, 0.5));

    float degreesPerSecond = 100;
    do
    {
        glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Camera::mainCamera.position.x = 5 * sin(radians(t * degreesPerSecond));
        // Camera::mainCamera.position.z = 5 * cos(radians(t * degreesPerSecond));

        cube.transform.rotation.x += dt * degreesPerSecond;
        cube.draw();
        float scale = (sin(radians(t * degreesPerSecond * 4)) + 1) / 2; // 0 to 1
        scale = scale * 0.5f + 0.5f;                                    // 0.5 to 1
        cube.transform.scale = vec3(scale);

        cube2.transform.rotation.y -= dt * degreesPerSecond;
        cube2.draw();

        for (int i = 0; i < transforms.size(); i++)
        {
            float arg = radians(t * degreesPerSecond);
            float phase = radians((float)i / 100 * 720);
            transforms[i].position.y = sin(arg + phase) * 0.5f;
            transforms[i].scale = vec3(sin(arg + phase + 180) + 1) * 0.02f / 2.0f + 0.01f;
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