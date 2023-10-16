#version 460 core
out vec3 color;
in vec3 baseColor;
in vec3 p;
in vec3 r;
in vec3 s;

void main(){
     color=baseColor;
     // color = s;
     // color = vec3(1) * dot(localPos,vec3(1.0, 1.0, 0.0));
}