//
// Created by Kristoffer Borgstrøm on 23/03/2023.
//
#pragma once

#include <glm/glm.hpp>

#include <ituGL/texture/Texture3DObject.h>

class Cloud {

public:
    Cloud(unsigned int highResWidth = 128, unsigned int lowResWidth = 32);

    void InitializeCloud();
    void PopulateWorleyCoords(std::shared_ptr<std::vector<glm::vec3>> cells, int width);

    std::shared_ptr<Texture3DObject> GetLowResCloud();
    std::shared_ptr<Texture3DObject> GetHighResCloud();

private:

    float ClosestWorleyPoint(std::shared_ptr<std::vector<glm::vec3>> cells, glm::vec3 samplePoint, float width);

    void InitializeWorley();
    void InitializePerlinWorley();

    unsigned int m_highResWidth;
    unsigned int m_lowResWidth;


    std::shared_ptr<std::vector<glm::vec3>> m_worleyCellsLowFreq = std::make_shared<std::vector<glm::vec3>>();
    std::shared_ptr<std::vector<glm::vec3>> m_worleyCellsMidFreq = std::make_shared<std::vector<glm::vec3>>();
    std::shared_ptr<std::vector<glm::vec3>> m_worleyCellsHighFreq = std::make_shared<std::vector<glm::vec3>>();
    int m_cellLowFreqWidth = 4;
    int m_cellMidFreqWidth = 12;
    int m_cellHighFreqWidth = 32;


    std::shared_ptr<Texture3DObject> m_perlinWorley;
    std::shared_ptr<Texture3DObject> m_worley;
};


