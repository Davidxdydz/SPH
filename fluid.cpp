#include "fluid.h"
#include <iostream>
#include <algorithm>
#include <glm/gtc/random.hpp>
using namespace glm;

Fluid::Fluid(GLuint instancingShaderID) : renderer(instancingShaderID, transforms, colors, 2)
{
    int nx = 20;
    int ny = 20;
    int nz = 1;
    int n = nx * ny * nz;
    this->dt = 0.05f;
    this->simulatedVolume = 1.0f;
    this->gravity = 0.001f;
    this->restDensity = 200.0f;
    this->volume = simulatedVolume / nx / ny / nz;
    this->h = 1.0f / pow(restDensity, 1 / 2) / 8;
    this->displayRaius = 0.02f;
    this->effectiveRadius = pow(volume * 3 / 4 / 3.1415926f, 1.0f / 3);
    this->targetDensity = restDensity;
    this->stiffness = 1.5f;
    this->damping = 0.1f;
    this->m = 1.0f;

    for (int x = 0; x < nx; x++)
    {
        for (int y = 0; y < ny; y++)
        {
            for (int z = 0; z < nz; z++)
            {
                float x0 = (float)x / nx;
                float y0 = (float)y / ny;
                float z0 = (float)z / nz;
                transforms.push_back(Transform(vec3(x0, y0, z0) - vec3(0.5), vec3(0), vec3(displayRaius)));
                vs.push_back(vec3(0));
                colors.push_back(vec3(x0, y0, z0));
            }
        }
    }
}

void Fluid::step()
{
    densities.assign(transforms.size(), 0);
    pressures.assign(transforms.size(), 0);
    as.assign(transforms.size(), vec3(0));

    for (int i = 0; i < transforms.size(); i++)
    {
        for (int j = i; j < transforms.size(); j++)
        {
            vec3 delta = transforms[i].position - transforms[j].position;
            float r = length(delta);
            // rho[kg/m^3] = m[kg] * W[m^-3]
            float d = m * W(r, h);
            densities[i] += d;
            densities[j] += d;
        }
    }
    float gamma = 1.3f;

    for (int i = 0; i < transforms.size(); i++)
    {
        // p [Nm^-2 = kgs^-2 m^-1] = k[m^2s^-2] * (rho[kg/m^3] - rho0[kg/m^3])
        pressures[i] = stiffness * (densities[i] - restDensity);
        // pressures[i] = stiffness * (pow(densities[i] / restDensity, 1.3) - 1);
    }

    for (int i = 0; i < transforms.size(); i++)
    {
        vec3 a = vec3(0);
        for (int j = i + 1; j < transforms.size(); j++)
        {
            vec3 dist = transforms[i].position - transforms[j].position;
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
    for (int i = 0; i < transforms.size(); i++)
    {
        vs[i].z = 0;
        vs[i] += as[i] * dt;
        transforms[i].position += vs[i] * dt;
        transforms[i].position.z = -0.5f;
    }
    float total_energy = 0.0f;
    for (int i = 0; i < transforms.size(); i++)
    {
        total_energy += 0.5f * m * dot(vs[i], vs[i]);
        total_energy += m * gravity * (transforms[i].position.y + 1);
    }
    // std::cout << "total energy: " << total_energy << std::endl;
    applyBoundaries();
}

float Fluid::W(float r, float h)
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

float Fluid::dW(float r, float h)
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

void Fluid::applyBoundaries()
{
    for (int i = 0; i < transforms.size(); i++)
    {
        if (transforms[i].position.y < -1)
        {
            vs[i].y *= -(1 - damping);
            transforms[i].position.y = -1;
        }
        if (transforms[i].position.y > 1)
        {
            vs[i].y *= -(1 - damping);
            transforms[i].position.y = 1;
        }
        if (transforms[i].position.x < -1)
        {
            transforms[i].position.x = -1;
            vs[i].x *= -(1 - damping);
        }
        if (transforms[i].position.x > 1)
        {
            transforms[i].position.x = 1;
            vs[i].x *= -(1 - damping);
        }
        if (transforms[i].position.z < -1)
        {
            transforms[i].position.z = -1;
            vs[i].z *= -(1 - damping);
        }
        if (transforms[i].position.z > 1)
        {
            transforms[i].position.z = 1;
            vs[i].z *= -(1 - damping);
        }
    }
}

void Fluid::draw()
{
    for (int i = 0; i < densities.size(); i++)
    {
        float normalized = densities[i] / targetDensity;
        colors[i] = vec3(normalized, 1 - normalized, 0);
    }
    renderer.draw();
}

Grid::Grid(float size, int tableSize, std::vector<Transform> &transforms) : size(size), transforms(transforms), tableSize(tableSize)
{
    hashs.assign(transforms.size(), 0);
    sortedHashIndices.assign(transforms.size(), 0);
    startIndices.assign(tableSize, -1);
    update();
}

void Grid::update()
{
    for (int i = 0; i < transforms.size(); i++)
    {
        hashs[i] = hash(transforms[i].position);
        sortedHashIndices[i] = i;
    }
    std::sort(sortedHashIndices.begin(), sortedHashIndices.end(), [&](int a, int b)
              { return hashs[a] < hashs[b]; });
    int lastHash = -1;
    for (int i = 0; i < transforms.size(); i++)
    {
        if (hashs[sortedHashIndices[i]] != lastHash)
        {
            startIndices[hashs[sortedHashIndices[i]]] = i;
            lastHash = hashs[sortedHashIndices[i]];
        }
    }
}

ivec3 Grid::cellIds(vec3 pos)
{
    return ivec3(pos / size);
}

int Grid::hash(vec3 pos)
{
    ivec3 ids = cellIds(pos);
    return abs((ids.x * 92837111) ^ (ids.y * 689287499) ^ (ids.z * 283923481)) % tableSize;
}

std::vector<int> Grid::getCell(vec3 pos)
{
    int index = hash(pos);
    int start = startIndices[index];
    std::vector<int> result;
    if (start == -1)
        return result;
    for (int i = start; i < sortedHashIndices.size(); i++)
    {
        if (hashs[sortedHashIndices[i]] != index)
            break;
        result.push_back(sortedHashIndices[i]);
    }
    return result;
}

std::vector<int> Grid::getNeighbors(vec3 pos)
{
    std::vector<int> result;
    for (int x = -1; x < 2; x++)
    {
        for (int y = -1; y < 2; y++)
        {
            for (int z = -1; z < 2; z++)
            {
                int cellHash = hash(pos + vec3(x, y, z) * size);
                int start = startIndices[cellHash];
                if (start == -1)
                    return result;
                for (int i = start; i < sortedHashIndices.size(); i++)
                {
                    if (hashs[sortedHashIndices[i]] != cellHash)
                        break;
                    result.push_back(sortedHashIndices[i]);
                }
            }
        }
    }
    return result;
}