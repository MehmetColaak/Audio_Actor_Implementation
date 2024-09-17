/* This project uses the following third-party libraries:
 *
 * 1. SFML - Simple and Fast Multimedia Library
 *    Copyright (c) Laurent Gomila
 *    Licensed under the zlib/libpng license
 *    https://www.sfml-dev.org/license.php
 *
 * 2. Steam Audio
 *    Copyright (c) Valve Corporation
 *    Licensed under the BSD 3-Clause "New" or "Revised" License
 *    https://github.com/ValveSoftware/steam-audio/blob/master/LICENSE.md
 *
 * This project itself is released under CC0 1.0 Universal (CC0 1.0) Public Domain Dedication.
 * To the extent possible under law, the author(s) have dedicated all copyright and related 
 * and neighboring rights to this software to the public domain worldwide.
 * This software is distributed without any warranty.
 * You should have received a copy of the CC0 Public Domain Dedication along with this software. 
 * If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.*/

#include <iostream>
#include <cmath>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "steamaudiomanager.h"

const int sW {1920};
const int sH {1080};

const float piVal {3.14159265358979323846};
const float movementSpeed {300.f};
const float radarSpeed {1500.f};

void InputMovement(sf::Vector2f& ballPos, float deltaTime) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) ballPos.y -= movementSpeed * deltaTime;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) ballPos.y += movementSpeed * deltaTime;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) ballPos.x += movementSpeed * deltaTime;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) ballPos.x -= movementSpeed * deltaTime;
}

void UpdateFocusShape(sf::VertexArray& shape, sf::Vector2f center, float radius, float startAngle, float endAngle, sf::Color color)
{
    shape[0].position = center;
    shape[0].color = color;

    for(size_t i = 1; i < shape.getVertexCount(); ++i)
    {
        float angle = startAngle + (endAngle - startAngle) * (i - 1) / (shape.getVertexCount() - 2);
        shape[i].position = center + sf::Vector2f(std::cos(angle) * radius, std::sin(angle) * radius);
        shape[i].color = sf::Color::Black;
    }
}

void UpdateRadarShape(sf::CircleShape& radarCircle, float& radarRadius, float maxRadarRadius, float deltaTime, bool& isRadarExpanding)
{
    if (isRadarExpanding)
    {
        radarRadius += radarSpeed * deltaTime;
        if (radarRadius >= maxRadarRadius)
        {
            isRadarExpanding = false;
            radarRadius = 10.f;
        }
        radarCircle.setRadius(radarRadius);
        radarCircle.setOrigin(radarRadius, radarRadius);
    }
}

