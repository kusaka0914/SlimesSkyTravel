/**
 * @file ui_state.h
 * @brief UIの状態を管理する構造体
 * @details タイトル画面、ステージクリア、ステージ選択、チュートリアル、エンディングなどのUI表示状態を保持します。
 */
#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "../../game/online_leaderboard_manager.h"

/**
 * @brief UIの状態を管理する構造体
 * @details タイトル画面、ステージクリア、ステージ選択、チュートリアル、エンディングなどのUI表示状態を保持します。
 */
struct UIState {
    bool showStageClearUI = false;
    float stageClearTimer = 0.0f;
    bool stageClearConfirmed = false;
    
    bool showUnlockConfirmUI = false;
    int unlockTargetStage = 0;
    int unlockRequiredStars = 0;
    
    bool showStarInsufficientUI = false;
    int insufficientTargetStage = 0;
    int insufficientRequiredStars = 0;
    
    bool showWarpTutorialUI = false;
    int warpTutorialStage = 0;
    bool blockEnterUntilReleased = false;
    
    bool showStageSelectionAssist = false;
    int assistTargetStage = 0;
    
    bool showLeaderboardAssist = false;
    bool showLeaderboardUI = false;
    int leaderboardTargetStage = 0;
    std::vector<LeaderboardEntry> leaderboardEntries;
    int leaderboardSelectedIndex = 0;
    bool isLoadingLeaderboard = false;
    bool isLoadingReplay = false;
    float leaderboardRetryTimer = 0.0f;
    int leaderboardRetryCount = 0;
    glm::vec3 leaderboardPosition = glm::vec3(0.0f, 1.0f, 0.0f);
    
    bool showStage0Tutorial = true;
    
    bool showTitleScreen = true;
    float titleScreenTimer = 0.0f;
    enum class TitleScreenPhase {
        BACKGROUND_FADE_IN,
        LOGO_ANIMATION,
        SHOW_TEXT
    };
    TitleScreenPhase titleScreenPhase = TitleScreenPhase::BACKGROUND_FADE_IN;
    
    bool showPlayerNameInput = false;
    std::string playerNameInput = "";
    int playerNameInputCursorPos = 0;
    
    bool isTransitioning = false;
    float transitionTimer = 0.0f;
    enum class TransitionType {
        NONE,
        FADE_OUT,
        FADE_IN
    };
    TransitionType transitionType = TransitionType::NONE;
    int pendingStageTransition = -1;
    bool pendingReadyScreen = false;
    bool pendingSkipCountdown = false;
    bool pendingFadeIn = false;
    bool stageLoadedForFadeIn = false;
    
    bool showEasyModeExplanationUI = false;
    
    bool showModeSelectionUI = false;
    bool showTimeAttackSelectionUI = false;
    int modeSelectionTargetStage = 0;
    
    bool showSecretStarExplanationUI = false;
    bool showSecretStarSelectionUI = false;
    
    bool showReadyScreen = false;
    bool readyScreenShown = false;
    int readyScreenSpeedLevel = 0;
    bool isCountdownActive = false;
    float countdownTimer = 3.0f;
    
    bool showTutorialUI = false;
    std::string tutorialMessage = "";
    
    bool showStaffRoll = false;
    bool showEndingMessage = false;
    float staffRollTimer = 0.0f;
    float endingMessageTimer = 0.0f;
    bool isEndingSequence = false;
};

