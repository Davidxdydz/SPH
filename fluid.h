#pragma once
#include <vector>

#include <glm/glm.hpp>

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
    std::vector<glm::vec3> colors;
    std::vector<glm::vec3> vs;
    std::vector<float> densities;
    std::vector<float> pressures;
    std::vector<glm::vec3> as;
    SpheresRenderer renderer;
    float W(float r, float h);
    float dW(float r, float h);
    void applyBoundaries();
};

class Grid
{
public:
    float size;
    int tableSize;
    Grid(float size, int tableSize, std::vector<Transform> &transforms);
    void update();
    /*
    Get indices of all particles in the same cell and the 26 surrounding cells
    */
    std::vector<int> getNeighbors(glm::vec3 pos);
    std::vector<int> getCell(glm::vec3 pos);

private:
    std::vector<Transform> &transforms;
    std::vector<int> hashs;
    std::vector<int> sortedHashIndices;
    std::vector<int> startIndices;
    int hash(glm::vec3 pos);
    glm::ivec3 cellIds(glm::vec3 pos);
};