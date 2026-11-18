#include "SphereCube.h"
#include <algorithm>
#include <cstddef>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <vector>
#define cimg_display 0
#include "CImg.h"

// Baseado na técnica descrita no video www.youtube.com/watch?v=sLqXFF8mlEU

SphereCube::SphereCube(int n, GLfloat *color, double x, double y, double z, double radius, double orbitalPeriod) : n(n), color(color), planetData{glm::vec3{x, y, z}, radius, orbitalPeriod}
{
    generateSphere(n);
}

SphereCube::SphereCube(int n, GLfloat *color, double x, double y, double z, double radius, double orbitalPeriod, const char *mapPath) : n(n), color(color), planetData{glm::vec3{x, y, z}, radius, orbitalPeriod}
{
    std::string heightMapPath = mapPath;
    int dot = heightMapPath.find_last_of(".");
    std::string basePath = heightMapPath.substr(0, dot);

    std::string normalMapPath = basePath + "_"; 
    normalMapPath += "NormalMap";
    normalMapPath += ".png";
    std::cout << normalMapPath << std::endl;

    loadHeightMap(mapPath, imgData);
    generateSphere(n);
    loadNormalMap(normalMapPath.c_str(), imgData);

}

void SphereCube::generateSphere(int n)
{
    vertices = new double[6 * n * n * 3];
    colors   = new double[6 * n * n * 3];
    normals  = new double[6 * n * n * 3];

    const int stripCount      = 6 * (n - 1);
    const int indicesPerStrip = 2 * n;
    const int restartCount    = stripCount - 1;
    indicesCount = stripCount * indicesPerStrip + restartCount;
    indices = new GLuint[indicesCount];
    int indexOffset = 0;

    for (int face = 0; face < 6; face++)
    {
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                int index = (face * n * n + i * n + j) * 3;
                double u = i / (double)(n - 1) * 2.0 - 1.0;
                double v = j / (double)(n - 1) * 2.0 - 1.0;
                double x, y, z;

                switch (face)
                {
                    case 0: x = u;    y = v;    z = 1.0;  break; // frente
                    case 1: x = u;    y = v;    z = -1.0; break; // trás
                    case 2: x = -1.0; y = u;    z = v;    break; // esquerda
                    case 3: x = 1.0;  y = u;    z = v;    break; // direita
                    case 4: x = u;    y = 1.0;  z = v;    break; // cima
                    case 5: x = u;    y = -1.0; z = v;    break; // baixo
                }

                double x2 = x * x;
                double y2 = y * y;
                double z2 = z * z;
                double x_sphere = x * sqrt(1.0 - (y2 / 2.0) - (z2 / 2.0) + (y2 * z2 / 3.0));
                double y_sphere = y * sqrt(1.0 - (z2 / 2.0) - (x2 / 2.0) + (z2 * x2 / 3.0));
                double z_sphere = z * sqrt(1.0 - (x2 / 2.0) - (y2 / 2.0) + (x2 * y2 / 3.0));

                if (imgData.data != nullptr)
                {
                    glm::vec2 p = sphericalToEquirectangular(x_sphere, y_sphere, z_sphere);
                    float d = sampleHeightMap(p.x, p.y);

                    glm::vec3 p2 = glm::vec3{x_sphere, y_sphere, z_sphere} * (1.0f + d * heightmapScale);
                    vertices[index]     = p2.x;
                    vertices[index + 1] = p2.y;
                    vertices[index + 2] = p2.z;

                    if (std::abs(d) < 0.0005)
                    {
                        colors[index]     = 0.0;
                        colors[index + 1] = 0.0;
                        colors[index + 2] = 1.0;
                    }
                    else
                    {
                        colors[index]     = 0.0;
                        colors[index + 1] = 1.0;
                        colors[index + 2] = 0.0;
                    }
                }
                else
                {
                    vertices[index]     = x_sphere;
                    vertices[index + 1] = y_sphere;
                    vertices[index + 2] = z_sphere;

                    colors[index]     = color[0];
                    colors[index + 1] = color[1];
                    colors[index + 2] = color[2];
                }
            }
        }

        const int vertexOffset = face * n * n;

        for (int i = 0; i < n - 1; i++)
        {
            for (int j = 0; j < n; j++)
            {
                const int a = vertexOffset + i * n + j;
                const int b = vertexOffset + (i + 1) * n + j;
                if (face == 1 || face == 2 || face == 4)
                {
                    indices[indexOffset++] = b;
                    indices[indexOffset++] = a;
                }
                else
                {
                    indices[indexOffset++] = a;
                    indices[indexOffset++] = b;
                }
            }

            if (face != 5 || i != n - 2)
            {
                indices[indexOffset++] = 0xFFFFFFFF;
            }
        }
    }
}

