#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "json_stage_loader.h"
#include "../core/utils/stage_utils.h"
#include "../core/constants/stage_constants.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#ifdef _WIN32
    #include <windows.h>
#else
    #include <cstdio>
#endif

bool JsonStageLoader::fileExists(const std::string& filename) {
    std::ifstream file(filename);
    bool exists = file.good();
    file.close();
    return exists;
}

bool JsonStageLoader::loadStageFromJSON(const std::string& filename, GameState& gameState, PlatformSystem& platformSystem) {
    if (!fileExists(filename)) {
        return false;
    }
    
    try {
        std::ifstream file(filename);
        nlohmann::json root;
        file >> root;
        
        if (root.contains("stageInfo")) {
            if (!parseStageInfo(root["stageInfo"], gameState)) {
                return false;
            }
        }
        
        if (root.contains("items")) {
            if (!parseItems(root, gameState, platformSystem)) {
                return false;
            }
        }
        
        if (root.contains("staticPlatforms")) {
            if (!parseStaticPlatforms(root, gameState, platformSystem)) {
                return false;
            }
        }
        
        if (root.contains("patrolPlatforms")) {
            if (!parsePatrolPlatforms(root, gameState, platformSystem)) {
                return false;
            }
        }
        
        if (root.contains("cyclingDisappearingPlatforms")) {
            if (!parseDisappearingPlatforms(root, gameState, platformSystem)) {
                return false;
            }
        }
        
        if (root.contains("consecutiveCyclingPlatforms")) {
            if (!parseConsecutiveCyclingPlatforms(root, gameState, platformSystem)) {
                return false;
            }
        }
        
        if (root.contains("flyingPlatforms")) {
            if (!parseFlyingPlatforms(root, gameState, platformSystem)) {
                return false;
            }
        }
        
        if (root.contains("stageSelectionAreas")) {
            if (!parseStageSelectionAreas(root, gameState, platformSystem)) {
                return false;
            }
        }
        
        if (root.contains("leaderboardArea")) {
            if (!parseLeaderboardArea(root, gameState, platformSystem)) {
                return false;
            }
        }
        
        if (root.contains("conditionalCyclingDisappearingPlatforms")) {
            if (!parseConditionalCyclingDisappearingPlatforms(root, gameState, platformSystem)) {
                return false;
            }
        }
        
        if (root.contains("conditionalPatrolPlatforms")) {
            if (!parseConditionalPatrolPlatforms(root, gameState, platformSystem)) {
                return false;
            }
        }
        
        if (root.contains("conditionalFlyingPlatforms")) {
            if (!parseConditionalFlyingPlatforms(root, gameState, platformSystem)) {
                return false;
            }
        }
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool JsonStageLoader::parseStageInfo(const nlohmann::json& stageInfo, GameState& gameState) {
    try {
        if (stageInfo.contains("playerStartPosition")) {
            auto pos = stageInfo["playerStartPosition"];
            gameState.player.position = glm::vec3(
                pos[0].get<float>(),
                pos[1].get<float>(),
                pos[2].get<float>()
            );
        }
        
        if (stageInfo.contains("goalPosition")) {
            auto pos = stageInfo["goalPosition"];
            gameState.progress.goalPosition = glm::vec3(
                pos[0].get<float>(),
                pos[1].get<float>(),
                pos[2].get<float>()
            );
        }
        
        if (stageInfo.contains("timeLimit")) {
            gameState.progress.timeLimit = stageInfo["timeLimit"].get<float>();
            gameState.progress.remainingTime = gameState.progress.timeLimit;
        }
        
        return true;
        
    } catch (const std::exception& e) {
        return false;
    }
}

bool JsonStageLoader::parseItems(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem) {
    try {
        for (int i = 0; i < root["items"].size(); i++) {
            Item item;
            auto items = root["items"][i];

            auto pos = items["position"];
            item.position = glm::vec3(
                pos[0].get<float>(),
                pos[1].get<float>(),
                pos[2].get<float>()
            );

            item.position += glm::vec3(0, 1, 0);
            
            auto color = items["color"];
            item.color = glm::vec3(
                color[0].get<float>(),
                color[1].get<float>(),
                color[2].get<float>()
            );

            item.itemId = i + 1;
        
            gameState.items.items.push_back(item);
            platformSystem.addPlatform(StaticPlatform(
                item.position - glm::vec3(0, 1, 0), glm::vec3(3, 1, 3), glm::vec3(0.2f, 0.6f, 1.0f)
            ));
        }
        
        return true;
        
    } catch (const std::exception& e) {
        return false;
    }
}

bool JsonStageLoader::parseStaticPlatforms(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem) {
    try {
        for (const auto& platform : root["staticPlatforms"]) {
            auto pos = platform["position"];
            glm::vec3 posVec = glm::vec3(
                pos[0].get<float>(),
                pos[1].get<float>(),
                pos[2].get<float>()
            );
            
            auto size = platform["size"];
            glm::vec3 sizeVec = glm::vec3(
                size[0].get<float>(),
                size[1].get<float>(),
                size[2].get<float>()
            );
            
            auto color = platform["color"];
            glm::vec3 colorVec = glm::vec3(
                color[0].get<float>(),
                color[1].get<float>(),
                color[2].get<float>()
            );
            
            platformSystem.addPlatform(StaticPlatform(posVec, sizeVec, colorVec));
        }
        
        return true;
        
    } catch (const std::exception& e) {
        return false;
    }
}

bool JsonStageLoader::parsePatrolPlatforms(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem) {
    try {
        std::vector<PatrolPlatformConfig> platformConfigs;
        
        for (const auto& platform : root["patrolPlatforms"]) {
            PatrolPlatformConfig config;
            
            for (const auto& point : platform["patrolPoints"]) {
                glm::vec3 patrolPoint(
                    point[0].get<float>(),
                    point[1].get<float>(),
                    point[2].get<float>()
                );
                config.points.push_back(patrolPoint);
            }
            
            config.description = platform["description"].get<std::string>();
            platformConfigs.push_back(config);
        }
        
        StageUtils::createPatrolPlatformsFromConfig(gameState, platformSystem, platformConfigs);
        
        return true;
        
    } catch (const std::exception& e) {
        return false;
    }
}

bool JsonStageLoader::parseDisappearingPlatforms(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem) {
    if (!root.contains("cyclingDisappearingPlatforms")) {
        return true;
    }
    
    std::vector<CyclingDisappearingConfig> configs;
    for (const auto& platform : root["cyclingDisappearingPlatforms"]) {
        CyclingDisappearingConfig config;
        config.position = glm::vec3(
            platform["position"][0].get<float>(),
            platform["position"][1].get<float>(),
            platform["position"][2].get<float>()
        );
        config.size = glm::vec3(
            platform["size"][0].get<float>(),
            platform["size"][1].get<float>(),
            platform["size"][2].get<float>()
        );
        config.color = glm::vec3(
            platform["color"][0].get<float>(),
            platform["color"][1].get<float>(),
            platform["color"][2].get<float>()
        );
        config.visibleTime = platform["visibleTime"].get<float>();
        config.invisibleTime = platform["invisibleTime"].get<float>();
        config.initialTimer = platform["initialTimer"].get<float>();
        config.description = platform["description"].get<std::string>();
        configs.push_back(config);
    }
    
    StageUtils::createCyclingDisappearingPlatforms(gameState, platformSystem, configs);
    return true;
}

bool JsonStageLoader::parseConsecutiveCyclingPlatforms(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem) {
    if (!root.contains("consecutiveCyclingPlatforms")) {
        return true;
    }
    
    std::vector<std::tuple<std::tuple<float, float, float>, int, float, std::tuple<float, float, float>, glm::vec3, float, float, float, float, std::tuple<float, float, float>, bool>> consecutiveConfigs;
    
    for (const auto& platform : root["consecutiveCyclingPlatforms"]) {
        glm::vec3 startPosition = glm::vec3(
            platform["startPosition"][0].get<float>(),
            platform["startPosition"][1].get<float>(),
            platform["startPosition"][2].get<float>()
        );
        int count = platform["count"].get<int>();
        float spacing = platform["spacing"].get<float>();
        glm::vec3 size = glm::vec3(
            platform["size"][0].get<float>(),
            platform["size"][1].get<float>(),
            platform["size"][2].get<float>()
        );
        glm::vec3 color = glm::vec3(
            platform["color"][0].get<float>(),
            platform["color"][1].get<float>(),
            platform["color"][2].get<float>()
        );
        float visibleTime = platform["visibleTime"].get<float>();
        float invisibleTime = platform["invisibleTime"].get<float>();
        float blinkTime = platform["blinkTime"].get<float>();
        float delay = platform["delay"].get<float>();
        glm::vec3 direction = glm::vec3(
            platform["direction"][0].get<float>(),
            platform["direction"][1].get<float>(),
            platform["direction"][2].get<float>()
        );
        bool reverseTimer = platform["reverseTimer"].get<bool>();
        
        consecutiveConfigs.push_back({
            {startPosition.x, startPosition.y, startPosition.z}, 
            count, 
            spacing, 
            {size.x, size.y, size.z}, 
            color, 
            visibleTime, invisibleTime, blinkTime, delay, 
            {direction.x, direction.y, direction.z}, 
            reverseTimer
        });
    }
    
    StageUtils::createConsecutiveCyclingPlatforms(gameState, platformSystem, consecutiveConfigs);
    return true;
}

bool JsonStageLoader::parseFlyingPlatforms(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem) {
    if (!root.contains("flyingPlatforms")) {
        return true;
    }
    
    std::vector<std::tuple<std::tuple<float, float, float>, std::tuple<float, float, float>, std::tuple<float, float, float>, std::tuple<float, float, float>, float, float, std::string>> flyingPlatforms;
    
    for (const auto& platform : root["flyingPlatforms"]) {
        glm::vec3 position = glm::vec3(
            platform["position"][0].get<float>(),
            platform["position"][1].get<float>(),
            platform["position"][2].get<float>()
        );
        glm::vec3 size = glm::vec3(
            platform["size"][0].get<float>(),
            platform["size"][1].get<float>(),
            platform["size"][2].get<float>()
        );
        glm::vec3 spawnPosition = glm::vec3(
            platform["spawnPosition"][0].get<float>(),
            platform["spawnPosition"][1].get<float>(),
            platform["spawnPosition"][2].get<float>()
        );
        glm::vec3 targetPosition = glm::vec3(
            platform["targetPosition"][0].get<float>(),
            platform["targetPosition"][1].get<float>(),
            platform["targetPosition"][2].get<float>()
        );
        float speed = platform["speed"].get<float>();
        float range = platform["range"].get<float>();
        std::string description = platform["description"].get<std::string>();
        
        flyingPlatforms.push_back({
            {position.x, position.y, position.z},
            {size.x, size.y, size.z},
            {spawnPosition.x, spawnPosition.y, spawnPosition.z},
            {targetPosition.x, targetPosition.y, targetPosition.z},
            speed,
            range,
            description
        });
    }
    
    StageUtils::createFlyingPlatforms(gameState, platformSystem, flyingPlatforms);
    return true;
}

bool JsonStageLoader::parseStageSelectionAreas(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem) {
    try {
        if (!root.contains("stageSelectionAreas")) {
            return true;
        }
        
        for (const auto& area : root["stageSelectionAreas"]) {
            int stageNumber = area["stageNumber"].get<int>();
            glm::vec3 position = glm::vec3(
                area["position"][0].get<float>(),
                area["position"][1].get<float>(),
                area["position"][2].get<float>()
            );
            glm::vec3 size = glm::vec3(
                area["size"][0].get<float>(),
                area["size"][1].get<float>(),
                area["size"][2].get<float>()
            );
            
            glm::vec3 color;
            if (gameState.progress.unlockedStages.count(stageNumber) && gameState.progress.unlockedStages[stageNumber]) {
                color = glm::vec3(0.2f, 1.0f, 0.2f);
            } else {
                color = glm::vec3(0.5f, 0.5f, 0.5f);
            }
            
            platformSystem.addPlatform(StaticPlatform(position, size, color));
        }
        
        return true;
        
    } catch (const std::exception& e) {
        return false;
    }
}

bool JsonStageLoader::parseLeaderboardArea(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem) {
    try {
        if (!root.contains("leaderboardArea")) {
            return true;
        }
        
        const auto& area = root["leaderboardArea"];
        glm::vec3 position = glm::vec3(
            area["position"][0].get<float>(),
            area["position"][1].get<float>(),
            area["position"][2].get<float>()
        );
        glm::vec3 size = glm::vec3(
            area["size"][0].get<float>(),
            area["size"][1].get<float>(),
            area["size"][2].get<float>()
        );
        
        gameState.ui.leaderboardPosition = position;
        
        glm::vec3 color = glm::vec3(0.2f, 0.4f, 1.0f);
        platformSystem.addPlatform(StaticPlatform(position, size, color));
        
        return true;
        
    } catch (const std::exception& e) {
        return false;
    }
}

bool JsonStageLoader::parseConditionalCyclingDisappearingPlatforms(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem) {
    if (!root.contains("conditionalCyclingDisappearingPlatforms")) {
        return true;
    }
    
    try {
        for (const auto& conditionalGroup : root["conditionalCyclingDisappearingPlatforms"]) {
            std::string condition = conditionalGroup["condition"].get<std::string>();
            bool shouldAdd = false;
            
            if (condition == "stage3Cleared") {
                shouldAdd = (gameState.progress.stageStars.count(3) && gameState.progress.stageStars.at(3) > 0);
            }
            
            if (shouldAdd) {
                std::vector<CyclingDisappearingConfig> configs;
                for (const auto& platform : conditionalGroup["platforms"]) {
                    CyclingDisappearingConfig config;
                    config.position = glm::vec3(platform["position"][0].get<float>(), platform["position"][1].get<float>(), platform["position"][2].get<float>());
                    config.size = glm::vec3(platform["size"][0].get<float>(), platform["size"][1].get<float>(), platform["size"][2].get<float>());
                    config.color = glm::vec3(platform["color"][0].get<float>(), platform["color"][1].get<float>(), platform["color"][2].get<float>());
                    config.visibleTime = platform["visibleTime"].get<float>();
                    config.invisibleTime = platform["invisibleTime"].get<float>();
                    config.initialTimer = platform["initialTimer"].get<float>();
                    config.description = platform["description"].get<std::string>();
                    configs.push_back(config);
                }
                
                StageUtils::createCyclingDisappearingPlatforms(gameState, platformSystem, configs);
            }
        }
        
        return true;
        
    } catch (const std::exception& e) {
        return false;
    }
}

bool JsonStageLoader::parseConditionalPatrolPlatforms(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem) {
    if (!root.contains("conditionalPatrolPlatforms")) {
        return true;
    }
    
    try {
        for (const auto& conditionalGroup : root["conditionalPatrolPlatforms"]) {
            std::string condition = conditionalGroup["condition"].get<std::string>();
            bool shouldAdd = false;
            
            if (condition == "stage2Cleared") {
                shouldAdd = (gameState.progress.stageStars.count(2) && gameState.progress.stageStars.at(2) > 0);
            }
            
            if (shouldAdd) {
                std::vector<PatrolPlatformConfig> configs;
                for (const auto& platform : conditionalGroup["platforms"]) {
                    PatrolPlatformConfig config;
                    config.description = platform["description"].get<std::string>();
                    
                    for (const auto& point : platform["patrolPoints"]) {
                        glm::vec3 patrolPoint = glm::vec3(
                            point[0].get<float>(),
                            point[1].get<float>(),
                            point[2].get<float>()
                        );
                        config.points.push_back(patrolPoint);
                    }
                    
                    configs.push_back(config);
                }
                
                StageUtils::createPatrolPlatformsFromConfig(gameState, platformSystem, configs);
            }
        }
        
        return true;
        
    } catch (const std::exception& e) {
        return false;
    }
}

bool JsonStageLoader::parseConditionalFlyingPlatforms(const nlohmann::json& root, GameState& gameState, PlatformSystem& platformSystem) {
    if (!root.contains("conditionalFlyingPlatforms")) {
        return true;
    }
    
    try {
        for (const auto& conditionalGroup : root["conditionalFlyingPlatforms"]) {
            std::string condition = conditionalGroup["condition"].get<std::string>();
            bool shouldAdd = false;
            
            if (condition == "stage4Cleared") {
                shouldAdd = (gameState.progress.stageStars.count(4) && gameState.progress.stageStars.at(4) > 0);
            }
            
            if (shouldAdd) {
                std::vector<std::tuple<std::tuple<float, float, float>, std::tuple<float, float, float>, std::tuple<float, float, float>, std::tuple<float, float, float>, float, float, std::string>> flyingPlatforms;
                
                for (const auto& platform : conditionalGroup["platforms"]) {
                    glm::vec3 position = glm::vec3(platform["position"][0].get<float>(), platform["position"][1].get<float>(), platform["position"][2].get<float>());
                    glm::vec3 size = glm::vec3(platform["size"][0].get<float>(), platform["size"][1].get<float>(), platform["size"][2].get<float>());
                    glm::vec3 spawnPosition = glm::vec3(platform["spawnPosition"][0].get<float>(), platform["spawnPosition"][1].get<float>(), platform["spawnPosition"][2].get<float>());
                    glm::vec3 targetPosition = glm::vec3(platform["targetPosition"][0].get<float>(), platform["targetPosition"][1].get<float>(), platform["targetPosition"][2].get<float>());
                    float speed = platform["speed"].get<float>();
                    float range = platform["range"].get<float>();
                    std::string description = platform["description"].get<std::string>();
                    
                    flyingPlatforms.push_back({
                        {position.x, position.y, position.z},
                        {size.x, size.y, size.z},
                        {spawnPosition.x, spawnPosition.y, spawnPosition.z},
                        {targetPosition.x, targetPosition.y, targetPosition.z},
                        speed,
                        range,
                        description
                    });
                }
                
                StageUtils::createFlyingPlatforms(gameState, platformSystem, flyingPlatforms);
            }
        }
        
        return true;
        
    } catch (const std::exception& e) {
        return false;
    }
}