#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "game_updater.h"
#include "game_loop.h"
#include "../physics/physics_system.h"
#include "../core/utils/physics_utils.h"
#include "../core/utils/ui_config_manager.h"
#include "../io/input_system.h"
#include "../game/replay_manager.h"
#include "../game/save_manager.h"
#include "input_handler.h"

namespace GameLoop {
void GameUpdater::updateGameState(
    GLFWwindow* window, 
    GameState& gameState, 
    StageManager& stageManager, 
    PlatformSystem& platformSystem, 
    float deltaTime, 
    float scaledDeltaTime,
    std::map<int, InputUtils::KeyState>& keyStates,
    std::function<void()> resetStageStartTime, 
    io::AudioManager& audioManager
) { 
    if (gameState.ui.isTransitioning) {
        const float FADE_DURATION = 0.5f;
        gameState.ui.transitionTimer += deltaTime;
        
        if (gameState.ui.transitionType == UIState::TransitionType::FADE_OUT) {
            if (gameState.ui.transitionTimer >= FADE_DURATION) {
                if (gameState.ui.pendingStageTransition == 6) {
                    gameState.ui.showTitleScreen = false;
                    
                    stageManager.loadStage(6, gameState, platformSystem);
                    gameState.ui.showReadyScreen = false;
                    gameState.ui.readyScreenShown = true;
                    gameState.ui.isCountdownActive = false;
                    gameState.ui.countdownTimer = 0.0f;
                    resetStageStartTime();
                    
                    gameState.ui.transitionType = UIState::TransitionType::FADE_IN;
                    gameState.ui.transitionTimer = deltaTime;
                }
            }
        } else if (gameState.ui.transitionType == UIState::TransitionType::FADE_IN) {
            if (gameState.ui.transitionTimer >= FADE_DURATION) {
                gameState.ui.isTransitioning = false;
                gameState.ui.transitionType = UIState::TransitionType::NONE;
                gameState.ui.transitionTimer = 0.0f;
                gameState.ui.pendingStageTransition = -1;
                gameState.ui.pendingReadyScreen = false;
                gameState.ui.pendingSkipCountdown = false;
            }
        }
        
        return;
    }
    
    if (gameState.replay.pendingReplayLoad && gameState.replay.pendingReplayStage > 0) {
        int replayStage = gameState.replay.pendingReplayStage;
        gameState.replay.pendingReplayLoad = false;
        gameState.replay.pendingReplayStage = 0;
        
        resetStageStartTime();

        gameState.progress.clearTime = 0.0f;
        gameState.progress.isTimeAttackMode = true;
        gameState.progress.currentTimeAttackTime = 0.0f;
        gameState.progress.timeAttackStartTime = 0.0f;
        
        gameState.replay.isReplayMode = true;
        gameState.replay.isReplayPaused = false;
        gameState.replay.replayPlaybackTime = 0.0f;
        gameState.replay.previousReplayPlaybackTime = 0.0f;
        gameState.replay.replayPlaybackSpeed = 1.0f;
        gameState.ui.showStageClearUI = false;
        gameState.progress.isStageCompleted = false;
        gameState.progress.isGoalReached = false;
        
        bool stageLoaded = stageManager.goToStage(replayStage, gameState, platformSystem);
        
        if (!gameState.replay.currentReplay.frames.empty()) {
            const auto& firstFrame = gameState.replay.currentReplay.frames[0];
            gameState.player.position = firstFrame.playerPosition;
            gameState.player.velocity = firstFrame.playerVelocity;
        }
    }
    
    static float fileCheckTimer = 0.0f;
    fileCheckTimer += deltaTime;
    if (fileCheckTimer >= 0.5f) {
        fileCheckTimer = 0.0f;
        stageManager.checkAndReloadStage(gameState, platformSystem);
        gfx::TextureManager::checkAndReloadTextures();
        audioManager.checkAndReloadAudio();
        UIConfig::UIConfigManager::getInstance().checkAndReloadConfig();
    }
    
    if (gameState.skills.isTimeStopped) {
        gameState.skills.timeStopTimer -= deltaTime;
        if (gameState.skills.timeStopTimer <= 0.0f) {
            gameState.skills.isTimeStopped = false;
            gameState.skills.timeStopTimer = 0.0f;
        }
    }
    
    if (gameState.skills.isFreeCameraActive) {
        gameState.skills.freeCameraTimer -= deltaTime;
        if (gameState.skills.freeCameraTimer <= 0.0f) {
            gameState.skills.isFreeCameraActive = false;
            gameState.skills.freeCameraTimer = 0.0f;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
    
    if (gameState.skills.burstJumpDelayTimer > 0.0f) {
        gameState.skills.burstJumpDelayTimer -= deltaTime;
        if (gameState.skills.burstJumpDelayTimer <= 0.0f) {
            gameState.skills.isInBurstJumpAir = true;
            gameState.skills.burstJumpDelayTimer = 0.0f;
        }
    }
    
    if (gameState.replay.isReplayMode) {
        float rewindSpeed = 0.0f;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            if (gameState.replay.isReplayPaused) {
                rewindSpeed = -0.5f;  // ポーズ中はゆっくり巻き戻し
            } else {
                rewindSpeed = -2.0f;  // 通常時は2倍速で巻き戻し
            }
        } else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            if (gameState.replay.isReplayPaused) {
                rewindSpeed = 0.5f;   // ポーズ中はゆっくり早送り
            } else {
                rewindSpeed = 2.0f;   // 通常時は2倍速で早送り
            }
        }
        
        if (!gameState.replay.isReplayPaused && rewindSpeed == 0.0f) {
            gameState.replay.replayPlaybackTime += deltaTime * gameState.replay.replayPlaybackSpeed;
        } else if (rewindSpeed != 0.0f) {
            gameState.replay.replayPlaybackTime += deltaTime * rewindSpeed * gameState.replay.replayPlaybackSpeed;
            if (gameState.replay.replayPlaybackTime < 0.0f) {
                gameState.replay.replayPlaybackTime = 0.0f;
            }
        }
        
        if (!gameState.replay.currentReplay.frames.empty()) {
            if (gameState.replay.replayPlaybackTime >= gameState.replay.currentReplay.frames.back().timestamp) {
                gameState.replay.replayPlaybackTime = gameState.replay.currentReplay.frames.back().timestamp;
                
                gameState.progress.clearTime = gameState.replay.replayPlaybackTime;
                
                if (gameState.progress.isTimeAttackMode) {
                    gameState.progress.currentTimeAttackTime = gameState.replay.replayPlaybackTime;
                }
                
                gameState.replay.isReplayMode = false;
                gameState.replay.isReplayPaused = false;
                gameState.ui.showStageClearUI = true;
                gameState.progress.gameWon = true;
                gameState.progress.isStageCompleted = true;
                gameState.progress.isGoalReached = true;
                gameState.replay.replayPlaybackTime = 0.0f;
                
                const auto& lastFrame = gameState.replay.currentReplay.frames.back();
                gameState.player.position = lastFrame.playerPosition;
                gameState.player.velocity = lastFrame.playerVelocity;
            } else {
                if (gameState.replay.currentReplay.frames.size() > 0 && 
                    gameState.replay.replayPlaybackTime <= gameState.replay.currentReplay.frames[0].timestamp) {
                    const auto& firstFrame = gameState.replay.currentReplay.frames[0];
                    gameState.player.position = firstFrame.playerPosition;
                    gameState.player.velocity = firstFrame.playerVelocity;
                } else {
                    bool frameFound = false;
                    for (size_t i = 0; i < gameState.replay.currentReplay.frames.size() - 1; i++) {
                        const auto& frame1 = gameState.replay.currentReplay.frames[i];
                        const auto& frame2 = gameState.replay.currentReplay.frames[i + 1];
                        
                            // フレーム間の補間（境界を含む）
                        if (gameState.replay.replayPlaybackTime >= frame1.timestamp && 
                            gameState.replay.replayPlaybackTime <= frame2.timestamp) {
                            float t = 0.0f;
                            if (frame2.timestamp > frame1.timestamp) {
                                t = (gameState.replay.replayPlaybackTime - frame1.timestamp) / 
                                (frame2.timestamp - frame1.timestamp);
                            }
                            t = std::clamp(t, 0.0f, 1.0f);
                            
                            gameState.player.position = glm::mix(frame1.playerPosition, frame2.playerPosition, t);
                            gameState.player.velocity = glm::mix(frame1.playerVelocity, frame2.playerVelocity, t);
                            
                            if (!frame1.itemCollectedStates.empty() && frame1.itemCollectedStates.size() == gameState.items.items.size() &&
                                !frame2.itemCollectedStates.empty() && frame2.itemCollectedStates.size() == gameState.items.items.size()) {
                                bool useFrame1 = (t < 0.5f);
                                
                                for (size_t j = 0; j < gameState.items.items.size() && j < frame1.itemCollectedStates.size(); j++) {
                                    bool shouldBeCollected = useFrame1 ? frame1.itemCollectedStates[j] : frame2.itemCollectedStates[j];
                                    
                                    if (gameState.items.items[j].isCollected != shouldBeCollected) {
                                        if (shouldBeCollected) {
                                            gameState.items.items[j].isCollected = true;
                                            gameState.items.collectedItems++;
                                        } else {
                                            gameState.items.items[j].isCollected = false;
                                            gameState.items.collectedItems--;
                                        }
                                    }
                                }
                            }
                            
                            frameFound = true;
                            break;
                        }
                    }
                    
                    if (!frameFound && !gameState.replay.currentReplay.frames.empty()) {
                        const auto& lastFrame = gameState.replay.currentReplay.frames.back();
                        gameState.player.position = lastFrame.playerPosition;
                        gameState.player.velocity = lastFrame.playerVelocity;
                    }
                }
            }
        }
    }
    
    if (gameState.replay.isRecordingReplay && gameState.progress.isTimeAttackMode) {
        if (!gameState.ui.isCountdownActive && gameState.progress.timeAttackStartTime > 0.0f) {
            gameState.replay.replayRecordTimer += deltaTime;
            
            if (gameState.replay.replayRecordTimer >= gameState.replay.REPLAY_RECORD_INTERVAL) {
                ReplayFrame frame;
                frame.timestamp = gameState.progress.currentTimeAttackTime;
                frame.playerPosition = gameState.player.position;
                frame.playerVelocity = gameState.player.velocity;
                frame.timeScale = gameState.progress.timeScale;
                frame.itemCollectedStates.clear();
                for (const auto& item : gameState.items.items) {
                    frame.itemCollectedStates.push_back(item.isCollected);
                }
                gameState.replay.replayBuffer.push_back(frame);
                gameState.replay.replayRecordTimer = 0.0f;
            }
        }
    }
    
    if (gameState.progress.isTimeAttackMode) {
        if (!gameState.ui.isCountdownActive && gameState.progress.timeAttackStartTime > 0.0f) {
            if (!gameState.progress.isStageCompleted && !gameState.skills.isTimeStopped) {
                gameState.progress.currentTimeAttackTime += deltaTime;
            }
        }
    } else {
        if (!gameState.progress.isStageCompleted && !gameState.progress.isTimeUp && !gameState.skills.isTimeStopped) {
            gameState.progress.remainingTime -= deltaTime;
            
            if (gameState.progress.remainingTime <= 0.0f) {
                gameState.progress.remainingTime = 0.0f;
                gameState.progress.isTimeUp = true;
                gameState.progress.isGameOver = true;
                gameState.progress.gameOverTimer = 0.0f;
            }
        }
    }
    
    if (gameState.progress.isGameOver) {
        gameState.progress.gameOverTimer += deltaTime;
    }
    
    static std::map<int, bool> lastFlyingState;
    const auto& platforms = platformSystem.getPlatforms();
    for (int i = 0; i < platforms.size(); i++) {
        std::visit([&](const auto& platform) {
            if constexpr (std::is_same_v<std::decay_t<decltype(platform)>, FlyingPlatform>) {
                bool isCurrentlyFlying = platform.hasSpawned && platform.isFlying;
                bool wasFlyingLastFrame = lastFlyingState[i];
                
                if (isCurrentlyFlying && !wasFlyingLastFrame) {
                    if (gameState.audioEnabled) {
                        audioManager.playSFX("flying");
                    }
                }
                
                lastFlyingState[i] = isCurrentlyFlying;
            }
        }, platforms[i]);
    }
    
    if (gameState.replay.isReplayMode && gameState.replay.isReplayPaused) {
        scaledDeltaTime = 0.0f;
    }
    
    // リプレイモード中は絶対時間から直接計算、そうでない場合は通常の更新
    if (gameState.replay.isReplayMode && !gameState.replay.isReplayPaused) {
        float currentTimeScale = 1.0f;
        // ギミックの更新に使う実際のゲーム時間を計算
        float actualGameTime = 0.0f;
        
        if (!gameState.replay.currentReplay.frames.empty()) {
            if (gameState.replay.replayPlaybackTime < gameState.replay.currentReplay.frames[0].timestamp) {
                currentTimeScale = gameState.replay.currentReplay.frames[0].timeScale;
                actualGameTime = gameState.replay.replayPlaybackTime * currentTimeScale;
            }
            else if (gameState.replay.replayPlaybackTime > gameState.replay.currentReplay.frames.back().timestamp) {
                currentTimeScale = gameState.replay.currentReplay.frames.back().timeScale;
                // 最後のフレームまでの累積時間を計算
                actualGameTime = 0.0f;
                const auto& firstFrame = gameState.replay.currentReplay.frames[0];
                if (firstFrame.timestamp > 0.0f) {
                    actualGameTime += firstFrame.timestamp * firstFrame.timeScale;
                }
                for (size_t i = 1; i < gameState.replay.currentReplay.frames.size() - 1; i++) {
                    const auto& frame1 = gameState.replay.currentReplay.frames[i];
                    const auto& frame2 = gameState.replay.currentReplay.frames[i + 1];
                    float segmentTime = frame2.timestamp - frame1.timestamp;
                    actualGameTime += segmentTime * frame1.timeScale;
                }
                float lastSegmentTime = gameState.replay.replayPlaybackTime - gameState.replay.currentReplay.frames.back().timestamp;
                actualGameTime += lastSegmentTime * currentTimeScale;
            }
            else {
                // 現在のフレーム区間を探す
                for (size_t i = 0; i < gameState.replay.currentReplay.frames.size() - 1; i++) {
                    const auto& frame1 = gameState.replay.currentReplay.frames[i];
                    const auto& frame2 = gameState.replay.currentReplay.frames[i + 1];
                    
                    if (gameState.replay.replayPlaybackTime >= frame1.timestamp && 
                        gameState.replay.replayPlaybackTime <= frame2.timestamp) {
                        float t = 0.0f;
                        if (frame2.timestamp > frame1.timestamp) {
                            t = (gameState.replay.replayPlaybackTime - frame1.timestamp) / 
                                 (frame2.timestamp - frame1.timestamp);
                        }
                        t = std::clamp(t, 0.0f, 1.0f);
                        if (frame1.timeScale == frame2.timeScale) {
                            currentTimeScale = frame1.timeScale;
                        } else {
                            currentTimeScale = (t < 0.5f) ? frame1.timeScale : frame2.timeScale;
                        }
                        
                        actualGameTime = 0.0f;
                        const auto& firstFrame = gameState.replay.currentReplay.frames[0];
                        if (firstFrame.timestamp > 0.0f && i == 0) {
                            actualGameTime = gameState.replay.replayPlaybackTime * currentTimeScale;
                        } else {
                            if (firstFrame.timestamp > 0.0f) {
                                actualGameTime += firstFrame.timestamp * firstFrame.timeScale;
                            }
                            if (i > 0) {
                                for (size_t j = 1; j < i; j++) {
                                    const auto& f1 = gameState.replay.currentReplay.frames[j];
                                    const auto& f2 = gameState.replay.currentReplay.frames[j + 1];
                                    float segmentTime = f2.timestamp - f1.timestamp;
                                    actualGameTime += segmentTime * f1.timeScale;
                                }
                            }
                            float currentSegmentTime = gameState.replay.replayPlaybackTime - frame1.timestamp;
                            actualGameTime += currentSegmentTime * currentTimeScale;
                        }
                        
                        break;
                    }
                }
            }
        }
        gameState.progress.timeScale = currentTimeScale;
        platformSystem.update(0.0f, gameState.player.position, actualGameTime);
    } else {
        platformSystem.update(scaledDeltaTime, gameState.player.position);
    }
    
    for (auto& [key, state] : keyStates) {
        state.update(glfwGetKey(window, key) == GLFW_PRESS);
    }
    
    InputHandler::handleInputProcessing(window, gameState, stageManager, platformSystem, keyStates, resetStageStartTime, scaledDeltaTime, audioManager);
    
    if (!gameState.replay.isReplayMode && !gameState.progress.isGameOver) {
        GameUpdater::updatePhysicsAndCollisions(window, gameState, stageManager, platformSystem, deltaTime, scaledDeltaTime, audioManager);
    }
    if (!gameState.replay.isReplayMode || !gameState.replay.isReplayPaused) {
        GameUpdater::updateItems(gameState, scaledDeltaTime, audioManager);
    }
    handleStageSelectionArea(window, gameState, stageManager, platformSystem, resetStageStartTime, keyStates, deltaTime);
}

void GameUpdater::updatePhysicsAndCollisions(
    GLFWwindow* window, 
    GameState& gameState, 
    StageManager& stageManager, 
    PlatformSystem& platformSystem, 
    float deltaTime, 
    float scaledDeltaTime, 
    io::AudioManager& audioManager
) {
    if (gameState.progress.isGameOver) {
        return;
    }
    
    glm::vec3 gravityDirection = glm::vec3(0, -1, 0);
    
    float gravityStrength = PhysicsUtils::calculateGravityStrength(GameConstants::BASE_GRAVITY, deltaTime, gameState.progress.timeScale, gravityDirection, gameState);
    glm::vec3 gravityForce = gravityDirection * gravityStrength;

    gameState.player.velocity += gravityForce;
    
    float airResistance = (gameState.progress.timeScale > 1.0f) ? GameConstants::AIR_RESISTANCE_FAST : GameConstants::AIR_RESISTANCE_NORMAL;
    gameState.player.velocity *= airResistance;
    
    gameState.player.position.y += gameState.player.velocity.y * scaledDeltaTime;

    glm::vec3 playerSize = GameConstants::PLAYER_SIZE;
    
    auto collisionResult = platformSystem.checkCollisionWithIndex(gameState.player.position, playerSize);
    PlatformVariant* currentPlatform = collisionResult.first;
    int currentPlatformIndex = collisionResult.second;
    
    static bool wasOnPlatform = false;
    bool isOnPlatform = (currentPlatform != nullptr);
    
    if (currentPlatform != nullptr) {
        if (gameState.progress.isEasyMode) {
            std::visit([&](const auto& platform) {
                gameState.player.lastPlatformPosition = platform.position;
                gameState.player.lastPlatformIndex = currentPlatformIndex;
                gameState.player.isTrackingPlatform = true;
            }, *currentPlatform);
        }
        
        std::visit(overloaded{
            [&](const StaticPlatform& platform) {
                if (!PhysicsSystem::checkPlatformCollision(gameState, gameState.player.position, playerSize)) {
                    PhysicsUtils::adjustPlayerPositionForGravity(gameState, platform.position, platform.size, playerSize, gravityDirection);
                }

                // ゴール処理
                if (platform.color.r > 0.9f && platform.color.g > 0.9f && platform.color.b < 0.1f) {
                    if (!gameState.progress.gameWon && gameState.items.collectedItems >= gameState.items.requiredItems) {
                        if (gameState.audioEnabled) {
                            audioManager.stopBGM();
                            gameState.bgmPlaying = false;
                            gameState.currentBGM = "";
                            
                            audioManager.playSFX("clear");
                        }
                        
                        gameState.progress.gameWon = true;
                        gameState.progress.isStageCompleted = true;
                        gameState.progress.isGoalReached = true;
                        
                        if (!gameState.replay.isReplayMode && 
                            (gameState.replay.currentReplay.frames.empty() || gameState.replay.currentReplay.clearTime <= 0.0f)) {
                            gameState.progress.clearTime = gameState.progress.gameTime;
                        }
                        
                        int currentStage = stageManager.getCurrentStage();
                        
                        if (gameState.progress.isTimeAttackMode) {
                            float clearTime = gameState.progress.currentTimeAttackTime;
                            
                            if (gameState.replay.isRecordingReplay) {
                                ReplayFrame lastFrame;
                                lastFrame.timestamp = clearTime;
                                lastFrame.playerPosition = gameState.player.position;
                                lastFrame.playerVelocity = gameState.player.velocity;
                                lastFrame.timeScale = gameState.progress.timeScale;
                                gameState.replay.replayBuffer.push_back(lastFrame);
                                
                                gameState.replay.isRecordingReplay = false; 
                            }
                            
                            gameState.progress.isNewRecord = false;
                            bool shouldSaveReplay = false;
                            if (gameState.progress.timeAttackRecords.find(currentStage) == gameState.progress.timeAttackRecords.end() ||
                                clearTime < gameState.progress.timeAttackRecords[currentStage]) {
                                gameState.progress.timeAttackRecords[currentStage] = clearTime;
                                gameState.progress.isNewRecord = true;
                                shouldSaveReplay = true;
                            }
                            
                            // 新記録の場合はローカルにリプレイを保存
                            if (shouldSaveReplay && !gameState.replay.replayBuffer.empty()) {
                                ReplayData replayData;
                                replayData.stageNumber = currentStage;
                                replayData.clearTime = clearTime;
                                replayData.frames = gameState.replay.replayBuffer;
                                replayData.frameRate = gameState.replay.REPLAY_RECORD_INTERVAL;
                                
                                auto now = std::time(nullptr);
                                std::stringstream ss;
                                ss << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S");
                                replayData.recordedDate = ss.str();
                                
                                ReplayManager::saveReplay(replayData, currentStage);
                            }
                            
                            // オンラインランキングには常にリプレイを送信
                            if (!gameState.replay.replayBuffer.empty()) {
                                ReplayData* replayData = new ReplayData();
                                replayData->stageNumber = currentStage;
                                replayData->clearTime = clearTime;
                                replayData->frames = gameState.replay.replayBuffer;
                                replayData->frameRate = gameState.replay.REPLAY_RECORD_INTERVAL;
                                
                                auto now = std::time(nullptr);
                                std::stringstream ss;
                                ss << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S");
                                replayData->recordedDate = ss.str();
                        
                                OnlineLeaderboardManager::submitTime(currentStage, clearTime, 
                                [currentStage, clearTime, replayData](bool success) {
                                    if (success) {
                                        printf("ONLINE: Record with replay submitted successfully for stage %d: %.2fs\n", currentStage, clearTime);
                                    } else {
                                        printf("ONLINE: Failed to submit record with replay for stage %d\n", currentStage);
                                    }
                                    delete replayData;
                                }, replayData);
                            }
                        } else {
                            float remainingTime = gameState.progress.timeLimit - gameState.progress.clearTime;
                            float limitTime = gameState.progress.timeLimit;
                            
                            if(limitTime >= GameConstants::LONG_TIME_THRESHOLD){
                                if (remainingTime >= GameConstants::STAR_3_TIME_LONG) {
                                    gameState.progress.earnedStars = 3;
                                } else if (remainingTime >= GameConstants::STAR_2_TIME_LONG) {
                                    gameState.progress.earnedStars = 2;
                                } else {
                                    gameState.progress.earnedStars = 1;
                                }
                            }else{
                                if (remainingTime >= GameConstants::STAR_3_TIME_SHORT) {
                                    gameState.progress.earnedStars = 3;
                                } else if (remainingTime >= GameConstants::STAR_2_TIME_SHORT) {
                                    gameState.progress.earnedStars = 2;
                                } else {
                                    gameState.progress.earnedStars = 1;
                                }
                            }
                            
                            int oldStars = (gameState.progress.stageStars.count(currentStage) > 0) ? gameState.progress.stageStars[currentStage] : 0;
                            int starDifference = gameState.progress.earnedStars - oldStars;
                            
                            if (starDifference > 0) {
                                gameState.progress.stageStars[currentStage] = gameState.progress.earnedStars;
                                gameState.progress.totalStars += starDifference;
                                SaveManager::saveGameData(gameState);
                            }
                        }
                        
                        if (gameState.progress.selectedSecretStarType != GameProgressState::SecretStarType::NONE && currentStage > 0) {
                            gameState.progress.secretStarCleared[currentStage].insert(gameState.progress.selectedSecretStarType);
                            SaveManager::saveGameData(gameState);
                        }
                        
                        if (currentStage == 5) {
                            gameState.ui.isEndingSequence = true;
                            gameState.ui.showStaffRoll = true;
                            gameState.ui.staffRollTimer = 0.0f;
                        } else {
                            gameState.ui.showStageClearUI = true;
                        }
                    }
                }
            },
            [&](const MovingPlatform& platform) {
                PhysicsUtils::adjustPlayerPositionForGravity(gameState, platform.position, platform.size, playerSize, gravityDirection);
                const_cast<MovingPlatform&>(platform).hasPlayerOnBoard = true;
                glm::vec3 platformMovement = platform.position - platform.previousPosition;
                gameState.player.position.x += platformMovement.x;
                gameState.player.position.z += platformMovement.z;
            },
            [&](const RotatingPlatform& platform) {
                glm::vec3 halfSize = platform.size * 0.5f;
                glm::vec3 platformMin = platform.position - halfSize;
                glm::vec3 platformMax = platform.position + halfSize;
                bool onPlatform = (gameState.player.position.x >= platformMin.x && gameState.player.position.x <= platformMax.x &&
                                   gameState.player.position.z >= platformMin.z && gameState.player.position.z <= platformMax.z);
                if (onPlatform) {
                    glm::vec3 localPlayerPos = gameState.player.position - platform.position;
                    if (glm::length(platform.rotationAxis - glm::vec3(0, 1, 0)) < 0.1f) {
                        float angle = glm::radians(platform.rotationSpeed * gameState.progress.gameTime);
                        float cosAngle = cos(angle);
                        float sinAngle = sin(angle);
                        float newX = localPlayerPos.x * cosAngle - localPlayerPos.z * sinAngle;
                        float newZ = localPlayerPos.x * sinAngle + localPlayerPos.z * cosAngle;
                        gameState.player.position = platform.position + glm::vec3(newX, localPlayerPos.y, newZ);
                    } else if (glm::length(platform.rotationAxis - glm::vec3(1, 0, 0)) < 0.1f) {
                        float angle = glm::radians(platform.rotationSpeed * gameState.progress.gameTime);
                        float cosAngle = cos(angle);
                        float sinAngle = sin(angle);
                        float newY = localPlayerPos.y * cosAngle - localPlayerPos.z * sinAngle;
                        float newZ = localPlayerPos.y * sinAngle + localPlayerPos.z * cosAngle;
                        gameState.player.position = platform.position + glm::vec3(localPlayerPos.x, newY, newZ);
                    }
                    PhysicsUtils::adjustPlayerPositionForGravity(gameState, platform.position, platform.size, playerSize, gravityDirection);
                } else {
                    PhysicsUtils::adjustPlayerPositionForGravity(gameState, platform.position, platform.size, playerSize, gravityDirection);
                }
            },
            [&](const PatrollingPlatform& platform) {
                glm::vec3 halfSize = platform.size * 0.5f;
                glm::vec3 platformMin = platform.position - halfSize;
                glm::vec3 platformMax = platform.position + halfSize;
                glm::vec3 extendedHalfSize = halfSize * 1.5f;
                glm::vec3 extendedMin = platform.position - extendedHalfSize;
                glm::vec3 extendedMax = platform.position + extendedHalfSize;
                bool onPlatform = (gameState.player.position.x >= platformMin.x && gameState.player.position.x <= platformMax.x &&
                                  gameState.player.position.z >= platformMin.z && gameState.player.position.z <= platformMax.z);
                bool inExtendedRange = (gameState.player.position.x >= extendedMin.x && gameState.player.position.x <= extendedMax.x &&
                                       gameState.player.position.z >= extendedMin.z && gameState.player.position.z <= extendedMax.z);
                if (onPlatform || inExtendedRange) {
                    PhysicsUtils::adjustPlayerPositionForGravity(gameState, platform.position, platform.size, playerSize, gravityDirection);
                    glm::vec3 platformMovement = platform.position - platform.previousPosition;
                    gameState.player.position.x += platformMovement.x;
                    gameState.player.position.z += platformMovement.z;
                    if (!onPlatform && inExtendedRange) {
                        glm::vec3 directionToCenter = platform.position - gameState.player.position;
                        directionToCenter.y = 0;
                        float distanceToCenter = glm::length(directionToCenter);
                        if (distanceToCenter > 0.1f) {
                            glm::vec3 normalizedDirection = glm::normalize(directionToCenter);
                            gameState.player.position += normalizedDirection * 0.5f * deltaTime;
                        }
                    }
                } else {
                    PhysicsUtils::adjustPlayerPositionForGravity(gameState, platform.position, platform.size, playerSize, gravityDirection);
                }
            },
            [&](const TeleportPlatform& platform) {
                PhysicsUtils::adjustPlayerPositionForGravity(gameState, platform.position, platform.size, playerSize, gravityDirection);
                if (!platform.hasTeleported && platform.cooldownTimer <= 0.0f) {
                    gameState.player.position = platform.teleportDestination;
                    const_cast<TeleportPlatform&>(platform).hasTeleported = true;
                    const_cast<TeleportPlatform&>(platform).cooldownTimer = 2.0f;
                }
            },
            [&](const JumpPad& platform) {
                PhysicsUtils::adjustPlayerPositionForGravity(gameState, platform.position, platform.size, playerSize, gravityDirection);
                gameState.player.velocity.y = platform.jumpPower;
            },
            [&](const CycleDisappearingPlatform& platform) {
                if (platform.isCurrentlyVisible) {
                    PhysicsUtils::adjustPlayerPositionForGravity(gameState, platform.position, platform.size, playerSize, gravityDirection);
                }
            },
            [&](const DisappearingPlatform& platform) {
                PhysicsUtils::adjustPlayerPositionForGravity(gameState, platform.position, platform.size, playerSize, gravityDirection);
            },
            [&](const FlyingPlatform& platform) {
                PhysicsUtils::adjustPlayerPositionForGravity(gameState, platform.position, platform.size, playerSize, gravityDirection);
            }
        }, *currentPlatform);
    } else {
        platformSystem.resetMovingPlatformFlags();
    }
    
    if (isOnPlatform && !wasOnPlatform && gameState.audioEnabled) {
        audioManager.playSFX("on_ground");
    }
    
    if (isOnPlatform && gameState.skills.isInBurstJumpAir) {
        gameState.skills.isInBurstJumpAir = false;
    }
    
    wasOnPlatform = isOnPlatform;
    
    if (gameState.player.position.y < 0 && !gameState.progress.isGameOver) {
        if (gameState.audioEnabled) {
            audioManager.playSFX("on_ground");
        }
        
        if (gameState.progress.isEasyMode) {
            if (gameState.player.isTrackingPlatform && gameState.player.lastPlatformIndex >= 0) {
                const auto& platforms = platformSystem.getPlatforms();
                if (gameState.player.lastPlatformIndex < platforms.size()) {
                    std::visit([&](const auto& platform) {
                        glm::vec3 currentPlatformPos = platform.position;
                        gameState.player.position = currentPlatformPos + glm::vec3(0, 2.0f, 0);
                    }, platforms[gameState.player.lastPlatformIndex]);
                } else {
                    gameState.player.position = gameState.player.lastPlatformPosition + glm::vec3(0, 2.0f, 0);
                }
            } else {
                gameState.player.position = gameState.player.lastPlatformPosition + glm::vec3(0, 2.0f, 0);
            }
            gameState.player.velocity = glm::vec3(0, 0, 0);
        } else if (stageManager.getCurrentStage() == 0) {
            gameState.player.position = glm::vec3(8, 2.0f, 0);
            gameState.player.velocity = glm::vec3(0, 0, 0);
        } else if (stageManager.getCurrentStage() == 6) {
            gameState.player.position = gameState.progress.tutorialStartPosition;
            gameState.player.velocity = glm::vec3(0, 0, 0);
        } else {
            gameState.progress.lives--;
            
            if (gameState.audioEnabled) {
                audioManager.playSFX("damage");
            }
            
            if (gameState.progress.lives <= 0) {
                gameState.progress.isGameOver = true;
                gameState.progress.gameOverTimer = 0.0f;
                if (gameState.player.lastCheckpointItemId != -1) {
                    gameState.player.position = gameState.player.lastCheckpoint;
                } else {
                    gameState.player.position = glm::vec3(0, 6.0f, -25.0f);
                }
                gameState.player.velocity = glm::vec3(0, 0, 0);
            } else {
                if (gameState.player.lastCheckpointItemId != -1) {
                    gameState.player.position = gameState.player.lastCheckpoint;
                } else {
                    gameState.player.position = glm::vec3(0, 6.0f, -25.0f);
                }
                gameState.player.velocity = glm::vec3(0, 0, 0);
            }
        }
    }
}

void GameUpdater::updateItems(
    GameState& gameState, 
    float scaledDeltaTime, 
    io::AudioManager& audioManager
) {
    for (auto& item : gameState.items.items) {
        if (!item.isCollected) {
            item.rotationAngle += scaledDeltaTime * 90.0f;
            if (item.rotationAngle >= 360.0f) {
                item.rotationAngle -= 360.0f;
            }
            
            item.bobTimer += scaledDeltaTime;
            item.bobHeight = sin(item.bobTimer * 2.0f) * 0.2f;
            
            float distance = glm::length(gameState.player.position - item.position);
            if (distance < 1.5f) { // 収集範囲
                if (gameState.audioEnabled) {
                    audioManager.playSFX("item");
                }
                
                item.isCollected = true;
                gameState.items.collectedItems++;
                
                if (gameState.progress.isTutorialStage) {
                    gameState.items.earnedItems++;
                }
                
                gameState.player.lastCheckpoint = item.position;
                gameState.player.lastCheckpointItemId = item.itemId;
            }
        }
    }
}

} // namespace GameLoop

