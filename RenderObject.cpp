#pragma once
#include "RenderObject.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "shapes.h"

using namespace glm;

mat4 Transform::getMatrix()
{
    mat4 model = translate(mat4(1.0f), position);
    model = rotate(model, radians(rotation.x), vec3(1, 0, 0));
    model = rotate(model, radians(rotation.y), vec3(0, 1, 0));
    model = rotate(model, radians(rotation.z), vec3(0, 0, 1));
    model = glm::scale(model, scale);
    return model;
}

Camera::Camera(vec3 position, vec3 target, float near, float far, int width, int height, float fov)
{
    this->position = position;
    this->target = target;
    this->width = width;
    this->height = height;
    this->fov = fov;
    this->near = near;
    this->far = far;
}
Camera Camera::mainCamera = Camera();

mat4 Camera::getViewMatrix()
{
    mat4 view = lookAt(position, target, vec3(0, 1, 0));
    return view;
}
mat4 Camera::getProjectionMatrix()
{
    return perspective(radians(fov), (float)width / (float)height, near, far);
}

void BufferAttribute::set() const
{
    glEnableVertexAttribArray(index);
    glBindBuffer(GL_ARRAY_BUFFER, bufferID);
    glVertexAttribPointer(
        index,
        size,
        type,
        normalized,
        stride,
        pointer);
    glVertexAttribDivisor(index, divisor);
}

void VertexAttribute::set() const
{
    glEnableVertexAttribArray(index);
    glVertexAttribPointer(
        index,
        size,
        type,
        normalized,
        stride,
        pointer);
}

SpheresRenderer::SpheresRenderer(GLuint shaderID, std::vector<Transform> &transforms, std::vector<vec3> &colors, int subdivisions)
    : transforms(transforms), shaderID(shaderID), colors(colors)
{
    IndexedMesh sphere = make_icosphere(subdivisions);
    vertexCount = sphere.first.size();
    triangleCount = sphere.second.size();
    transformCount = transforms.size();

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(vec3), &sphere.first[0], GL_STATIC_DRAW);

    glGenBuffers(1, &triangleBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangleCount * sizeof(Triangle), &sphere.second[0], GL_STATIC_DRAW);

    glGenBuffers(1, &transformsBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, transformsBuffer);

    glGenBuffers(1, &colorsBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorsBuffer);

    vpMatrixID = glGetUniformLocation(shaderID, "vp");
    positionsAttribute = BufferAttribute(transformsBuffer, glGetAttribLocation(shaderID, "position"), 3, GL_FLOAT, GL_FALSE, sizeof(Transform), (void *)0, 1);
    rotationsAttribute = BufferAttribute(transformsBuffer, glGetAttribLocation(shaderID, "rotation"), 3, GL_FLOAT, GL_FALSE, sizeof(Transform), (void *)(3 * sizeof(float)), 1);
    scalesAttribute = BufferAttribute(transformsBuffer, glGetAttribLocation(shaderID, "scale"), 3, GL_FLOAT, GL_FALSE, sizeof(Transform), (void *)(6 * sizeof(float)), 1);
    colorsAttribute = BufferAttribute(colorsBuffer, glGetAttribLocation(shaderID, "color"), 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *)0, 1);
    vertexAttribute = VertexAttribute(glGetAttribLocation(shaderID, "vertex"), 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
}

void SpheresRenderer::draw()
{
    glUseProgram(shaderID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    vertexAttribute.set();

    mat4 view = Camera::mainCamera.getViewMatrix();
    mat4 projection = Camera::mainCamera.getProjectionMatrix();
    mat4 vp = projection * view;
    glUniformMatrix4fv(vpMatrixID, 1, GL_FALSE, &vp[0][0]);

    glBindBuffer(GL_ARRAY_BUFFER, transformsBuffer);
    glBufferData(GL_ARRAY_BUFFER, transforms.size() * sizeof(Transform), &transforms[0], GL_DYNAMIC_DRAW);

    positionsAttribute.set();
    rotationsAttribute.set();
    scalesAttribute.set();

    glBindBuffer(GL_ARRAY_BUFFER, colorsBuffer);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(vec3), &colors[0], GL_DYNAMIC_DRAW);

    colorsAttribute.set();

    glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)triangleCount * 3, GL_UNSIGNED_INT, (void *)0, transforms.size());
}

RenderObject::RenderObject(GLuint shaderID, GLsizei vertexCount, std::vector<BufferAttribute> vertexAttributes, vec3 position, vec3 rotation, vec3 scale)
{
    transform = Transform(
        position,
        rotation,
        scale);
    this->shaderID = shaderID;
    this->vertexCount = vertexCount;
    this->bufferAttributes = vertexAttributes;
    this->mvpMatrixID = glGetUniformLocation(shaderID, "MVP");
}
void RenderObject::draw()
{
    glUseProgram(shaderID);
    mat4 model = transform.getMatrix();
    mat4 view = Camera::mainCamera.getViewMatrix();
    mat4 projection = Camera::mainCamera.getProjectionMatrix();
    mat4 mvp = projection * view * model;
    glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);
    for (const BufferAttribute &va : bufferAttributes)
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
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}