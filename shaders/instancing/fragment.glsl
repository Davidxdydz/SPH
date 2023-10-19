#version 460
out vec3 color;
in vec3 baseColor;
in vec3 normal;
in vec3 lightDirection;

void main(){
     color=clamp(dot(lightDirection,normal),0,1)*baseColor;
     // color = baseColor;
     // color = normal * 0.5 + 0.5;
}