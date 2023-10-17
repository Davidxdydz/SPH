#pragma once
#include <vector>
#include "RenderObject.h"

class Fluid
{
public:
    Fluid(GLuint instancingShaderID);
    void step();
    void draw();

private:
    float dt;
    float simulatedVolume;
    float gravity;
    float restDensity;
    float volume;
    float h;
    float displayRaius;
    float effectiveRadius;
    float targetDensity;
    float stiffness;
    float damping;
    float m;
    std::vector<Transform> transforms;
    std::vector<vec3> colors;
    std::vector<vec3> vs;
    std::vector<float> densities;
    std::vector<float> pressures;
    std::vector<vec3> as;
    SpheresRenderer renderer;
    float W(float r, float h);
    float dW(float r, float h);
    void applyBoundaries();
};