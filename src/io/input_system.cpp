#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "input_system.h"
#include "../physics/physics_system.h"
#include "../core/constants/game_constants.h"
#include "audio_manager.h"

void InputSystem::mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    GameState* gameState = static_cast<GameState*>(glfwGetWindowUserPointer(window));
    
    if (gameState->camera.firstMouse) {
        gameState->camera.lastMouseX = xpos;
        gameState->camera.lastMouseY = ypos;
        gameState->camera.firstMouse = false;
    }
    float xoffset = float(xpos - gameState->camera.lastMouseX);
    float yoffset = float(ypos - gameState->camera.lastMouseY);
    gameState->camera.lastMouseX = float(xpos);
    gameState->camera.lastMouseY = float(ypos);
    
    constexpr float MOUSE_SENSITIVITY = 0.1f;

    xoffset *= MOUSE_SENSITIVITY;
    yoffset *= MOUSE_SENSITIVITY;

    constexpr float MIN_CAMERA_PITCH = -89.0f;
    constexpr float MAX_CAMERA_PITCH = 89.0f;
    
    if (gameState->skills.isFreeCameraActive) {
        gameState->skills.freeCameraYaw += xoffset;
        gameState->skills.freeCameraPitch -= yoffset;
        gameState->skills.freeCameraPitch = std::max(MIN_CAMERA_PITCH, 
                                             std::min(MAX_CAMERA_PITCH, gameState->skills.freeCameraPitch));
    } else if (gameState->camera.isFirstPersonView) {
        gameState->camera.yaw += xoffset;
        gameState->camera.pitch -= yoffset;
        gameState->camera.pitch = std::max(MIN_CAMERA_PITCH, 
                                        std::min(MAX_CAMERA_PITCH, gameState->camera.pitch));
    }
}

void InputSystem::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    GameState* gameState = static_cast<GameState*>(glfwGetWindowUserPointer(window));

    constexpr float MIN_CAMERA_DISTANCE = 1.0f;
    constexpr float MAX_CAMERA_DISTANCE = 50.0f;
    
    gameState->camera.distance -= float(yoffset);
    gameState->camera.distance = std::max(MIN_CAMERA_DISTANCE, 
                                       std::min(MAX_CAMERA_DISTANCE, gameState->camera.distance));
}

void InputSystem::processInput(GLFWwindow* window, GameState& gameState, float deltaTime) {
    float moveSpeed = 6.0f;
    glm::vec3 gravityDirection = glm::vec3(0, -1, 0);
    glm::vec3 moveDir(0.0f);
    gameState.player.isMovingBackward = false;
    
    if (gameState.progress.isGoalReached) {
        return;
    }
    
    if (gameState.progress.isGameOver) {
        return;
    }
    
    if (gameState.skills.isFreeCameraActive) {
        float yaw = glm::radians(gameState.skills.freeCameraYaw);
        float pitch = glm::radians(gameState.skills.freeCameraPitch);
        float cosYaw = cos(yaw);
        float sinYaw = sin(yaw);
        
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            moveDir.x -= cosYaw * cos(pitch);
            moveDir.z -= sinYaw * cos(pitch);
            gameState.player.isShowingFrontTexture = false;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            moveDir.x += cosYaw * cos(pitch);
            moveDir.z += sinYaw * cos(pitch);
            gameState.player.isMovingBackward = true;
            gameState.player.isShowingFrontTexture = true;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            moveDir.x -= sinYaw;
            moveDir.z += cosYaw;
            gameState.player.isShowingFrontTexture = false;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            moveDir.x += sinYaw;
            moveDir.z -= cosYaw;
            gameState.player.isShowingFrontTexture = false;
        }
    } else if (gameState.camera.isFirstPersonView) {
        float yaw = glm::radians(gameState.camera.yaw);
        float pitch = glm::radians(gameState.camera.pitch);
        float cosYaw = cos(yaw);
        float sinYaw = sin(yaw);
        
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            moveDir.x += cosYaw * cos(pitch);
            moveDir.z += sinYaw * cos(pitch);
            gameState.player.isShowingFrontTexture = false;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            moveDir.x -= cosYaw * cos(pitch);
            moveDir.z -= sinYaw * cos(pitch);
            gameState.player.isMovingBackward = true;
            gameState.player.isShowingFrontTexture = true;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            moveDir.x += sinYaw;
            moveDir.z -= cosYaw;
            gameState.player.isShowingFrontTexture = false;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            moveDir.x -= sinYaw;
            moveDir.z += cosYaw;
            gameState.player.isShowingFrontTexture = false;
        }
    } else {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            moveDir.z += 1.0f;
            gameState.player.isShowingFrontTexture = false;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            moveDir.z -= 1.0f;
            gameState.player.isMovingBackward = true;
            gameState.player.isShowingFrontTexture = true;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            moveDir.x += 1.0f;
            gameState.player.isShowingFrontTexture = false;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            moveDir.x -= 1.0f;
            gameState.player.isShowingFrontTexture = false;
        }
    }

    if (glm::length(moveDir) > 0.0f) {
        moveDir = glm::normalize(moveDir);
        
        float moveDistance = moveSpeed * deltaTime;
        
        if (gameState.skills.isInBurstJumpAir) {
            moveDistance *= 2.0f;
        }
        
        glm::vec3 newPosition = gameState.player.position;
        newPosition.x += moveDir.x * moveDistance;
        newPosition.z += moveDir.z * moveDistance;
        
        glm::vec3 playerSize = glm::vec3(1.0f, 1.0f, 1.0f);
        if (!PhysicsSystem::checkPlatformCollision(gameState, newPosition, playerSize)) {
            gameState.player.position = newPosition;
        }
    }
}

