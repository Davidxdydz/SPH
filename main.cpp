#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
using namespace glm;

#include "loadShader.h"

#include "shapes.h"

#include "RenderObject.h"

#include <glm/gtc/matrix_transform.hpp>

int main()
{
    float fpsAverageTime = 1.0f;
    glewExperimental = true; // Needed for core profile
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    GLFWwindow *window; // (In the accompanying source code, this variable is global for simplicity)
    window = glfwCreateWindow(Camera::mainCamera.width, Camera::mainCamera.height, "Test", NULL, NULL);
    if (window == NULL)
    {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window); // Initialize GLEW
    glewExperimental = true;        // Needed in core profile
    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }
    glfwSwapInterval(0);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    GLuint cubeVertexBuffer;
    glGenBuffers(1, &cubeVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    VertexAttribute cubeVertexLocations = VertexAttribute(cubeVertexBuffer, 0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    GLuint triangleVertexBuffer;
    glGenBuffers(1, &triangleVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, triangleVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);
    VertexAttribute triangleVertexLocations = VertexAttribute(triangleVertexBuffer, 0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    GLuint programID = LoadShaders("shaders/localPosition");
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");

    float dt = 0.0f;
    float t = 0.0f;
    // keep track of last frame time
    double lastFrame = glfwGetTime();
    double fpsLastTime = glfwGetTime();
    int frameCount = 0;

    IndexedMesh sphere = make_icosphere(1);

    GLuint sphereVertexBuffer;
    glGenBuffers(1, &sphereVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sphere.first.size() * sizeof(vec3), &sphere.first[0], GL_STATIC_DRAW);
    VertexAttribute sphereVertexLocations = VertexAttribute(sphereVertexBuffer, 0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    GLuint sphereTriangleBuffer;
    glGenBuffers(1, &sphereTriangleBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereTriangleBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere.second.size() * sizeof(Triangle), &sphere.second[0], GL_STATIC_DRAW);

    // glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    RenderObject cube = RenderObject(
        programID,
        sizeof(cubeVertices) / 3,
        {cubeVertexLocations},
        MatrixID);

    RenderObject cube2 = RenderObject(
        programID,
        sizeof(cubeVertices) / 3,
        {cubeVertexLocations},
        MatrixID,
        vec3(1, 0, 0),
        vec3(0, 0, 0),
        vec3(0.5, 1, 0.5));

    RenderObject triangle = RenderObject(
        programID,
        sizeof(triangleVertices) / 3,
        {triangleVertexLocations},
        MatrixID,
        vec3(-1, 0, 0),
        vec3(0, 0, 0),
        vec3(1, 1, 1));

    float degreesPerSecond = 100;
    do
    {
        glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Camera::mainCamera.position.x = 5 * sin(radians(t * degreesPerSecond));
        Camera::mainCamera.position.z = 5 * cos(radians(t * degreesPerSecond));

        cube.rotation.x += dt * degreesPerSecond;
        cube.draw();
        float scale = (sin(radians(t * degreesPerSecond * 4)) + 1) / 2; // 0 to 1
        scale = scale * 0.5 + 0.5;                                      // 0.5 to 1
        cube.scale = vec3(scale);

        cube2.rotation.y -= dt * degreesPerSecond;
        cube2.draw();

        triangle.rotation.z += dt * degreesPerSecond;
        triangle.draw();

        // draw the sphere
        glBindBuffer(GL_ARRAY_BUFFER, sphereVertexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereTriangleBuffer);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
        glDrawElements(GL_TRIANGLES, sphere.second.size() * 3, GL_UNSIGNED_INT, (void *)0);

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