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
    GLuint divisor;
    BufferAttribute(GLuint bufferID = 0, GLuint index = 0, GLint size = 0, GLenum type = 0, GLboolean normalized = false, GLsizei stride = 0, const GLvoid *pointer = (void *)0, GLuint divisor = 0)
        : bufferID(bufferID), index(index), size(size), type(type), normalized(normalized), stride(stride), pointer(pointer), divisor(divisor){};
    void set() const;
};
struct VertexAttribute
{
    GLuint index;
    GLint size;
    GLenum type;
    GLboolean normalized;
    GLsizei stride;
    const GLvoid *pointer;
    VertexAttribute(GLuint index = 0, GLint size = 0, GLenum type = 0, GLboolean normalized = false, GLsizei stride = 0, const GLvoid *pointer = (void *)0)
        : index(index), size(size), type(type), normalized(normalized), stride(stride), pointer(pointer){};
    void set() const;
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
    SpheresRenderer(GLuint shaderID, std::vector<Transform> &transforms, std::vector<vec3> &colors, int subdivisions = 1);
    void draw();

protected:
    GLuint vertexBuffer;
    GLuint triangleBuffer;
    GLuint transformsBuffer;
    GLuint colorsBuffer;
    GLsizei triangleCount;
    GLsizei vertexCount;
    GLsizei transformCount;
    GLuint vpMatrixID;
    std::vector<Transform> &transforms;
    std::vector<vec3> &colors;
    BufferAttribute positionsAttribute;
    BufferAttribute rotationsAttribute;
    BufferAttribute scalesAttribute;
    BufferAttribute colorsAttribute;
    VertexAttribute vertexAttribute;
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
    RenderObject(GLuint shaderID, GLsizei vertexCount, std::vector<BufferAttribute> vertexAttributes, vec3 position = vec3(0), vec3 rotation = vec3(0), vec3 scale = vec3(1));
    void draw();
};