void SphereCube::loadHeightMap(const char *path, SphereCube::HeightMapData &data)
{
    data.data =  stbi_load(path, &data.width, &data.height, &data.channels, 1);
    if (!data.data)
    {
        std::cerr << "Falha ao carregar imagem: "
                << stbi_failure_reason() << '\n';
        return;
    }
    smoothHeightMap(data.data, data.width, data.height, 2.5f);
}

// Função para o carregamento do normalMap 
// Bastante ajuda do Gemini para fazer essa função se comportar da forma correta
void SphereCube::loadNormalMap(const char *path, HeightMapData &data)  
{
    if (imgData.data == nullptr)
    {
        std::cerr << "HeightMap não carregado!" << std::endl;
        return;
    }

    int nmWidth = 0, nmHeight = 0, nmChannels = 0;
    data.normalMap = stbi_load(path, &nmWidth, &nmHeight, &nmChannels, 3);

    // Geramos normalmap se ainda não existir
    if (!data.normalMap)
    {
        std::cout << "NormalMap ainda não gerado. Gerando..." << std::endl;
        generateNormalMap(path);
        data.normalMap = stbi_load(path, &nmWidth, &nmHeight, &nmChannels, 3);
    }

    if (!data.normalMap)
    {
        std::cerr << "Falha ao carregar NormalMap: " << path << std::endl;
        return;
    }

    // Preenchemos o array normals com os valores de normal lidos da imagem
    const int totalComponents = 6 * n * n * 3;
    for (int i = 0; i < totalComponents; i += 3)
    {
        // Normalizamos o vértice para que asin(y) não passe de -1 a 1
        glm::vec3 dir = glm::normalize(glm::vec3(vertices[i], vertices[i + 1], vertices[i + 2]));

        glm::vec2 p = sphericalToEquirectangular(dir.x, dir.y, dir.z); // Coordenadas UV para amostrar o mapa normal
        glm::vec3 tangentNormal = sampleNormalMap(p.x, p.y); // Vetor normal à superfície no espaço tangente

        // glNormalPointer espera valores no espaço XYZ, mas o normalmap é gerado no espaço tangente. 
        // Se y for 1 ou -1, o produto vetorial torna-se 0. Ao normalizar esse vetor, teriamos divisão por zero.
        // Para evitar esse problema, trocamos o eixo up de y para x. 
        glm::vec3 upAxis = (std::abs(dir.y) < 0.999f)
            ? glm::vec3(0.0f, 1.0f, 0.0f)
            : glm::vec3(1.0f, 0.0f, 0.0f);

        // Calculamos a base do espaço tangente no ponto p. O vetor normal N é o dir. Calculamos o vetor tangente T e bitangente B:
        glm::vec3 T = glm::normalize(glm::cross(upAxis, dir));
        glm::vec3 B = glm::cross(T, dir);

        // Convertemos a normal do espaço tangente para o espaço 3D do objeto
        // Esse passo é uma transformação de coordenadas T, B, N para X, Y, Z
        glm::vec3 objNormal = glm::normalize(
            T   * tangentNormal.x +
            B   * tangentNormal.y +
            dir * tangentNormal.z
        );

        // Escrevemos as normais no array normals
        normals[i]     = objNormal.x;
        normals[i + 1] = objNormal.y;
        normals[i + 2] = objNormal.z;
    }
}

