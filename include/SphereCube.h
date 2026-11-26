
#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <cmath>
#include <cstdint>
#include <string>
#include "Shader.h"
#include <vector>

class SphereCube
{
    private:
        struct PlanetData{glm::vec3 coords{0.0,0.0,0.0}; double r = 1.0; double orbitalPeriod = 0.0;};
        struct HeightMapData{
            std::vector<uint16_t> data;
            std::vector<uint16_t> normalMap;
            int width = 0;
            int height = 0;
            int normalMapWidth = 0;
            int normalMapHeight = 0;
            int channels = 0;
        };

        GLuint* indices = nullptr;
        GLfloat *color = nullptr;
        HeightMapData imgData;
        double* vertices = nullptr;
        double* colors = nullptr;
        double* normals = nullptr;
        int indicesCount = 0;
        int verticesCount = 0;
        int n = 0;
        float heightmapScale = 0.05f; 
        Shader *shader;
        
        void generateSphere(int n);
        glm::vec2 sphericalToEquirectangular(double x, double y, double z) const;
        double sampleHeightMap(double u, double v) const;
        glm::vec3 sampleNormalMap(double u, double v) const;
        void loadHeightMap(const char *path, HeightMapData &data);
        void loadNormalMap(const char *path, HeightMapData &data);
        bool generateNormalMap(const char *path);
        void smoothHeightMap(uint16_t *data, int width, int height, float sigma = 2.5f);     

    public:
        SphereCube() = default;
        SphereCube(int n, GLfloat *color, double x, double y, double z, double radius, double orbitalPeriod);
        SphereCube(int n, GLfloat *color, double x, double y, double z, double radius, double orbitalPeriod, const char *mapPath);
        double* getVertices() { return vertices; }
        double* getColors() { return colors; }
        double* getNormals() {return normals;}
        GLuint* getIndices() { return indices; }
        int getVerticesCount() { return verticesCount; }
        int getIndicesCount() { return indicesCount; }
        PlanetData planetData;
        double localHeight(const glm::vec3& direction) const;
        void draw(double time) const;

        void setShaders(Shader *shader);
        
};