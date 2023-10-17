#include "fluid.h"
#include <iostream>
#include <glm/gtc/random.hpp>

float W(float r, float h)
{
    // https://de.wikipedia.org/wiki/Smoothed_Particle_Hydrodynamics#Kern
    // 0<=q <=1 : (4-6q^2 + 3q^3)
    // 1<q <=2 : (2-q)^3
    // q > 2 : 0
    // integral of w from -inf to inf has to be 1, correct with factor sigma = 2/9
    float h3 = h * h * h;
    float q = r / h;

    float result = 0;
    if (q <= 1)
    {
        result = (4 - 6 * q * q + 3 * q * q * q);
    }
    else
    {
        if (q <= 2)
        {
            result = (2 - q) * (2 - q) * (2 - q);
        }
    }
    return result * 1 / 6 / h3;
}

float dW(float r, float h)
{
    // 0<= q <=1 : -12q + 9q^2
    // h< q <=2 : -3(2-q)^2
    // q > 2 : 0
    // always
    float h4 = h * h * h * h;
    float q = r / h;

    float result = 0;
    if (q <= 1)
    {
        result = (-12 * q + 9 * q * q);
    }
    else
    {
        if (q <= 2)
        {
            result = (-3) * (2 - q) * (2 - q);
        }
    }
    return result * 1 / 6 / h4;
}

void sphStep(std::vector<float> &densities, std::vector<Transform> &spheres, std::vector<vec3> &vs, float dt, float h, float stiffness, float restDensity, float gravity, float m, float volume)
{
    densities.assign(spheres.size(), 0);
    std::vector<float> pressures = std::vector<float>(spheres.size(), 0);
    std::vector<vec3> as = std::vector<vec3>(spheres.size(), vec3(0));

    for (int i = 0; i < spheres.size(); i++)
    {
        for (int j = i; j < spheres.size(); j++)
        {
            vec3 delta = spheres[i].position - spheres[j].position;
            float r = length(delta);
            // rho[kg/m^3] = m[kg] * W[m^-3]
            float d = m * W(r, h);
            densities[i] += d;
            densities[j] += d;
        }
    }
    float gamma = 1.3f;

    for (int i = 0; i < spheres.size(); i++)
    {
        // p [Nm^-2 = kgs^-2 m^-1] = k[m^2s^-2] * (rho[kg/m^3] - rho0[kg/m^3])
        pressures[i] = stiffness * (densities[i] - restDensity);
        // pressures[i] = stiffness * (pow(densities[i] / restDensity, 1.3) - 1);
    }

    for (int i = 0; i < spheres.size(); i++)
    {
        vec3 a = vec3(0);
        for (int j = i + 1; j < spheres.size(); j++)
        {
            vec3 dist = spheres[i].position - spheres[j].position;
            float r = length(dist);
            vec3 direction = normalize(dist);
            if (r < 1e-5)
                direction = sphericalRand(1.0f);

            // dP/dx[Nm^-3] = kgm^-1s^-2 * kg^-2m^6 * m^-4
            // = kg^-1 s^-2 m
            vec3 pressureGradient = (pressures[i] / densities[i] / densities[i] + pressures[j] / densities[j] / densities[j]) * dW(r, h) * direction;
            as[i] -= pressureGradient;
            as[j] += pressureGradient;
        }
        as[i] /= densities[i];
        as[i].y -= gravity;
        // as[i].x = 0;
        // as[i].y = -gravity;
        // as[i].z = 0;
    }
    // leapfrog integration
    for (int i = 0; i < spheres.size(); i++)
    {
        vs[i].z = 0;
        vs[i] += as[i] * dt;
        spheres[i].position += vs[i] * dt;
        spheres[i].position.z = -0.5f;
    }
    float total_energy = 0.0f;
    for (int i = 0; i < spheres.size(); i++)
    {
        total_energy += 0.5f * m * dot(vs[i], vs[i]);
        total_energy += m * gravity * (spheres[i].position.y + 1);
    }
    std::cout << "total energy: " << total_energy << std::endl;
}

void applyBoundaries(std::vector<Transform> &spheres, std::vector<vec3> &vs, float damping, float gravity)
{
    for (int i = 0; i < spheres.size(); i++)
    {
        if (spheres[i].position.y < -1)
        {
            float deltaH = -(spheres[i].position.y + 1);
            float deltaE = deltaH * gravity * 1;
            // std::cout << deltaE << std::endl;
            float v = vs[i].y;
            float v2 = v * v;
            float dv = sqrt(v2 + 2 * deltaE / 1) - v2;
            vs[i].y *= -(1 - damping);
            // vs[i].y -= dv;
            // std::cout << dv << std::endl;
            spheres[i].position.y = -1;
        }
        if (spheres[i].position.y > 1)
        {
            float deltaE = (spheres[i].position.y - 1) * gravity;
            float v = vs[i].y;
            float v2 = v * v;
            float dv = sqrt(v2 + 2 * deltaE / 1) - v2;
            vs[i].y *= -(1 - damping);
            // vs[i].y -= dv;
            spheres[i].position.y = 1;
        }
        if (spheres[i].position.x < -1)
        {
            spheres[i].position.x = -1;
            vs[i].x *= -(1 - damping);
        }
        if (spheres[i].position.x > 1)
        {
            spheres[i].position.x = 1;
            vs[i].x *= -(1 - damping);
        }
        if (spheres[i].position.z < -1)
        {
            spheres[i].position.z = -1;
            vs[i].z *= -(1 - damping);
        }
        if (spheres[i].position.z > 1)
        {
            spheres[i].position.z = 1;
            vs[i].z *= -(1 - damping);
        }
    }
}