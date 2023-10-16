#pragma once
#include <GL/glew.h>
#include <vector>

#include <glm/glm.hpp>
using namespace glm;

struct Transform
{
    vec3 position;
    vec3 rotation; // euler angles in degrees
    vec3 scale;
    Transform(vec3 position = vec3(0), vec3 rotation = vec3(0), vec3 scale = vec3(1)) : position(position), rotation(rotation), scale(scale){};
    mat4 getMatrix();
};

struct BufferAttribute
{
    GLuint bufferID;
    GLuint index;
    GLint size;
    GLenum type;
    GLboolean normalized;
    GLsizei stride;
    const GLvoid *pointer;
    BufferAttribute(GLuint bufferID = 0, GLuint index = 0, GLint size = 0, GLenum type = 0, GLboolean normalized = false, GLsizei stride = 0, const GLvoid *pointer = (void *)0);
};

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
    Camera(vec3 position = vec3(0, 0, 5), vec3 target = vec3(0), float near = 0.01f, float far = 1000, int width = 1000, int height = 1000, float fov = 60);
    mat4 getViewMatrix();
    mat4 getProjectionMatrix();
};

class SpheresRenderer
{
public:
    GLuint shaderID;
    std::vector<GLuint> bufferIDs;
    std::vector<BufferAttribute> bufferAttributes;
    SpheresRenderer(GLuint shaderID, std::vector<GLuint> bufferIds, std::vector<BufferAttribute> bufferAttributes, std::vector<Transform> &transforms, int subdivisions);
    void draw();

private:
    GLuint vertexBuffer;
    GLuint triangleBuffer;
    GLuint transformsBuffer;
    GLsizei triangleCount;
    GLsizei vertexCount;
    GLsizei transformCount;
    GLuint vpMatrixID;
    std::vector<Transform> &transforms;
    BufferAttribute positionsAttribute;
    BufferAttribute rotationsAttribute;
    BufferAttribute scalesAttribute;
};

class RenderObject
{
public:
    Transform transform;
    GLuint shaderID;
    GLuint mvpMatrixID;
    GLsizei vertexCount;
    std::vector<GLuint> bufferIDs;
    std::vector<BufferAttribute> bufferAttributes;
    RenderObject(GLuint shaderID, GLsizei vertexCount, std::vector<BufferAttribute> vertexAttributes, GLuint mvpMatrixID, vec3 position = vec3(0), vec3 rotation = vec3(0), vec3 scale = vec3(1));
    void draw();
};