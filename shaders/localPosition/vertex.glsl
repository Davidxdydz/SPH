#version 460 core

layout(location=0)in vec3 vertexPosition_modelspace;

uniform mat4 MVP;
out vec3 localPos;

void main(){
    gl_Position=MVP*vec4(vertexPosition_modelspace,1);
    localPos = vertexPosition_modelspace;
}
