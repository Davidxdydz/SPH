#pragma once
#include "RenderObject.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "shapes.h"

Transform::Transform(vec3 position, vec3 rotation, vec3 scale)
{
    this->position = position;
    this->rotation = rotation;
    this->scale = scale;
}

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

BufferAttribute::BufferAttribute(GLuint bufferID, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer)
{
    this->bufferID = bufferID;
    this->index = index;
    this->size = size;
    this->type = type;
    this->normalized = normalized;
    this->stride = stride;
    this->pointer = pointer;
}

SpheresRenderer::SpheresRenderer(GLuint shaderID, std::vector<GLuint> bufferIds, std::vector<BufferAttribute> bufferAttributes, std::vector<Transform> &transforms, int subdivisions) : transforms(transforms), shaderID(shaderID), bufferAttributes(bufferAttributes), bufferIDs(bufferIds)
{
    IndexedMesh sphere = make_icosphere(1);
    vertexCount = sphere.first.size();
    triangleCount = sphere.second.size();
    transformCount = transforms.size();

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(vec3), &sphere.first[0], GL_STATIC_DRAW);

    glGenBuffers(1, &triangleBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangleCount * sizeof(Triangle), &sphere.second[0], GL_STATIC_DRAW);
    vpMatrixID = glGetUniformLocation(shaderID, "vp");

    glGenBuffers(1, &transformsBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, transformsBuffer);
    glBufferData(GL_ARRAY_BUFFER, transformCount * sizeof(Transform), &transforms[0], GL_DYNAMIC_DRAW);
    positionsAttribute = BufferAttribute(transformsBuffer, 1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)0);
    rotationsAttribute = BufferAttribute(transformsBuffer, 2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(3 * sizeof(float)));
    scalesAttribute = BufferAttribute(transformsBuffer, 3, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(6 * sizeof(float)));
}

void SpheresRenderer::draw()
{
    glUseProgram(shaderID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleBuffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    mat4 view = Camera::mainCamera.getViewMatrix();
    mat4 projection = Camera::mainCamera.getProjectionMatrix();
    mat4 vp = projection * view;
    glUniformMatrix4fv(vpMatrixID, 1, GL_FALSE, &vp[0][0]);
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
    glBindBuffer(GL_ARRAY_BUFFER, transformsBuffer);
    glBufferData(GL_ARRAY_BUFFER, transforms.size() * sizeof(Transform), &transforms[0], GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(positionsAttribute.index);
    glVertexAttribPointer(
        positionsAttribute.index,
        positionsAttribute.size,
        positionsAttribute.type,
        positionsAttribute.normalized,
        positionsAttribute.stride,
        positionsAttribute.pointer);
    glVertexAttribDivisor(positionsAttribute.index, 1);
    glEnableVertexAttribArray(rotationsAttribute.index);
    glVertexAttribPointer(
        rotationsAttribute.index,
        rotationsAttribute.size,
        rotationsAttribute.type,
        rotationsAttribute.normalized,
        rotationsAttribute.stride,
        rotationsAttribute.pointer);
    glVertexAttribDivisor(rotationsAttribute.index, 1);
    glEnableVertexAttribArray(scalesAttribute.index);
    glVertexAttribPointer(
        scalesAttribute.index,
        scalesAttribute.size,
        scalesAttribute.type,
        scalesAttribute.normalized,
        scalesAttribute.stride,
        scalesAttribute.pointer);
    glVertexAttribDivisor(scalesAttribute.index, 1);
    glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)triangleCount * 3, GL_UNSIGNED_INT, (void *)0, transforms.size());
}

RenderObject::RenderObject(GLuint shaderID, GLsizei vertexCount, std::vector<BufferAttribute> vertexAttributes, GLuint mvpMatrixID, vec3 position, vec3 rotation, vec3 scale)
{
    transform = Transform(
        position,
        rotation,
        scale);
    this->shaderID = shaderID;
    this->vertexCount = vertexCount;
    this->bufferAttributes = vertexAttributes;
    this->mvpMatrixID = mvpMatrixID;
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
