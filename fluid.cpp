#include "fluid.h"
#include <iostream>
#include <algorithm>
#include <glm/gtc/random.hpp>
using namespace glm;

Fluid::Fluid(GLuint instancingShaderID) : renderer(instancingShaderID, transforms, colors, 2), grid(-1, 1000, transforms)
{
    int nx = 10;
    int ny = 10;
    int nz = 10;
    int n = nx * ny * nz;
    this->dt = 0.02f;
    this->simulatedVolume = 1.0f;
    this->gravity = 0.05f;
    this->restDensity = 600;
    this->h = 1.0f / 20;
    this->displayRaius = 0.05f;
    this->stiffness = 5.5f;
    this->damping = 0.3f;
    this->m = 1.0f;
    this->grid.size = 2 * h;
    this->mu = 0.0f;

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
    grid.update();

    for (int i = 0; i < transforms.size(); i++)
    {
        std::vector<int> neighbors = grid.getCell(transforms[i].position);
        for (int j = 0; j < neighbors.size(); j++)
        {
            int neighborIndex = neighbors[j];
            if (neighborIndex < i)
                continue;
            vec3 delta = transforms[i].position - transforms[neighborIndex].position;
            float r = length(delta);
            // rho[kg/m^3] = m[kg] * W[m^-3]
            float d = m * W(r, h);
            densities[i] += d;
            if (neighborIndex != i)
                densities[neighborIndex] += d;
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
        std::vector<int> neighbors = grid.getNeighbors(transforms[i].position);
        for (int j = 0; j < neighbors.size(); j++)
        {
            int neighborIndex = neighbors[j];
            if (neighborIndex <= i)
                continue;

            vec3 dist = transforms[i].position - transforms[neighborIndex].position;
            float r = length(dist);
            vec3 direction = normalize(dist);
            if (r < 1e-5)
                direction = sphericalRand(1.0f);

            // dP/dx[Nm^-3] = kgm^-1s^-2 * kg^-2m^6 * m^-4
            // = kg^-1 s^-2 m
            vec3 pressureGradient = (pressures[i] / densities[i] / densities[i] + pressures[neighborIndex] / densities[neighborIndex] / densities[neighborIndex]) * dW(r, h) * direction;
            as[i] -= pressureGradient;
            as[neighborIndex] += pressureGradient;
            // viscosity
            vec3 dv = 2 * mu * m / (densities[i] + densities[neighborIndex]) * (vs[neighborIndex] - vs[i]) * dW(r, h);
            vs[i] += dv * dt;
            vs[neighborIndex] -= dv * dt;
        }
        as[i] /= densities[i];
        as[i].y -= gravity;
    }

    // leapfrog integration
    for (int i = 0; i < transforms.size(); i++)
    {
        // vs[i].z = 0;
        vs[i] += as[i] * dt;
        transforms[i].position += vs[i] * dt;
        // transforms[i].position.z = -0.5f;
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
    h *= 2; // to get the radius to be h
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
    h *= 2; // to get the radius to be h
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
        // float normalized = densities[i] / restDensity /3;
        float normalized = length(vs[i]) * 5;
        colors[i] = vec3(normalized, 1 - normalized, 0);
    }
    renderer.draw();
}

Grid::Grid(float size, int tableSize, std::vector<Transform> &transforms) : size(size), transforms(transforms), tableSize(tableSize)
{
}

void Grid::update()
{
    // TODO move these to constructor and include their sizes in the constructor. atm not possible as transforms is unititialized in constructor
    hashs.assign(transforms.size(), 0);
    sortedHashIndices.assign(transforms.size(), 0);
    startIndices.assign(tableSize, -1);
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
            lastHash = hashs[sortedHashIndices[i]];
            startIndices[lastHash] = i;
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
    // probably incredibly slow to create a vector for every cell for every step
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
                    continue;
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