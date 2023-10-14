#version 460 core
out vec3 color;
in vec3 localPos;
in vec3 p;
in vec3 r;
in vec3 s;

void main(){
     color=localPos+.5;
     // color = s;
     // color = vec3(1) * dot(localPos,vec3(1.0, 1.0, 0.0));
}