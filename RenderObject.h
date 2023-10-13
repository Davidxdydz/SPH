#pragma once
#include <GL/glew.h>
#include <vector>

#include <glm/glm.hpp>
using namespace glm;

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

class Camera
{
public:
    vec3 position;
    vec3 target;
    float near;
    float far;
    float fov;
    int width;
    int height;
    static Camera mainCamera;
    Camera(vec3 position = vec3(0, 0, 5), vec3 target = vec3(0), float near = 0.01f, float far = 1000, int width = 1000, int height = 1000, float fov = 60)
    {
        this->position = position;
        this->target = target;
        this->width = width;
        this->height = height;
        this->fov = fov;
        this->near = near;
        this->far = far;
    }
    mat4 getViewMatrix()
    {
        mat4 view = lookAt(position, target, vec3(0, 1, 0));
        return view;
    }
    mat4 getProjectionMatrix()
    {
        return perspective(radians(fov), (float)width / (float)height, near, far);
    }
};
Camera Camera::mainCamera = Camera();

struct VertexAttribute
{
    GLuint bufferID;
    GLuint index;
    GLint size;
    GLenum type;
    GLboolean normalized;
    GLsizei stride;
    const GLvoid *pointer;
    VertexAttribute(GLuint bufferID, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer)
    {
        this->bufferID = bufferID;
        this->index = index;
        this->size = size;
        this->type = type;
        this->normalized = normalized;
        this->stride = stride;
        this->pointer = pointer;
    }
};

class RenderObject
{
public:
    vec3 position;
    vec3 rotation; // euler angles in degrees
    vec3 scale;
    GLuint shaderID;
    GLuint mvpMatrixID;
    GLsizei vertexCount;
    std::vector<GLuint> bufferIDs;
    std::vector<VertexAttribute> vertexAttributes;
    RenderObject(GLuint shaderID, GLsizei vertexCount, std::vector<VertexAttribute> vertexAttributes, GLuint mvpMatrixID, vec3 position = vec3(0), vec3 rotation = vec3(0), vec3 scale = vec3(1))
    {
        this->position = position;
        this->rotation = rotation;
        this->scale = scale;
        this->shaderID = shaderID;
        this->vertexCount = vertexCount;
        this->vertexAttributes = vertexAttributes;
        this->mvpMatrixID = mvpMatrixID;
    }
    void draw()
    {
        mat4 model = translate(mat4(1.0f), position);
        model = rotate(model, radians(rotation.x), vec3(1, 0, 0));
        model = rotate(model, radians(rotation.y), vec3(0, 1, 0));
        model = rotate(model, radians(rotation.z), vec3(0, 0, 1));
        model = glm::scale(model, scale);
        mat4 view = Camera::mainCamera.getViewMatrix();
        mat4 projection = Camera::mainCamera.getProjectionMatrix();
        mat4 mvp = projection * view * model;
        glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);
        for (const VertexAttribute &va : vertexAttributes)
        {
            glEnableVertexAttribArray(va.index);
            glBindBuffer(GL_ARRAY_BUFFER, va.bufferID);
            glVertexAttribPointer(
                va.index,
                va.size,
                va.type,
                va.normalized,
                va.stride,
                va.pointer);
        }
        glUseProgram(shaderID);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }
};