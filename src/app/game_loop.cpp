#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "game_loop.h"
#include "game_updater.h"
#include "game_renderer.h"
#include "input_handler.h"
#include <thread>
#include <iostream>
#include "../core/utils/ui_config_manager.h"
#include "../core/utils/resource_path.h"
#include "../game/save_manager.h"

namespace GameLoop {
    void run(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
            PlatformSystem& platformSystem,
            std::unique_ptr<gfx::OpenGLRenderer>& renderer,
            std::unique_ptr<gfx::UIRenderer>& uiRenderer,
            std::unique_ptr<gfx::GameStateUIRenderer>& gameStateUIRenderer,
            std::map<int, InputUtils::KeyState>& keyStates,
            std::function<void()> resetStageStartTime,
            std::chrono::high_resolution_clock::time_point& startTime,
            io::AudioManager& audioManager) {
        
        if (gameState.audioEnabled) {
            audioManager.loadSFX("jump", ResourcePath::getResourcePath("assets/audio/se/jump.ogg"));
            audioManager.loadSFX("clear", ResourcePath::getResourcePath("assets/audio/se/clear.ogg"));
            audioManager.loadSFX("item", ResourcePath::getResourcePath("assets/audio/se/item.ogg"));
            audioManager.loadSFX("on_ground", ResourcePath::getResourcePath("assets/audio/se/on_ground.ogg"));
            audioManager.loadSFX("flying", ResourcePath::getResourcePath("assets/audio/se/flying.ogg"));
            audioManager.loadSFX("countdown", ResourcePath::getResourcePath("assets/audio/se/countdown.ogg"));
            audioManager.loadSFX("tutorial_ok", ResourcePath::getResourcePath("assets/audio/se/tutorial_ok.ogg"));
            audioManager.loadSFX("damage", ResourcePath::getResourcePath("assets/audio/se/damage.ogg"));
        }
        
        auto lastFrameTime = startTime;
        bool gameRunning = true;

        while (!glfwWindowShouldClose(window) && gameRunning) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float>(currentTime - lastFrameTime).count();
            
            deltaTime = std::min(deltaTime, 0.1f);
            
            lastFrameTime = currentTime;
            
            if (!gameState.replay.isReplayMode) {
                gameState.progress.gameTime = std::chrono::duration<float>(currentTime - startTime).count();
            }
            
            float scaledDeltaTime = deltaTime * gameState.progress.timeScale;
            
            if (!gameState.ui.showTitleScreen) {
                int currentStage = stageManager.getCurrentStage();
                bool shouldPlayStageBGM = gameState.audioEnabled && 
                                        !gameState.progress.isGoalReached && 
                                        !gameState.ui.showTitleScreen &&
                                        !gameState.ui.isTransitioning &&
                                        gameState.ui.transitionType != UIState::TransitionType::FADE_OUT;
                
                if (shouldPlayStageBGM) {
                    std::string targetBGM = "";
                    std::string bgmPath = "";
                    
                    switch (currentStage) {
                        case 0:
                            targetBGM = "stage_select_field.ogg";
                            bgmPath = ResourcePath::getResourcePath("assets/audio/bgm/stage_select_field.ogg");
                            break;
                        case 1:
                            targetBGM = "stage1.ogg";
                            bgmPath = ResourcePath::getResourcePath("assets/audio/bgm/stage1.ogg");
                            break;
                        case 2:
                            targetBGM = "stage2.ogg";
                            bgmPath = ResourcePath::getResourcePath("assets/audio/bgm/stage2.ogg");
                            break;
                        case 3:
                            targetBGM = "stage3.ogg";
                            bgmPath = ResourcePath::getResourcePath("assets/audio/bgm/stage3.ogg");
                            break;
                        case 4:
                            targetBGM = "stage4.ogg";
                            bgmPath = ResourcePath::getResourcePath("assets/audio/bgm/stage4.ogg");
                            break;
                        case 5:
                            targetBGM = "stage5.ogg";
                            bgmPath = ResourcePath::getResourcePath("assets/audio/bgm/stage5.ogg");
                            break;
                        case 6:
                            targetBGM = "tutorial.ogg";
                            bgmPath = ResourcePath::getResourcePath("assets/audio/bgm/tutorial.ogg");
                            break;
                        default:
                            break;
                    }
                    
                    if (!targetBGM.empty()) {
                        if (gameState.currentBGM != targetBGM) {
                            if (gameState.bgmPlaying) {
                                audioManager.stopBGM();
                                gameState.bgmPlaying = false;
                            }
                            
                            if (audioManager.loadBGM(bgmPath)) {
                                audioManager.playBGM();
                                gameState.currentBGM = targetBGM;
                                gameState.bgmPlaying = true;
                                std::cout << "BGM started: " << targetBGM << std::endl;
                            }
                        }
                    } else if (gameState.bgmPlaying) {
                        audioManager.stopBGM();
                        gameState.bgmPlaying = false;
                        gameState.currentBGM = "";
                        std::cout << "BGM stopped" << std::endl;
                    }
                }
            }

            if (gameState.ui.showReadyScreen) {
                handleReadyScreen(window, gameState, stageManager, platformSystem, renderer, gameStateUIRenderer, keyStates, resetStageStartTime, audioManager, deltaTime);
                glfwPollEvents();
                continue;
            }
            
            if (gameState.ui.isCountdownActive) {
                handleCountdown(window, gameState, stageManager, platformSystem, renderer, gameStateUIRenderer, keyStates, resetStageStartTime, deltaTime);
                glfwPollEvents();
                continue;
            }

            if (gameState.ui.isEndingSequence) {
                handleEndingSequence(window, gameState, stageManager, platformSystem, renderer, gameStateUIRenderer, keyStates, deltaTime);
                glfwPollEvents();
                continue;
            }

            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                SaveManager::saveGameData(gameState);
                gameRunning = false;
            }

