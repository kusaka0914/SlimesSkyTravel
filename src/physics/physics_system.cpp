#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "physics_system.h"

bool PhysicsSystem::checkPlatformCollision(const GameState& gameState, const glm::vec3& playerPos, const glm::vec3& playerSize) {
    for (const auto& platform : gameState.platforms) {
        if (platform.size.x <= 0 || platform.size.y <= 0 || platform.size.z <= 0) continue;
        
        glm::vec3 platformMin = platform.position - platform.size * 0.5f;
        glm::vec3 platformMax = platform.position + platform.size * 0.5f;
        glm::vec3 playerMin = playerPos - playerSize * 0.5f;
        glm::vec3 playerMax = playerPos + playerSize * 0.5f;
        
        bool basicCollision = (playerMax.x >= platformMin.x && playerMin.x <= platformMax.x &&
                              playerMax.y >= platformMin.y && playerMin.y <= platformMax.y &&
                              playerMax.z >= platformMin.z && playerMin.z <= platformMax.z);
        
        if (!basicCollision) continue;
        
        bool onTopSurface = (std::abs(playerMin.y - platformMax.y) < 0.1f) && 
                           (playerMax.x >= platformMin.x && playerMin.x <= platformMax.x &&
                            playerMax.z >= platformMin.z && playerMin.z <= platformMax.z);
        
        if (!onTopSurface) {
            return true;
        }
    }
    return false;
}