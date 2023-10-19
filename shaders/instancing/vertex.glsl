#version 460

in vec3 vertex;
in vec3 position;
in vec3 rotation;
in vec3 scale;
in vec3 color;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 lightDir;
out vec3 baseColor;
out vec3 normal;
out vec3 lightDirection;

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
    // build the rotation matrix
    mat4 rotX = rotationX(radians(rotation.x));
    mat4 rotY = rotationY(radians(rotation.y));
    mat4 rotZ = rotationZ(radians(rotation.z));
    mat4 rot = rotZ*rotY*rotX;

    // translation matrix is identity, with last column being the translation
    mat4 translate = mat4(1.0);
    translate[3] = vec4(position, 1.0);

    // scale matrix is identity, with scale in x,y and z on the diagonal
    mat4 scaleMat = mat4(1.0);
    scaleMat[0][0] = scale.x;
    scaleMat[1][1] = scale.y;
    scaleMat[2][2] = scale.z;

    // model matrix is first rotate, then scale, then translate
    mat4 model = translate*scaleMat*rot;

    // the camera projection space position of the vertex is
    // object space position to world space position
    // then world space position to camera space position
    // then camera space position to projection space position
    gl_Position=projection*view*model*vec4(vertex,1);


    baseColor = color;
    // it's a sphere, so the normal is just the vertex after rotation
    normal = -vertex;
    normal = (rot * vec4(normal,1)).xyz;
    // light direction stays in world space
    lightDirection = normalize(lightDir);

}