            if (gameState.ui.showTitleScreen) {
                if (gameState.ui.isTransitioning) {
                    const float FADE_DURATION = 0.5f;
                    gameState.ui.transitionTimer += deltaTime;
                    
                    if (gameState.ui.transitionType == UIState::TransitionType::FADE_OUT) {
                        if (gameState.ui.transitionTimer >= FADE_DURATION) {
                            if (gameState.ui.pendingStageTransition >= 0) {
                                gameState.ui.pendingFadeIn = true;
                            }
                        }
                    }
                }
                
                bool justTransitioned = false;
                if (gameState.ui.pendingFadeIn && gameState.ui.isTransitioning) {
                    gameState.ui.showTitleScreen = false;
                    justTransitioned = true;
                    
                    int targetStage = gameState.ui.pendingStageTransition;
                    if (targetStage == 6) {
                        stageManager.loadStage(6, gameState, platformSystem);
                        gameState.ui.showReadyScreen = false;
                        gameState.ui.readyScreenShown = true;
                        gameState.ui.isCountdownActive = false;
                        gameState.ui.countdownTimer = 0.0f;
                        resetStageStartTime();
                    } else if (targetStage == 0) {
                        resetStageStartTime();
                        stageManager.goToStage(0, gameState, platformSystem);
                        gameState.ui.showReadyScreen = false;
                        gameState.ui.readyScreenShown = false;
                        gameState.progress.timeScale = 1.0f;
                        gameState.progress.timeScaleLevel = 0;
                        
                        gameState.camera.isFirstPersonMode = false;
                        gameState.camera.isFirstPersonView = false;
                        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                        
                        gameState.progress.selectedSecretStarType = GameProgressState::SecretStarType::NONE;
                    }
                    gameState.ui.pendingFadeIn = false;
                    gameState.ui.isTransitioning = false;
                    gameState.ui.transitionType = UIState::TransitionType::NONE;
                    gameState.ui.transitionTimer = 0.0f;
                    gameState.ui.pendingStageTransition = -1;
                    gameState.ui.pendingReadyScreen = false;
                    gameState.ui.pendingSkipCountdown = false;
                    gameState.ui.stageLoadedForFadeIn = false;
                }
                
                gameState.ui.titleScreenTimer += deltaTime;
                
                const float BACKGROUND_FADE_DURATION = 1.5f;
                const float LOGO_ANIMATION_DURATION = 1.0f;
                
                if (gameState.ui.titleScreenPhase == UIState::TitleScreenPhase::BACKGROUND_FADE_IN) {
                    if (gameState.ui.titleScreenTimer >= BACKGROUND_FADE_DURATION) {
                        gameState.ui.titleScreenPhase = UIState::TitleScreenPhase::LOGO_ANIMATION;
                        gameState.ui.titleScreenTimer = 0.0f;
                    }
                } else if (gameState.ui.titleScreenPhase == UIState::TitleScreenPhase::LOGO_ANIMATION) {
                    if (gameState.ui.titleScreenTimer >= LOGO_ANIMATION_DURATION) {
                        gameState.ui.titleScreenPhase = UIState::TitleScreenPhase::SHOW_TEXT;
                        gameState.ui.titleScreenTimer = 0.0f;
                    }
                }
                
                bool shouldPlayBGM = gameState.audioEnabled && 
                                    !gameState.ui.isTransitioning && 
                                    !gameState.ui.pendingFadeIn &&
                                    !justTransitioned &&
                                    gameState.ui.transitionType != UIState::TransitionType::FADE_OUT &&
                                    gameState.ui.pendingStageTransition < 0;
                
                if (shouldPlayBGM) {
                    std::string titleBGM = "title.ogg";
                    if (gameState.currentBGM != titleBGM) {
                        if (gameState.bgmPlaying) {
                            audioManager.stopBGM();
                            gameState.bgmPlaying = false;
                        }
                        
                        std::string bgmPath = ResourcePath::getResourcePath("assets/audio/bgm/title.ogg");
                        if (audioManager.loadBGM(bgmPath)) {
                            audioManager.playBGM();
                            gameState.currentBGM = titleBGM;
                            gameState.bgmPlaying = true;
                            std::cout << "Title BGM started: " << titleBGM << std::endl;
                        }
                    }
                } else {
                    if (gameState.bgmPlaying && gameState.currentBGM == "title.ogg") {
                        audioManager.stopBGM();
                        gameState.bgmPlaying = false;
                        gameState.currentBGM = "";
                    }
                }
                
                for (auto& [key, state] : keyStates) {
                    state.update(glfwGetKey(window, key) == GLFW_PRESS);
                }
                
                if (gameState.ui.showPlayerNameInput) {
                    if (keyStates[GLFW_KEY_BACKSPACE].justPressed()) {
                        if (!gameState.ui.playerNameInput.empty() && gameState.ui.playerNameInputCursorPos > 0) {
                            gameState.ui.playerNameInput.erase(gameState.ui.playerNameInputCursorPos - 1, 1);
                            gameState.ui.playerNameInputCursorPos--;
                        }
                    }
                    
                    for (int key = GLFW_KEY_A; key <= GLFW_KEY_Z; key++) {
                        if (keyStates[key].justPressed()) {
                            if (gameState.ui.playerNameInput.length() < 12) {
                                char c = 'A' + (key - GLFW_KEY_A);
                                gameState.ui.playerNameInput.insert(gameState.ui.playerNameInputCursorPos, 1, c);
                                gameState.ui.playerNameInputCursorPos++;
                            }
                        }
                    }
                    
                    if (keyStates[GLFW_KEY_ENTER].justPressed()) {
                        std::string finalName = gameState.ui.playerNameInput.empty() ? "Player" : gameState.ui.playerNameInput;
                        OnlineLeaderboardManager::setPlayerName(finalName);
                        
                        SaveManager::saveGameData(gameState);
                        
                        gameState.ui.showPlayerNameInput = false;
                        
                        gameState.ui.isTransitioning = true;
                        gameState.ui.transitionType = UIState::TransitionType::FADE_OUT;
                        gameState.ui.transitionTimer = 0.0f;
                        gameState.ui.pendingStageTransition = 6;
                        gameState.ui.pendingReadyScreen = false;
                        gameState.ui.pendingSkipCountdown = true;
                    }
                } else if (keyStates[GLFW_KEY_ENTER].justPressed() && 
                    gameState.ui.titleScreenPhase == UIState::TitleScreenPhase::SHOW_TEXT &&
                    !gameState.ui.isTransitioning) {
                    if (gameState.bgmPlaying && gameState.currentBGM == "title.ogg") {
                        audioManager.stopBGM();
                        gameState.bgmPlaying = false;
                        gameState.currentBGM = "";
                    }
                    
                    bool hasClearedTutorial = (gameState.progress.stageStars.count(6) > 0 && gameState.progress.stageStars[6] > 0) ||
                                             (gameState.progress.unlockedStages.count(1) > 0 && gameState.progress.unlockedStages[1]) ||
                                             !gameState.ui.showStage0Tutorial;
                    
                    std::string currentPlayerName = OnlineLeaderboardManager::getPlayerName();
                    bool isPlayerNameSet = !currentPlayerName.empty() && currentPlayerName != "Player";
                    
                    if (!hasClearedTutorial || !isPlayerNameSet) {
                        gameState.ui.showPlayerNameInput = true;
                        if (isPlayerNameSet) {
                            gameState.ui.playerNameInput = currentPlayerName;
                            gameState.ui.playerNameInputCursorPos = currentPlayerName.length();
                        } else {
                            gameState.ui.playerNameInput = "";
                            gameState.ui.playerNameInputCursorPos = 0;
                        }
                    } else {
                        gameState.ui.isTransitioning = true;
                        gameState.ui.transitionType = UIState::TransitionType::FADE_OUT;
                        gameState.ui.transitionTimer = 0.0f;
                        gameState.ui.pendingStageTransition = 0;
                        gameState.ui.pendingReadyScreen = false;
                        gameState.ui.pendingSkipCountdown = true;
                    }
                }
                
                GameRenderer::renderFrame(window, gameState, stageManager, platformSystem, renderer, uiRenderer, gameStateUIRenderer, keyStates, deltaTime);
                
                glfwPollEvents();
                if (gameState.ui.showTitleScreen) {
                    continue;
                }
            }

