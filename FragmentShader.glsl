#version 460 core
out vec3 color;
in vec3 localPos;

void main(){
     color = localPos + 0.5;
     // color = vec3(1) * dot(localPos,vec3(1,1,0));
}