int main()
{
    SteamAudioManager audioManager;

    if (!audioManager.Initialize())
    {
        std::cerr << "Failed to initialize Steam Audio Manager" << std::endl;
        return 1;
    }

    audioManager.DebugPrint();

    /*IPLSource source;
    IPLSourceSettings sourceSettings = {};
    sourceSettings.flags = IPL_SIMULATIONFLAGS_DIRECT;
    iplSourceCreate(audioManager.GetContext(), &sourceSettings, &source);*/

    sf::RenderWindow window(sf::VideoMode(sW, sH), "Audio Actor Test!");
    window.setFramerateLimit(144);

    sf::Clock clock;
    float currentElapsedTime = 0.0f;
    int frameCount = 0;
    float fpsUpdateTime = 0.0f;
    float fpsUpdateInterval = 0.2f;
    float fpsVal = 0.f;

    sf::Vector2f ballPos = {sW/2, sH/2};

    sf::SoundBuffer radarBuffer;
    if(!radarBuffer.loadFromFile("assets/audiofiles/radarSFX.wav"))
    {
        std::cerr << "Failed to load audio file." << std::endl;
        return -1;
    }

    sf::Sound radarSound(radarBuffer);
    radarSound.setVolume(100);

    /*IPLAudioBuffer saBuffer;
    saBuffer.numChannels = radarBuffer.getChannelCount();
    saBuffer.numSamples = radarBuffer.getSampleCount();

    std::vector<IPLfloat32> saData(saBuffer.numChannels * saBuffer.numSamples);

    const sf::Int16* sfmlBufferData = radarBuffer.getSamples();

    for(size_t i = 0; i < saBuffer.numChannels * saBuffer.numSamples; ++i)
    {
        saData[i] = static_cast<IPLfloat32>(sfmlBufferData[i] / 32768.0f);
    }

    IPLfloat32* dataPtr = saData.data();
    saBuffer.data = &dataPtr;*/

    sf::Font font;
    if(!font.loadFromFile("assets/fonts/ARIAL.TTF"))
    {
        std::cerr << "Failed to load font file." << std::endl;
        return -1;
    }

    sf::Color circleColor(100, 100, 100);
    sf::Color focusColor(140, 10, 60);
    sf::Color radarColor(255, 255, 255, 30);

    sf::Text timeText;
    timeText.setFont(font);
    timeText.setCharacterSize(20);
    timeText.setFillColor(sf::Color::White);
    timeText.setPosition(20, 20);

    sf::Text mousePosText;
    mousePosText.setFont(font);
    mousePosText.setCharacterSize(20);
    mousePosText.setFillColor(sf::Color::White);
    mousePosText.setPosition(20, 50);

    sf::Text radianText;
    radianText.setFont(font);
    radianText.setCharacterSize(20);
    radianText.setFillColor(sf::Color::White);
    radianText.setPosition(20, 80);

    sf::Text fpsText;
    fpsText.setFont(font);
    fpsText.setCharacterSize(20);
    fpsText.setFillColor(sf::Color::White); 
    fpsText.setPosition(1750, 20);

    sf::CircleShape circleShape(20.f);
    circleShape.setFillColor(circleColor);
    circleShape.setOrigin(20.f, 20.f);

    float outerRadius = 200.f;
    float maxRadius = 1000.f;
    float minRadius = 60.f;
    float startAngle;
    float endAngle;
    int   segments = 100;

    sf::VertexArray focusShape(sf::PrimitiveType::TriangleFan, segments);

    float radarRadius = 10.f;
    float maxRadarRadius = 600.f;
    sf::CircleShape radarCircle(radarRadius);
    radarCircle.setFillColor(radarColor);
    radarCircle.setOrigin(radarRadius, radarRadius);
    bool isRadarExpanding = false;

    sf::Mouse mouse;

    while(window.isOpen())
    {
        sf::Event event;
        while(window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
                window.close();
            if(event.type == sf::Event::KeyPressed)
            {
                if(event.key.code == sf::Keyboard::F)
                {
                    radarSound.play();
                    std::cout << "Radar Pulse played." << std::endl;
                    isRadarExpanding = true;
                    radarRadius = 10.f;
                }
            }
        }

        float deltaTime = clock.restart().asSeconds();
        frameCount++;
        currentElapsedTime += deltaTime;

        if (currentElapsedTime - fpsUpdateTime >= fpsUpdateInterval)
        {
            fpsVal = frameCount / (currentElapsedTime - fpsUpdateTime);
            fpsUpdateTime = currentElapsedTime;
            frameCount = 0;
        }

        sf::Vector2i mousePos = mouse.getPosition(window);

        float mouseBallDistance = std::sqrt(std::pow(mousePos.x - ballPos.x, 2) + std::pow(mousePos.y - ballPos.y, 2));        
        float maxDistance = 800.f;

        float focusRadian = std::atan2f(mousePos.y - ballPos.y, mousePos.x - ballPos.x);

        float normalizedDistance = std::min(mouseBallDistance, maxDistance) / maxDistance;

        float invertedNormalizedDistance = 1.0f - normalizedDistance;

        outerRadius = minRadius + normalizedDistance * (maxRadius - minRadius);

        float minSectorWidth = 5.0f * (piVal / 180.0f);
        float maxSectorWidth = 240.0f * (piVal / 180.0f); 
        float sectorWidth = minSectorWidth + invertedNormalizedDistance * (maxSectorWidth - minSectorWidth);

        startAngle = focusRadian - (sectorWidth * 0.5f);
        endAngle = focusRadian + (sectorWidth * 0.5f);

        InputMovement(ballPos, deltaTime);
        UpdateFocusShape(focusShape, (sf::Vector2f){ballPos}, outerRadius, startAngle, endAngle, focusColor);
        UpdateRadarShape(radarCircle, radarRadius, maxRadarRadius, deltaTime, isRadarExpanding);

        window.clear(sf::Color::Black);

        timeText.setString("Elapsed Time: " + std::to_string(currentElapsedTime));
        mousePosText.setString("Mouse Position: x = " + std::to_string(mousePos.x) + " y = " + std::to_string(mousePos.y));
        radianText.setString("Mouse Radian: " + std::to_string(focusRadian));
        fpsText.setString("FPS: " + std::to_string(fpsVal));

        circleShape.setPosition(ballPos);
        radarCircle.setPosition(ballPos);

        window.draw(focusShape);
        window.draw(radarCircle);
        window.draw(circleShape);
        window.draw(timeText);
        window.draw(mousePosText);
        window.draw(radianText);
        window.draw(fpsText);

        window.display();
    }

    return 0;
}