            GameUpdater::updateGameState(window, gameState, stageManager, platformSystem, deltaTime, scaledDeltaTime, keyStates, resetStageStartTime, audioManager);
            GameRenderer::renderFrame(window, gameState, stageManager, platformSystem, renderer, uiRenderer, gameStateUIRenderer, keyStates, deltaTime);
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
            glfwPollEvents();
        }
    }

    void handleReadyScreen(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                          PlatformSystem& platformSystem,
                          std::unique_ptr<gfx::OpenGLRenderer>& renderer,
                          std::unique_ptr<gfx::GameStateUIRenderer>& gameStateUIRenderer,
                          std::map<int, InputUtils::KeyState>& keyStates,
                          std::function<void()> resetStageStartTime,
                          io::AudioManager& audioManager,
                          float deltaTime) {
        static float readyScreenFileCheckTimer = 0.0f;
        readyScreenFileCheckTimer += deltaTime;
        if (readyScreenFileCheckTimer >= 0.5f) {
            readyScreenFileCheckTimer = 0.0f;
            UIConfig::UIConfigManager::getInstance().checkAndReloadConfig();
        }
        
        // Immersiveモードの場合は、Ready画面でもFPS視点を維持
        if (gameState.progress.selectedSecretStarType == GameProgressState::SecretStarType::IMMERSIVE_STAR) {
            if (!gameState.camera.isFirstPersonView || !gameState.camera.isFirstPersonMode) {
                gameState.camera.isFirstPersonMode = true;
                gameState.camera.isFirstPersonView = true;
                gameState.camera.yaw = 90.0f;
                gameState.camera.pitch = -10.0f;
                gameState.camera.firstMouse = true;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
        }
        
        int width, height;
        GameRenderer::prepareFrame(window, gameState, stageManager, renderer, width, height, deltaTime);
        
        GameRenderer::renderPlatforms(platformSystem, renderer, gameState, stageManager);
        
        GameRenderer::renderPlayer(gameState, renderer);
        
        gameStateUIRenderer->renderReadyScreen(width, height, gameState.ui.readyScreenSpeedLevel, gameState.camera.isFirstPersonMode);
        
        renderer->endFrame();
        
        for (auto& [key, state] : keyStates) {
            state.update(glfwGetKey(window, key) == GLFW_PRESS);
        }
        
        if (keyStates[GLFW_KEY_T].justPressed()) {
            gameState.ui.readyScreenSpeedLevel = (gameState.ui.readyScreenSpeedLevel + 1) % 3;
        }
        
        
        if (keyStates[GLFW_KEY_ENTER].justPressed()) {
            gameState.ui.showReadyScreen = false;
            gameState.ui.isCountdownActive = true;
            gameState.ui.countdownTimer = 3.0f;
            
            if (gameState.audioEnabled) {
                audioManager.playSFX("countdown");
            }
            
            gameState.camera.isFirstPersonView = gameState.camera.isFirstPersonMode;
            
            if (gameState.camera.isFirstPersonMode) {
                gameState.camera.yaw = 90.0f;
                gameState.camera.pitch = -10.0f;
                gameState.camera.firstMouse = true;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            
            switch (gameState.ui.readyScreenSpeedLevel) {
                case 0:
                    gameState.progress.timeScale = 1.0f;
                    gameState.progress.timeScaleLevel = 0;
                    break;
                case 1:
                    gameState.progress.timeScale = 2.0f;
                    gameState.progress.timeScaleLevel = 1;
                    break;
                case 2:
                    gameState.progress.timeScale = 3.0f;
                    gameState.progress.timeScaleLevel = 2;
                    break;
            }
        }
    }

    void handleCountdown(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                        PlatformSystem& platformSystem,
                        std::unique_ptr<gfx::OpenGLRenderer>& renderer,
                        std::unique_ptr<gfx::GameStateUIRenderer>& gameStateUIRenderer,
                        std::map<int, InputUtils::KeyState>& keyStates,
                        std::function<void()> resetStageStartTime,
                        float deltaTime) {
        
        if (!gameState.progress.timeLimitApplied) {
            float baseTimeLimit = gameState.progress.timeLimit;
            
            if (gameState.camera.isFirstPersonMode) {
                baseTimeLimit += GameConstants::FIRST_PERSON_TIME_BONUS;
            }
            if (gameState.progress.isEasyMode) {
                baseTimeLimit += GameConstants::EASY_MODE_TIME_BONUS;
            }
            
            gameState.progress.timeLimit = baseTimeLimit;
            gameState.progress.remainingTime = baseTimeLimit;
            gameState.progress.timeLimitApplied = true;
        }
        
        int width, height;
        GameRenderer::prepareFrame(window, gameState, stageManager, renderer, width, height, deltaTime);
        
        GameRenderer::renderPlatforms(platformSystem, renderer, gameState, stageManager);
        
        GameRenderer::renderPlayer(gameState, renderer);
        
        int count = (int)gameState.ui.countdownTimer + 1;
        if (count > 0) {
            gameStateUIRenderer->renderCountdown(width, height, count);
        }
        
        renderer->endFrame();
        
        for (auto& [key, state] : keyStates) {
            state.update(glfwGetKey(window, key) == GLFW_PRESS);
        }
        
        gameState.ui.countdownTimer -= deltaTime;
        
        if (gameState.ui.countdownTimer <= 0.0f) {
            gameState.ui.isCountdownActive = false;
            resetStageStartTime();
            
            if (gameState.progress.selectedSecretStarType == GameProgressState::SecretStarType::MAX_SPEED_STAR) {
                gameState.progress.timeScale = 3.0f;
                gameState.progress.timeScaleLevel = 2;  // 3倍に設定
            }
            
            if (gameState.progress.selectedSecretStarType == GameProgressState::SecretStarType::IMMERSIVE_STAR) {
                gameState.camera.isFirstPersonView = true;
                gameState.camera.yaw = 90.0f;
                gameState.camera.pitch = -10.0f;
                gameState.camera.firstMouse = true;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
            
            if (gameState.progress.isTimeAttackMode) {
                gameState.progress.timeAttackStartTime = gameState.progress.gameTime;
                gameState.progress.currentTimeAttackTime = 0.0f;
                
                gameState.replay.isRecordingReplay = true;
                gameState.replay.replayBuffer.clear();
                gameState.replay.replayRecordTimer = 0.0f;
                
                ReplayFrame firstFrame;
                firstFrame.timestamp = 0.0f;
                firstFrame.playerPosition = gameState.player.position;
                firstFrame.playerVelocity = gameState.player.velocity;
                firstFrame.timeScale = gameState.progress.timeScale;
                firstFrame.itemCollectedStates.clear();
                for (const auto& item : gameState.items.items) {
                    firstFrame.itemCollectedStates.push_back(item.isCollected);
                }
                gameState.replay.replayBuffer.push_back(firstFrame);
            }
        }
    }

    void handleEndingSequence(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                             PlatformSystem& platformSystem,
                             std::unique_ptr<gfx::OpenGLRenderer>& renderer,
                             std::unique_ptr<gfx::GameStateUIRenderer>& gameStateUIRenderer,
                             std::map<int, InputUtils::KeyState>& keyStates,
                             float deltaTime) {
        int width, height;
        GameRenderer::prepareFrame(window, gameState, stageManager, renderer, width, height, deltaTime);
        
        if (gameState.ui.showStaffRoll) {
            gameState.ui.staffRollTimer += deltaTime;
            
            if (gameState.ui.staffRollTimer >= 14.0f) {
                gameState.ui.showStaffRoll = false;
                gameState.ui.showEndingMessage = true;
                gameState.ui.endingMessageTimer = 0.0f;
            } else {
                gameStateUIRenderer->renderStaffRoll(width, height, gameState.ui.staffRollTimer);
            }
        }
        
        if (gameState.ui.showEndingMessage) {
            gameState.ui.endingMessageTimer += deltaTime;
            
            if (gameState.ui.endingMessageTimer >= 5.0f) {
                gameState.ui.isEndingSequence = false;
                gameState.ui.showStaffRoll = false;
                gameState.ui.showEndingMessage = false;
                gameState.ui.staffRollTimer = 0.0f;
                gameState.ui.endingMessageTimer = 0.0f;
                
                gameState.progress.isGameCleared = true;
                if (!gameState.progress.hasShownSecretStarExplanationUI) {
                    gameState.ui.showSecretStarExplanationUI = true;
                    gameState.progress.hasShownSecretStarExplanationUI = true;
                }
                
                InputHandler::returnToField(window, gameState, stageManager, platformSystem, -1);
            } else {
                gameStateUIRenderer->renderEndingMessage(width, height, gameState.ui.endingMessageTimer);
            }
        }
        
        if (keyStates[GLFW_KEY_ENTER].justPressed()) {
            gameState.ui.isEndingSequence = false;
            gameState.ui.showStaffRoll = false;
            gameState.ui.showEndingMessage = false;
            gameState.ui.staffRollTimer = 0.0f;
            gameState.ui.endingMessageTimer = 0.0f;
            
            gameState.progress.isGameCleared = true;
            if (!gameState.progress.hasShownSecretStarExplanationUI) {
                gameState.ui.showSecretStarExplanationUI = true;
                gameState.progress.hasShownSecretStarExplanationUI = true;
            }
            
            InputHandler::returnToField(window, gameState, stageManager, platformSystem, -1);
        }
        
        renderer->endFrame();
        
        for (auto& [key, state] : keyStates) {
            state.update(glfwGetKey(window, key) == GLFW_PRESS);
        }
    }

    void handleStageSelectionArea(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                                 PlatformSystem& platformSystem, std::function<void()> resetStageStartTime,
                                 std::map<int, InputUtils::KeyState>& keyStates, float deltaTime) {
        if (stageManager.getCurrentStage() == 0) {
            int selectedStage = -1;
            for (int stage = 0; stage < 5; stage++) {
                const auto& stageArea = GameConstants::STAGE_AREAS[stage];
                if (gameState.player.position.x > stageArea.x - GameConstants::STAGE_SELECTION_RANGE && 
                    gameState.player.position.x < stageArea.x + GameConstants::STAGE_SELECTION_RANGE && 
                    gameState.player.position.z > stageArea.z - GameConstants::STAGE_SELECTION_RANGE && 
                    gameState.player.position.z < stageArea.z + GameConstants::STAGE_SELECTION_RANGE
                ) {
                    selectedStage = stage + 1;
                    break;
                }
            }
            
            if (selectedStage > 0) {
                gameState.ui.showStageSelectionAssist = true;
                gameState.ui.assistTargetStage = selectedStage;
            } else {
                gameState.ui.showStageSelectionAssist = false;
                gameState.ui.assistTargetStage = 0;
            }
            
            if (!gameState.ui.showWarpTutorialUI && glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
                if (selectedStage > 0) {
                    InputHandler::processStageSelectionAction(selectedStage, gameState, stageManager, platformSystem, resetStageStartTime, window);
                }
            }
            
            // ランキングボードの検出（JSONから読み込んだ位置を使用）
            const glm::vec3& leaderboardPos = gameState.ui.leaderboardPosition;
            bool isNearLeaderboard = 
                gameState.player.position.x > leaderboardPos.x - GameConstants::LEADERBOARD_SELECTION_RANGE &&
                gameState.player.position.x < leaderboardPos.x + GameConstants::LEADERBOARD_SELECTION_RANGE &&
                gameState.player.position.z > leaderboardPos.z - GameConstants::LEADERBOARD_SELECTION_RANGE &&
                gameState.player.position.z < leaderboardPos.z + GameConstants::LEADERBOARD_SELECTION_RANGE;
            
            if (isNearLeaderboard) {
                gameState.ui.showLeaderboardAssist = true;
            } else {
                gameState.ui.showLeaderboardAssist = false;
            }
            
            // リーダーボードUI表示中の処理
            if (gameState.ui.showLeaderboardUI) {
                // ローディング中でエラーが発生した場合の自動リトライ
                if (gameState.ui.isLoadingLeaderboard && gameState.ui.leaderboardEntries.empty()) {
                    const float RETRY_INTERVAL = 2.0f;  // 2秒ごとにリトライ
                    const float TIMEOUT_SECONDS = 10.0f;  // 10秒でタイムアウト
                    
                    gameState.ui.leaderboardRetryTimer += deltaTime;
                    
                    // 10秒経過したらタイムアウト
                    if (gameState.ui.leaderboardRetryTimer >= TIMEOUT_SECONDS) {
                        gameState.ui.isLoadingLeaderboard = false;
                        gameState.ui.leaderboardRetryTimer = 0.0f;
                        gameState.ui.leaderboardRetryCount = 0;
                    } else if (gameState.ui.leaderboardRetryTimer >= RETRY_INTERVAL) {
                        // 2秒ごとにリトライ（最大5回まで）
                        const int MAX_RETRY_COUNT = 5;
                        if (gameState.ui.leaderboardRetryCount < MAX_RETRY_COUNT) {
                            gameState.ui.leaderboardRetryTimer = 0.0f;
                            gameState.ui.leaderboardRetryCount++;                
                            
                            // リトライ
                            OnlineLeaderboardManager::fetchLeaderboard(
                                gameState.ui.leaderboardTargetStage, 
                                [&gameState](const std::vector<LeaderboardEntry>& entries) {
                                    if (!entries.empty()) { 
                                        gameState.ui.leaderboardEntries = entries;
                                        gameState.ui.isLoadingLeaderboard = false;
                                        gameState.ui.leaderboardRetryCount = 0;
                                        gameState.ui.leaderboardRetryTimer = 0.0f;
                                    }
                                }
                            );
                        }
                    }
                }
                
                // A/Dキーでステージ切り替え
                if (keyStates[GLFW_KEY_A].justPressed() || keyStates[GLFW_KEY_LEFT].justPressed()) {
                    int currentStage = gameState.ui.leaderboardTargetStage;
                    int newStage = currentStage - 1;
                    if (newStage < 1) {
                        newStage = 5;  // 1-5の範囲でループ
                    }
                    gameState.ui.leaderboardTargetStage = newStage;
                    gameState.ui.isLoadingLeaderboard = true;
                    gameState.ui.leaderboardEntries.clear();
                    gameState.ui.leaderboardRetryCount = 0;
                    gameState.ui.leaderboardRetryTimer = 0.0f;
                    
                    // 新しいステージのランキングを取得
                    OnlineLeaderboardManager::fetchLeaderboard(newStage, [&gameState, newStage](const std::vector<LeaderboardEntry>& entries) {
                        if (!entries.empty()) {
                            gameState.ui.leaderboardEntries = entries;
                            gameState.ui.isLoadingLeaderboard = false;
                            gameState.ui.leaderboardRetryCount = 0;
                            gameState.ui.leaderboardRetryTimer = 0.0f;
                        }
                    });
                }
                
                if (keyStates[GLFW_KEY_D].justPressed() || keyStates[GLFW_KEY_RIGHT].justPressed()) {
                    int currentStage = gameState.ui.leaderboardTargetStage;
                    int newStage = currentStage + 1;
                    if (newStage > 5) {
                        newStage = 1;  // 1-5の範囲でループ
                    }
                    gameState.ui.leaderboardTargetStage = newStage;
                    gameState.ui.isLoadingLeaderboard = true;
                    gameState.ui.leaderboardEntries.clear();
                    gameState.ui.leaderboardRetryCount = 0;
                    gameState.ui.leaderboardRetryTimer = 0.0f;
                    
                    // 新しいステージのランキングを取得
                    OnlineLeaderboardManager::fetchLeaderboard(newStage, [&gameState, newStage](const std::vector<LeaderboardEntry>& entries) {
                        if (!entries.empty()) {
                            gameState.ui.leaderboardEntries = entries;
                            gameState.ui.isLoadingLeaderboard = false;
                            gameState.ui.leaderboardRetryCount = 0;
                            gameState.ui.leaderboardRetryTimer = 0.0f;
                        }
                    });
                }
                
                // W/Sキーでエントリ選択
                if (keyStates[GLFW_KEY_W].justPressed() || keyStates[GLFW_KEY_UP].justPressed()) {
                    if (!gameState.ui.leaderboardEntries.empty()) {
                        gameState.ui.leaderboardSelectedIndex--;
                        if (gameState.ui.leaderboardSelectedIndex < 0) {
                            gameState.ui.leaderboardSelectedIndex = static_cast<int>(gameState.ui.leaderboardEntries.size()) - 1;
                        }
                    }
                }
                
                if (keyStates[GLFW_KEY_S].justPressed() || keyStates[GLFW_KEY_DOWN].justPressed()) {
                    if (!gameState.ui.leaderboardEntries.empty()) {
                        gameState.ui.leaderboardSelectedIndex++;
                        if (gameState.ui.leaderboardSelectedIndex >= static_cast<int>(gameState.ui.leaderboardEntries.size())) {
                            gameState.ui.leaderboardSelectedIndex = 0;
                        }
                    }
                }
                
                // Spaceキーでリプレイを視聴
                if (keyStates[GLFW_KEY_SPACE].justPressed()) {
                    if (gameState.ui.leaderboardEntries.empty()) {
                        printf("ONLINE: No entries available\n");
                    } else {
                        int selectedIndex = gameState.ui.leaderboardSelectedIndex;
                        if (selectedIndex >= 0 && selectedIndex < static_cast<int>(gameState.ui.leaderboardEntries.size())) {
                            const auto& selectedEntry = gameState.ui.leaderboardEntries[selectedIndex];
                            if (selectedEntry.hasReplay && selectedEntry.id > 0) {
                                gameState.ui.isLoadingReplay = true;
                                
                                // リプレイを取得（コールバック内でステージを読み込む）
                                OnlineLeaderboardManager::fetchReplay(selectedEntry.id, [&gameState](const ReplayData* replayData) {
                                    if (replayData != nullptr) {
                                        // リプレイデータをコピーして保存
                                        gameState.replay.currentReplay = *replayData;
                                        gameState.replay.pendingReplayStage = replayData->stageNumber;
                                        gameState.replay.pendingReplayLoad = true;
                                        gameState.replay.isOnlineReplay = true;  // オンラインリプレイフラグを設定
                                        
                                        // ランキングUIを閉じる
                                        gameState.ui.showLeaderboardUI = false;
                                        gameState.ui.leaderboardEntries.clear();
                                        gameState.ui.leaderboardTargetStage = 0;
                                        gameState.ui.leaderboardSelectedIndex = 0;
                                    }
                                    gameState.ui.isLoadingReplay = false;
                                });
                            }
                        }
                    }
                }
                
                // Enterキーで閉じる
                if (keyStates[GLFW_KEY_ENTER].justPressed()) {
                    gameState.ui.showLeaderboardUI = false;
                    gameState.ui.leaderboardEntries.clear();
                    gameState.ui.leaderboardTargetStage = 0;
                    gameState.ui.leaderboardSelectedIndex = 0;
                    gameState.ui.isLoadingLeaderboard = false;
                    gameState.ui.leaderboardRetryCount = 0;
                    gameState.ui.leaderboardRetryTimer = 0.0f;
                }
            } else if (!gameState.ui.showWarpTutorialUI && 
                !gameState.ui.showUnlockConfirmUI &&
                !gameState.ui.showStarInsufficientUI &&
                !gameState.ui.showStageSelectionAssist &&
                keyStates[GLFW_KEY_ENTER].justPressed()) {
                // ランキングボードでEnterキーを押したときの処理
                if (isNearLeaderboard && !gameState.ui.showLeaderboardUI && !gameState.ui.isLoadingLeaderboard) {
                    // ステージ選択UIを表示して、どのステージのランキングを見るか選択させる
                    // まずは現在選択中のステージ（または最後にクリアしたステージ）のランキングを表示
                    int targetStage = gameState.ui.assistTargetStage > 0 ? gameState.ui.assistTargetStage : 1;
                    gameState.ui.leaderboardTargetStage = targetStage;
                    gameState.ui.isLoadingLeaderboard = true;
                    gameState.ui.showLeaderboardUI = true;
                    gameState.ui.leaderboardEntries.clear();
                    gameState.ui.leaderboardSelectedIndex = 0;
                    gameState.ui.leaderboardRetryCount = 0;
                    gameState.ui.leaderboardRetryTimer = 0.0f;
                    
                    // ランキングを取得
                    OnlineLeaderboardManager::fetchLeaderboard(targetStage, [&gameState, targetStage](const std::vector<LeaderboardEntry>& entries) {
                        if (!entries.empty()) {
                            gameState.ui.leaderboardEntries = entries;
                            gameState.ui.isLoadingLeaderboard = false;
                            gameState.ui.leaderboardRetryCount = 0;
                            gameState.ui.leaderboardRetryTimer = 0.0f;
                        }
                    });
                }
            }
        }
    }
} // namespace GameLoop
