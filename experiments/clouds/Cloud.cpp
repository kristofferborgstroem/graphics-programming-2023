//
// Created by Kristoffer Borgstrøm on 23/03/2023.
//

#include <vector>
#include <random>
#include <limits>
#include <iostream>

#define STB_PERLIN_IMPLEMENTATION
#include <stb_perlin.h>

#include "Cloud.hpp"

Cloud::Cloud(unsigned int highResWidth, unsigned int lowResWidth) :
    m_highResWidth(highResWidth),
    m_lowResWidth(lowResWidth)
{
}


float Cloud::ClosestWorleyPoint(std::shared_ptr<std::vector<glm::vec3>> cells, glm::vec3 samplePoint, float width) {
    float min = std::numeric_limits<float>::max();

    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            for (int z = -1; z <= 1; ++z) {
                auto offset = glm::vec3(x, y, z);

                auto idx = glm::floor(samplePoint) + offset;

                auto mod = glm::mod(idx, glm::vec3(width));
                // TODO: Don't compute length for every pair
                int index = mod.x * width * width + mod.y * width + mod.z;
                auto neighbor = cells->at(index);
                float dist = glm::length(samplePoint - (idx + neighbor));
                min = glm::min(min, abs(dist));
            }
        }
    }


    // Max range is equal to the square root of 2
    // 0.707 is 1 over max range
    // Maps distance into [0; 1]
    return 1.0f - min;
    return glm::clamp(1.0f - min, 0.0f, 1.0f);
    float maxRange = 1.42f;
    return (maxRange - min) * 0.707f;
}

void Cloud::InitializeCloud() {
    PopulateWorleyCoords(m_worleyCellsLowFreq, m_cellLowFreqWidth);
    PopulateWorleyCoords(m_worleyCellsMidFreq, m_cellMidFreqWidth);
    PopulateWorleyCoords(m_worleyCellsHighFreq, m_cellHighFreqWidth);

    InitializePerlinWorley();
    InitializeWorley();
}

float PerlinFBM(float frequency, int x, int y, int z) {
    float t = stb_perlin_fbm_noise3(x * frequency, y * frequency, z * frequency, 1.9f, 0.5f, 8);
    return t;
}

void Cloud::InitializePerlinWorley() {

    m_perlinWorley = std::make_shared<Texture3DObject>();

    std::vector<float> texels;

    float ratioRed   = (float) m_cellLowFreqWidth / (float) m_highResWidth;
    float ratioGreen = (float) m_cellMidFreqWidth / (float) m_highResWidth;
    float ratioBlue  = (float) m_cellHighFreqWidth / (float) m_highResWidth;

    for (int x = 0; x < m_highResWidth; ++x) {
        for (int y = 0; y < m_highResWidth; ++y) {
            for (int z = 0; z < m_highResWidth; ++z) {
                glm::vec3 samplePoint = glm::vec3((float) x, (float) y, (float) z) * glm::vec3(ratioRed);
                float red = ClosestWorleyPoint(m_worleyCellsLowFreq, samplePoint, (float) m_cellLowFreqWidth);

                samplePoint = glm::vec3((float) x, (float) y, (float) z) * glm::vec3(ratioGreen);
                float green = ClosestWorleyPoint(m_worleyCellsMidFreq, samplePoint, (float) m_cellMidFreqWidth);

                samplePoint = glm::vec3((float) x, (float) y, (float) z) * glm::vec3(ratioBlue);
                float blue = ClosestWorleyPoint(m_worleyCellsHighFreq, samplePoint, (float) m_cellHighFreqWidth);


                float frequency = 0.5f;
                //float perlin = stb_perlin_fbm_noise3(x * frequency, y * frequency, z * frequency, 1.9f, 0.5f, 4);
                //float perlinSAT = ((float) perlin + 1.0f) * 0.5f;

                texels.push_back(glm::mix(red, PerlinFBM(0.3f, x, y, z), 0.2f));
                texels.push_back(glm::mix(green, PerlinFBM(0.8f, x, y, z), 0.2f));
                texels.push_back(blue);
            }
        }
    }

    m_perlinWorley->Bind();
    m_perlinWorley->SetImage<float>(0, m_highResWidth, m_highResWidth, m_highResWidth, TextureObject::FormatRGB, TextureObject::InternalFormatRGB16F, texels);
    m_perlinWorley->GenerateMipmap();

    m_perlinWorley->SetParameter(Texture3DObject::ParameterEnum::MinFilter, GL_LINEAR);
    m_perlinWorley->SetParameter(Texture3DObject::ParameterEnum::MagFilter, GL_LINEAR);

    m_perlinWorley->SetParameter(Texture3DObject::ParameterEnum::WrapT, GL_REPEAT);
    m_perlinWorley->SetParameter(Texture3DObject::ParameterEnum::WrapS, GL_REPEAT);
    m_perlinWorley->SetParameter(Texture3DObject::ParameterEnum::WrapR, GL_REPEAT);

    texels.clear();

}

