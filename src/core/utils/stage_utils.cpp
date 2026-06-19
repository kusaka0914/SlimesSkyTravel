#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "stage_utils.h"
#include "../states/game_state.h"
#include "../../game/platform_system.h"
#include <iostream>

namespace StageUtils {
    
void createPatrolPlatforms(PlatformSystem& platformSystem, const std::vector<PatrolConfig>& patrolConfigs) {
    for (const auto& config : patrolConfigs) {
        platformSystem.addPlatform(PatrollingPlatform(
            config.points[0], glm::vec3(2.5f, 1, 2.5f), glm::vec3(0.2f, 0.8f, 0.8f),
            config.points, 2.0f
        ));
        printf("Created %s with %zu patrol points\n", config.description.c_str(), config.points.size());
    }
}

void createPatrolPlatforms(GameState& gameState, PlatformSystem& platformSystem,
                          const std::vector<std::vector<glm::vec3>>& paths,
                          const std::string& baseDescription) {
    std::vector<PatrolConfig> patrolConfigs;
    patrolConfigs.reserve(paths.size());
    for (size_t i = 0; i < paths.size(); i++) {
        patrolConfigs.push_back({ paths[i], baseDescription + std::to_string(i + 1) });
    }
    createPatrolPlatforms(platformSystem, patrolConfigs);
}

void createPatrolPlatforms(GameState& gameState, PlatformSystem& platformSystem,
                          std::initializer_list<std::tuple<std::vector<std::tuple<float, float, float>>, std::string>> patrols) {
    for (const auto& patrol : patrols) {
        const auto& points = std::get<0>(patrol);
        std::string description = std::get<1>(patrol);
        
        std::vector<glm::vec3> glmPoints;
        for (const auto& point : points) {
            glmPoints.push_back(glm::vec3(std::get<0>(point), std::get<1>(point), std::get<2>(point)));
        }
        
        platformSystem.addPlatform(PatrollingPlatform(
            glmPoints[0], glm::vec3(2.5f, 1, 2.5f), glm::vec3(0.2f, 0.8f, 0.8f),
            glmPoints, 2.0f
        ));
        printf("Created %s with %zu patrol points\n", description.c_str(), glmPoints.size());
    }
}

void createMovingPlatforms(GameState& gameState, PlatformSystem& platformSystem,
                          std::initializer_list<std::tuple<std::tuple<float, float, float>, std::tuple<float, float, float>, std::tuple<float, float, float>, glm::vec3, float, std::string>> platforms) {
    for (const auto& platform : platforms) {
        const auto& startPos = std::get<0>(platform);
        const auto& endPos = std::get<1>(platform);
        const auto& size = std::get<2>(platform);
        glm::vec3 startPosition = glm::vec3(std::get<0>(startPos), std::get<1>(startPos), std::get<2>(startPos));
        glm::vec3 endPosition = glm::vec3(std::get<0>(endPos), std::get<1>(endPos), std::get<2>(endPos));
        glm::vec3 sizeVec = glm::vec3(std::get<0>(size), std::get<1>(size), std::get<2>(size));
        glm::vec3 color = std::get<3>(platform);
        float speed = std::get<4>(platform);
        std::string description = std::get<5>(platform);
        
        platformSystem.addPlatform(MovingPlatform(startPosition, sizeVec, color, endPosition, speed));
        printf("Created %s from (%.1f, %.1f, %.1f) to (%.1f, %.1f, %.1f)\n", 
               description.c_str(), startPosition.x, startPosition.y, startPosition.z,
               endPosition.x, endPosition.y, endPosition.z);
    }
}

void createFlyingPlatforms(GameState& gameState, PlatformSystem& platformSystem,
                          const std::vector<std::tuple<std::tuple<float, float, float>, std::tuple<float, float, float>, std::tuple<float, float, float>, std::tuple<float, float, float>, float, float, std::string>>& platforms) {
    for (const auto& platform : platforms) {
        const auto& pos = std::get<0>(platform);
        const auto& size = std::get<1>(platform);
        const auto& spawn = std::get<2>(platform);
        const auto& target = std::get<3>(platform);
        glm::vec3 position = glm::vec3(std::get<0>(pos), std::get<1>(pos), std::get<2>(pos));
        glm::vec3 sizeVec = glm::vec3(std::get<0>(size), std::get<1>(size), std::get<2>(size));
        glm::vec3 spawnPosition = glm::vec3(std::get<0>(spawn), std::get<1>(spawn), std::get<2>(spawn));
        glm::vec3 targetPosition = glm::vec3(std::get<0>(target), std::get<1>(target), std::get<2>(target));
        glm::vec3 color = glm::vec3(0.8f, 0.2f, 0.2f);
        float speed = std::get<4>(platform);
        float range = std::get<5>(platform);
        std::string description = std::get<6>(platform);
        
        platformSystem.addPlatform(FlyingPlatform(position, sizeVec, color, spawnPosition, targetPosition, speed, range));
    }
}

void createCyclingDisappearingPlatforms(GameState& gameState, PlatformSystem& platformSystem,
                                       const std::vector<CyclingDisappearingConfig>& configs) {
    for (const auto& config : configs) {
        platformSystem.addPlatform(CycleDisappearingPlatform(
            config.position, config.size, config.color,
            config.visibleTime, config.invisibleTime, config.initialTimer
        ));
    }
}

void createConsecutiveCyclingPlatforms(GameState& gameState, PlatformSystem& platformSystem,
                                      std::initializer_list<std::tuple<std::tuple<float, float, float>, int, float, std::tuple<float, float, float>, glm::vec3, float, float, float, float, std::tuple<float, float, float>, bool>> platforms) {
    for (const auto& platform : platforms) {
        const auto& startPos = std::get<0>(platform);
        int count = std::get<1>(platform);
        float spacing = std::get<2>(platform);
        const auto& size = std::get<3>(platform);
        glm::vec3 color = std::get<4>(platform);
        float visibleTime = std::get<5>(platform);
        float invisibleTime = std::get<6>(platform);
        float blinkTime = std::get<7>(platform);
        float delay = std::get<8>(platform);
        const auto& direction = std::get<9>(platform);
        bool reverseTimer = std::get<10>(platform);
        
        glm::vec3 startPosition = glm::vec3(std::get<0>(startPos), std::get<1>(startPos), std::get<2>(startPos));
        glm::vec3 sizeVec = glm::vec3(std::get<0>(size), std::get<1>(size), std::get<2>(size));
        glm::vec3 directionVec = glm::vec3(std::get<0>(direction), std::get<1>(direction), std::get<2>(direction));
        
        for (int i = 0; i < count; i++) {
            glm::vec3 position = startPosition + directionVec * (i * spacing);
            float initialTimer;
            if (reverseTimer) {
                initialTimer = (count - 1 - i) * delay;  // 逆順のタイマー
            } else {
                initialTimer = i * delay;  // 通常のタイマー
            }
            platformSystem.addPlatform(CycleDisappearingPlatform(
                position, sizeVec, color,
                visibleTime + invisibleTime, visibleTime, blinkTime, initialTimer
            ));
        }
    }
}

void createConsecutiveCyclingPlatforms(GameState& gameState, PlatformSystem& platformSystem,
                                      const std::vector<std::tuple<std::tuple<float, float, float>, int, float, std::tuple<float, float, float>, glm::vec3, float, float, float, float, std::tuple<float, float, float>, bool>>& platforms) {
    for (const auto& platform : platforms) {
        const auto& startPos = std::get<0>(platform);
        int count = std::get<1>(platform);
        float spacing = std::get<2>(platform);
        const auto& size = std::get<3>(platform);
        glm::vec3 color = std::get<4>(platform);
        float visibleTime = std::get<5>(platform);
        float invisibleTime = std::get<6>(platform);
        float blinkTime = std::get<7>(platform);
        float delay = std::get<8>(platform);
        const auto& direction = std::get<9>(platform);
        bool reverseTimer = std::get<10>(platform);
        
        glm::vec3 startPosition = glm::vec3(std::get<0>(startPos), std::get<1>(startPos), std::get<2>(startPos));
        glm::vec3 sizeVec = glm::vec3(std::get<0>(size), std::get<1>(size), std::get<2>(size));
        glm::vec3 directionVec = glm::vec3(std::get<0>(direction), std::get<1>(direction), std::get<2>(direction));
        
        for (int i = 0; i < count; i++) {
            glm::vec3 position = startPosition + directionVec * (i * spacing);
            float initialTimer;
            if (reverseTimer) {
                initialTimer = (count - 1 - i) * delay;  // 逆順のタイマー
            } else {
                initialTimer = i * delay;  // 通常のタイマー
            }
            platformSystem.addPlatform(CycleDisappearingPlatform(
                position, sizeVec, color,
                visibleTime + invisibleTime, visibleTime, blinkTime, initialTimer
            ));
        }
    }
}

void createPatrolPlatformsFromConfig(GameState& gameState, PlatformSystem& platformSystem, const std::vector<PatrolPlatformConfig>& configs) {
    for (const auto& config : configs) {
        if (config.points.size() >= 2) {
            platformSystem.addPlatform(PatrollingPlatform(
                config.points[0], glm::vec3(3, 1, 3), glm::vec3(0.8f, 0.4f, 0.2f),
                config.points, 2.0f
            ));
        }
    }
}

} // namespace StageUtils
