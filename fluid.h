#pragma once
#include <vector>
#include "RenderObject.h"

void sphStep(std::vector<float> &densities, std::vector<Transform> &spheres, std::vector<vec3> &vs, float dt, float h, float stiffness = 1.5f, float restDensity = 1.0f, float gravity = 0.0f, float m = 1.0f, float volume = 1.0f);
void applyBoundaries(std::vector<Transform> &spheres, std::vector<vec3> &vs, float damping = 0.01f, float gravitiy = 1.0f);