void Cloud::InitializeWorley() {
    m_worley = std::make_shared<Texture3DObject>();

    std::vector<float> texels;

    float ratioRed   = (float) m_cellLowFreqWidth / (float) m_lowResWidth;
    float ratioGreen = (float) m_cellMidFreqWidth / (float) m_lowResWidth;
    float ratioBlue  = (float) m_cellHighFreqWidth / (float) m_lowResWidth;

    for (int x = 0; x < m_lowResWidth; ++x) {
        for (int y = 0; y < m_lowResWidth; ++y) {
            for (int z = 0; z < m_lowResWidth; ++z) {
                glm::vec3 samplePoint = glm::vec3((float) x, (float) y, (float) z) * glm::vec3(ratioRed);
                float red = ClosestWorleyPoint(m_worleyCellsLowFreq, samplePoint, (float) m_cellLowFreqWidth);
                samplePoint = glm::vec3((float) x, (float) y, (float) z) * glm::vec3(ratioGreen);
                float green = ClosestWorleyPoint(m_worleyCellsMidFreq, samplePoint, (float) m_cellMidFreqWidth);
                samplePoint = glm::vec3((float) x, (float) y, (float) z) * glm::vec3(ratioBlue);
                float blue = ClosestWorleyPoint(m_worleyCellsHighFreq, samplePoint, (float) m_cellHighFreqWidth);
                texels.push_back(red);
                texels.push_back(green);
                texels.push_back(blue);
            }
        }
    }

    m_worley->Bind();
    m_worley->SetImage<float>(0, m_lowResWidth, m_lowResWidth, m_lowResWidth, TextureObject::FormatRGB, TextureObject::InternalFormatRGB16F, texels);
    m_worley->GenerateMipmap();

    m_worley->SetParameter(Texture3DObject::ParameterEnum::MinFilter, GL_LINEAR);
    m_worley->SetParameter(Texture3DObject::ParameterEnum::MagFilter, GL_LINEAR);

    m_worley->SetParameter(Texture3DObject::ParameterEnum::WrapT, GL_REPEAT);
    m_worley->SetParameter(Texture3DObject::ParameterEnum::WrapS, GL_REPEAT);
    m_worley->SetParameter(Texture3DObject::ParameterEnum::WrapR, GL_REPEAT);

    texels.clear();
}

std::shared_ptr<Texture3DObject> Cloud::GetHighResCloud() {
    return m_perlinWorley;
}

std::shared_ptr<Texture3DObject> Cloud::GetLowResCloud() {
    return m_worley;
}

void Cloud::PopulateWorleyCoords(std::shared_ptr<std::vector<glm::vec3>> cells, int width)
{
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < width; ++y) {
            for (int z = 0; z < width; ++z) {
                float a = rand() / double(RAND_MAX);
                float b = rand() / double(RAND_MAX);
                float c = rand() / double(RAND_MAX);
                cells->emplace_back(glm::vec3(a, b, c));
            }
        }
    }
}