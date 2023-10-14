#version 460 core

layout(location=0)in vec3 vertexPosition_modelspace;
layout(location=1) in vec3 position;
layout(location=2) in vec3 rotation;
layout(location=3) in vec3 scale;

uniform mat4 vp;
out vec3 localPos;
out vec3 p;
out vec3 r;
out vec3 s;

mat4 rotationX( in float angle ) {
	return mat4(	1.0,		0,			0,			0,
			 		0, 	cos(angle),	-sin(angle),		0,
					0, 	sin(angle),	 cos(angle),		0,
					0, 			0,			  0, 		1);
}

mat4 rotationY( in float angle ) {
	return mat4(	cos(angle),		0,		sin(angle),	0,
			 				0,		1.0,			 0,	0,
					-sin(angle),	0,		cos(angle),	0,
							0, 		0,				0,	1);
}

mat4 rotationZ( in float angle ) {
	return mat4(	cos(angle),		-sin(angle),	0,	0,
			 		sin(angle),		cos(angle),		0,	0,
							0,				0,		1,	0,
							0,				0,		0,	1);
}


void main(){
    mat4 model = mat4(1.0);
    model[3] = vec4(position, 1.0);
    model = rotationX(radians(rotation.x)) * rotationY(radians(rotation.y)) * rotationZ(radians(rotation.z))*model;
    mat4 scaleMat = mat4(0.0);
    scaleMat[0][0] = scale.x;
    scaleMat[1][1] = scale.y;
    scaleMat[2][2] = scale.z;
    scaleMat[3][3] = 1.0;
    model = model*scaleMat;
    gl_Position=vp*model*vec4(vertexPosition_modelspace,1);
    localPos=vertexPosition_modelspace;
    p = position;
    r = rotation;
    s = scale;

}