void InputSystem::processJumpAndFloat(GLFWwindow* window, GameState& gameState, float deltaTime, const glm::vec3& gravityDirection, PlatformSystem& platformSystem, io::AudioManager& audioManager) {
    if (gameState.progress.isGoalReached) {
        return;
    }
    
    if (gameState.progress.isGameOver) {
        return;
    }
    
    static bool spacePressed = false;
    
    bool spaceCurrentlyPressed = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
    
    bool shouldJump = spaceCurrentlyPressed && !spacePressed;

    if (shouldJump) {
        glm::vec3 playerSize = glm::vec3(1.0f, 1.0f, 1.0f);
        bool onPlatform = false;
        
        const auto& platforms = platformSystem.getPlatforms();
        
        for (const auto& platform : platforms) {
            std::visit([&](const auto& p) {
                if (p.size.x <= 0 || p.size.y <= 0 || p.size.z <= 0) return;
                
                if (gravityDirection.y > 0.5f) {
                    glm::vec3 platformMin = p.position - p.size * 0.5f;
                    glm::vec3 platformMax = p.position + p.size * 0.5f;
                    glm::vec3 playerMin = gameState.player.position - playerSize * 0.5f;
                    glm::vec3 playerMax = gameState.player.position + playerSize * 0.5f;
                    
                    bool horizontalOverlap = (playerMax.x >= platformMin.x && playerMin.x <= platformMax.x &&
                                             playerMax.z >= platformMin.z && playerMin.z <= platformMax.z);
                    
                    if (horizontalOverlap && std::abs(playerMax.y - platformMin.y) < 0.5f) {
                        onPlatform = true;
                    }
                } else {
                    glm::vec3 platformMin = p.position - p.size * 0.5f;
                    glm::vec3 platformMax = p.position + p.size * 0.5f;
                    glm::vec3 playerMin = gameState.player.position - playerSize * 0.5f;
                    glm::vec3 playerMax = gameState.player.position + playerSize * 0.5f;
                    
                    bool horizontalOverlap = (playerMax.x >= platformMin.x && playerMin.x <= platformMax.x &&
                                             playerMax.z >= platformMin.z && playerMin.z <= platformMax.z);
                    
                    if (horizontalOverlap && std::abs(playerMin.y - platformMax.y) < 0.5f) {
                        onPlatform = true;
                    }
                }
            }, platform);
            
            if (onPlatform) break;
        }
        
        if (onPlatform) {
            if (gameState.audioEnabled) {
                audioManager.playSFX("jump");
            }
            
            if (gameState.skills.isBurstJumpActive && !gameState.skills.hasUsedBurstJump) {
                if (gravityDirection.y > 0.5f) {
                    gameState.player.velocity.y = -20.0f; // 重力反転時は下向きにバーストジャンプ
                } else {
                    gameState.player.velocity.y = 20.0f; // 通常時は上向きにバーストジャンプ
                }
                gameState.skills.hasUsedBurstJump = true;
                gameState.skills.isBurstJumpActive = false; // バーストジャンプを使用したので非アクティブに
                gameState.skills.burstJumpDelayTimer = 0.01f; // 1秒後に空中フラグを設定
            } else {
                if (gravityDirection.y > 0.5f) {
                    gameState.player.velocity.y = -8.0f; // 重力反転時は下向きにジャンプ
                } else {
                    gameState.player.velocity.y = 8.0f; // 通常時は上向きにジャンプ
                }
            }
            gameState.player.canDoubleJump = true;
            if (gameState.skills.isInBurstJumpAir) {
                gameState.skills.isInBurstJumpAir = false; // バーストジャンプ空中フラグをリセット
            }
        } else if ((gameState.progress.selectedSecretStarType == GameProgressState::SecretStarType::NONE) && 
                   !gameState.progress.isTimeAttackMode &&
                   ((gameState.progress.isEasyMode && gameState.player.canDoubleJump) || 
                   (!gameState.progress.isEasyMode && gameState.skills.hasDoubleJumpSkill && gameState.skills.doubleJumpRemainingUses > 0 && gameState.player.canDoubleJump))) {
            if (gameState.progress.currentStage != 0) {
                if (gameState.audioEnabled) {
                    audioManager.playSFX("jump");
                }
                
                if (gravityDirection.y > 0.5f) {
                    gameState.player.velocity.y = -6.0f; // 重力反転時は下向きにジャンプ
                } else {
                    gameState.player.velocity.y = 6.0f; // 通常時は上向きにジャンプ
                }
                gameState.player.canDoubleJump = false; // 二段ジャンプを使用
                
                if (!gameState.progress.isEasyMode) {
                    gameState.skills.doubleJumpRemainingUses--;
                }
            }
        } else if ((gameState.progress.selectedSecretStarType == GameProgressState::SecretStarType::NONE) && 
                   !gameState.progress.isTimeAttackMode &&
                   gameState.skills.isBurstJumpActive && !gameState.skills.hasUsedBurstJump && !gameState.skills.isInBurstJumpAir) {
            if (gameState.audioEnabled) {
                audioManager.playSFX("jump");
            }
            
            if (gravityDirection.y > 0.5f) {
                gameState.player.velocity.y = -20.0f; // 重力反転時は下向きにバーストジャンプ
            } else {
                gameState.player.velocity.y = 20.0f; // 通常時は上向きにバーストジャンプ
            }
            gameState.skills.hasUsedBurstJump = true;
            gameState.skills.isBurstJumpActive = false; // バーストジャンプを使用したので非アクティブに
            gameState.skills.isInBurstJumpAir = true; // バーストジャンプ空中フラグを設定
        }
    }
    spacePressed = spaceCurrentlyPressed;
}