void SphereCube::generateNormalMap(const char *path)
{
    if (imgData.data == nullptr)
    {
        std::cout << "HeightMap não carregado!" << std::endl;
        return;
    }

    int width  = imgData.width;
    int height = imgData.height;
    std::vector<unsigned char> normalMap(width * height * 3);

    // helper para obter o valor do heightmap em x e y. Usamos aritmética modular para fazer os pontos "darem a volta" no mapa.
    auto heightAt = [&](int x, int y) {
        x = (x + width) % width;
        y = std::clamp(y, 0, height - 1);
        return imgData.data[y * width + x] / 255.0f;
    };

    // for paralelo para acelerar a geração do normalmap
    #pragma omp parallel for schedule(static)
    for (int j = 0; j < height; j++)
    {
        // Em coordenadas equirectangulares, o eixo y vai de -PI/2 a PI/2.
        // ajustamos o j nesse intervalo
        float v      = (static_cast<float>(j) / static_cast<float>(height - 1) * 2.0f) - 1.0f;
        float phi    = v * M_PI_2;
        // As distâncias em x sofrem uma escala de 1/cos(phi) com o eixo y. Precisamos escalar a derivada parcial 
        // dFdx por esse fator para corrigir esse efeito.
        float cosPhi = std::max(std::cos(phi), 1e-4f);

        for (int i = 0; i < width; i++)
        {
            float hE = heightAt(i + 1, j);
            float hW = heightAt(i - 1, j);
            float hN = heightAt(i, j + 1);
            float hS = heightAt(i, j - 1);
            float p  = heightAt(i, j);

            // Diferenças finitas avaliadas nos pontos N, S, E e W
            float dFdx = (hE - hW) / 2.0f;
            float dFdy = (hN - hS) / 2.0f;
            dFdx /= cosPhi;

            // Fator de escala devido ao tamanho do pixel no mapa
            float str = heightmapScale * height / (M_PI * (1.0f + p * heightmapScale));
            dFdx *= str;
            dFdy *= str;

            // O vetor normal é dado por <-dFdx, -dFdy, 1.0>. 
            glm::vec3 n = glm::normalize(glm::vec3(-dFdx, -dFdy, 1.0f));

            // Convertemos as componentes para 0 - 255 para serem salvo como PNG
            int index = (i + width * j) * 3;
            normalMap[index]     = static_cast<unsigned char>(std::clamp(std::round((n.x * 0.5f + 0.5f) * 255.0f), 0.0f, 255.0f));
            normalMap[index + 1] = static_cast<unsigned char>(std::clamp(std::round((n.y * 0.5f + 0.5f) * 255.0f), 0.0f, 255.0f));
            normalMap[index + 2] = static_cast<unsigned char>(std::clamp(std::round((n.z * 0.5f + 0.5f) * 255.0f), 0.0f, 255.0f));
        }
    }

    int success = stbi_write_png(path, width, height, 3, normalMap.data(), width * 3);
    if (!success)
        std::cerr << "Failed to write normal map: " << path << '\n';
}

void SphereCube::smoothHeightMap(unsigned char *data, int width, int height, float sigma)
{
    cimg_library::CImg<unsigned char> img(data, width, height, 1, 1, true);
    img.blur(sigma);
}

double SphereCube::localHeight(const glm::vec3& direction) const
{
    if (imgData.data == nullptr)
        return 0;
    glm::vec3 v_n = glm::normalize(direction);
    glm::vec2 p = sphericalToEquirectangular(v_n.x, v_n.y, v_n.z);
    return planetData.r * heightmapScale * sampleHeightMap(p.x, p.y);
}

glm::vec2 SphereCube::sphericalToEquirectangular(double x, double y, double z) const
{
    double u = 0.5 - atan2(z, x) / (2.0 * M_PI);
    double v = 0.5 - asin(std::clamp(y, -1.0, 1.0)) / M_PI;
    return glm::vec2{u, v};
}

double SphereCube::sampleHeightMap(double u, double v) const
{
    double fx = std::clamp(u, 0.0, 1.0) * (imgData.width - 1.0);
    double fy = std::clamp(v, 0.0, 1.0) * (imgData.height - 1.0);

    int x0 = static_cast<int>(std::floor(fx));
    int y0 = static_cast<int>(std::floor(fy));
    int x1 = (x0 + 1) % imgData.width;
    int y1 = std::min(y0 + 1, imgData.height - 1);

    double tx = fx - x0;
    double ty = fy - y0;

    auto at = [&](int x, int y){
        return static_cast<double>(imgData.data[y * imgData.width + x]) / 255.0;
    };

    double top = at(x0, y0) * (1.0 - tx) + at(x1, y0) * tx;
    double bottom = at(x0, y1) * (1.0 - tx) + at(x1, y1) * tx;
    return top * (1.0 - ty) + bottom * ty;
}

glm::vec3 SphereCube::sampleNormalMap(double u, double v) const
{
    double fx = std::clamp(u, 0.0, 1.0) * (imgData.width - 1.0);
    double fy = std::clamp(v, 0.0, 1.0) * (imgData.height - 1.0);

    int x0 = static_cast<int>(std::floor(fx));
    int y0 = static_cast<int>(std::floor(fy));
    int x1 = (x0 + 1) % imgData.width;
    int y1 = std::min(y0 + 1, imgData.height - 1);

    double tx = fx - x0;
    double ty = fy - y0;

    auto at = [&](int x, int y){
        int idx = (y * imgData.width + x) * 3;
        return glm::vec3(
            (imgData.normalMap[idx] / 255.0) * 2.0f - 1.0f,
            (imgData.normalMap[idx + 1] / 255.0) * 2.0f - 1.0f,
            (imgData.normalMap[idx + 2] / 255.0) * 2.0f - 1.0f
        );
    };

    glm::vec3 top = glm::mix(at(x0, y0), at(x1, y0), tx);
    glm::vec3 bottom = glm::mix(at(x0, y1), at(x1, y1), tx);
    return glm::normalize(glm::mix(top, bottom, ty));
}
