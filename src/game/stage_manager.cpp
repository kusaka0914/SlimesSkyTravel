#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "stage_manager.h"
#include "json_stage_loader.h"
#include "../core/constants/game_constants.h"
#include "../core/utils/resource_path.h"
#ifdef _WIN32
    #include <sys/stat.h>
    #include <io.h>
#else
    #include <sys/stat.h>
    #include <unistd.h>
#endif

StageManager::StageManager() : currentStage(1), lastFileModificationTime(0) {
    initializeStages();
    
    stageFilePaths[0] = "../assets/stages/stage_selection.json";
    stageFilePaths[1] = "../assets/stages/stage1.json";
    stageFilePaths[2] = "../assets/stages/stage2.json";
    stageFilePaths[3] = "../assets/stages/stage3.json";
    stageFilePaths[4] = "../assets/stages/stage4.json";
    stageFilePaths[5] = "../assets/stages/stage5.json";
    stageFilePaths[6] = "../assets/stages/tutorial.json";
}

void StageManager::initializeStages() {
    stages.clear();
    
    stages.push_back({
        0,
        generateStageSelectionField,
        true,
        false
    });
    
    stages.push_back({
        1,
        generateStage1,
        true,
        false
    });
    
    stages.push_back({
        2,
        generateStage2,
        true,
        false
    });
    
    stages.push_back({
        3,
        generateStage3,
        true,
        false
    });
    
    stages.push_back({
        4,
        generateStage4,
        true,
        false
    });
    
    stages.push_back({
        5,
        generateStage5,
        true,
        false
    });

    stages.push_back({
        6,
        generateTutorialStage,
        true,
        false
    });
}

bool StageManager::loadStage(int stageNumber, GameState& gameState, PlatformSystem& platformSystem) {
    if (stageNumber < 0 || stageNumber >= static_cast<int>(stages.size())) {
        return false;
    }
    
    auto stageIt = std::find_if(stages.begin(), stages.end(),
        [stageNumber](const StageData& stage) { return stage.stageNumber == stageNumber; });
    
    if (stageIt == stages.end()) {
        return false;
    }
    
    if (!stageIt->isUnlocked) {
        return false;
    }
    
    gameState.platforms.clear();
    gameState.items.items.clear();
    gameState.items.collectedItems = 0;
    gameState.progress.gameWon = false;
    
    gameState.progress.remainingTime = gameState.progress.timeLimit;
    gameState.progress.earnedStars = 0;
    
    if (!gameState.replay.isReplayMode) {
        gameState.progress.clearTime = 0.0f;
    }
    gameState.progress.isTimeUp = false;
    gameState.progress.isStageCompleted = false;
    gameState.ui.showStageClearUI = false;
    gameState.ui.stageClearTimer = 0.0f;
    gameState.ui.stageClearConfirmed = false;
    gameState.progress.isGoalReached = false;
    gameState.progress.isGameOver = false;
    gameState.progress.isTimeUp = false;
    gameState.ui.readyScreenShown = false;
    
    gameState.skills.doubleJumpRemainingUses = gameState.skills.doubleJumpMaxUses;
    gameState.skills.heartFeelRemainingUses = gameState.skills.heartFeelMaxUses;
    gameState.skills.freeCameraRemainingUses = gameState.skills.freeCameraMaxUses;
    gameState.skills.burstJumpRemainingUses = gameState.skills.burstJumpMaxUses;
    gameState.skills.timeStopRemainingUses = gameState.skills.timeStopMaxUses;

    gameState.camera.isFirstPersonMode = false;
    gameState.camera.isFirstPersonView = false;        
    
    gameState.skills.isFreeCameraActive = false;
    gameState.skills.freeCameraTimer = 0.0f;
    gameState.skills.isBurstJumpActive = false;
    gameState.skills.hasUsedBurstJump = false;
    gameState.skills.isInBurstJumpAir = false;
    gameState.skills.burstJumpDelayTimer = 0.0f;
    gameState.skills.isTimeStopped = false;
    gameState.skills.timeStopTimer = 0.0f;
    
    gameState.progress.isTutorialStage = false;
    gameState.progress.tutorialStep = 0;
    gameState.progress.tutorialStepCompleted = false;
    gameState.ui.showTutorialUI = false;
    
    if (stageNumber == 6) {
        gameState.progress.selectedSecretStarType = GameProgressState::SecretStarType::NONE;
    }
    
    gameState.player.lastCheckpointItemId = -1;
    
    platformSystem.clear();
    
    gameState.progress.currentStage = stageNumber;
    
    stageIt->generateFunction(gameState, platformSystem);
    
    if (stageNumber != 0 && stageNumber != 6 && gameState.progress.selectedSecretStarType == GameProgressState::SecretStarType::MAX_SPEED_STAR) {
        gameState.progress.timeScale = 3.0f;
        gameState.progress.timeScaleLevel = 2;
    }
    
    updateCurrentStageFileInfo(stageNumber);
    
    currentStage = stageNumber;
    return true;
}

void StageManager::completeStage(int stageNumber) {
    auto stageIt = std::find_if(stages.begin(), stages.end(),
        [stageNumber](const StageData& stage) { return stage.stageNumber == stageNumber; });
    
    if (stageIt != stages.end()) {
        stageIt->isCompleted = true;
    }
}

bool StageManager::isStageUnlocked(int stageNumber) const {
    auto stageIt = std::find_if(stages.begin(), stages.end(),
        [stageNumber](const StageData& stage) { return stage.stageNumber == stageNumber; });
    
    return stageIt != stages.end() && stageIt->isUnlocked;
}

bool StageManager::isStageCompleted(int stageNumber) const {
    auto stageIt = std::find_if(stages.begin(), stages.end(),
        [stageNumber](const StageData& stage) { return stage.stageNumber == stageNumber; });
    
    return stageIt != stages.end() && stageIt->isCompleted;
}

const StageData* StageManager::getStageData(int stageNumber) const {
    auto stageIt = std::find_if(stages.begin(), stages.end(),
        [stageNumber](const StageData& stage) { return stage.stageNumber == stageNumber; });
    
    return stageIt != stages.end() ? &(*stageIt) : nullptr;
}

bool StageManager::goToStage(int stageNumber, GameState& gameState, PlatformSystem& platformSystem) {
    if (stageNumber < 0 || stageNumber >= static_cast<int>(stages.size())) {
        return false;
    }
    
    return loadStage(stageNumber, gameState, platformSystem);
}


void StageManager::generateStage1(GameState& gameState, PlatformSystem& platformSystem) {
    if (!JsonStageLoader::loadStageFromJSON(ResourcePath::getResourcePath("assets/stages/stage1.json"), gameState, platformSystem)) {
        return;
    }
}

void StageManager::generateStage2(GameState& gameState, PlatformSystem& platformSystem) {
    if (!JsonStageLoader::loadStageFromJSON(ResourcePath::getResourcePath("assets/stages/stage2.json"), gameState, platformSystem)) {
        return;
    }
}

void StageManager::generateStage3(GameState& gameState, PlatformSystem& platformSystem) {
    if (!JsonStageLoader::loadStageFromJSON(ResourcePath::getResourcePath("assets/stages/stage3.json"), gameState, platformSystem)) {
        return;
    }
}

void StageManager::generateStage4(GameState& gameState, PlatformSystem& platformSystem) {
    if (!JsonStageLoader::loadStageFromJSON(ResourcePath::getResourcePath("assets/stages/stage4.json"), gameState, platformSystem)) {
        return;
    }
}

void StageManager::generateStage5(GameState& gameState, PlatformSystem& platformSystem) {
    if (!JsonStageLoader::loadStageFromJSON(ResourcePath::getResourcePath("assets/stages/stage5.json"), gameState, platformSystem)) {
        return;
    }
}

void StageManager::generateStageSelectionField(GameState& gameState, PlatformSystem& platformSystem) {
    platformSystem.clear();
    
    if (!JsonStageLoader::loadStageFromJSON(ResourcePath::getResourcePath("assets/stages/stage_selection.json"), gameState, platformSystem)) {
        return;
    }
}

void StageManager::generateTutorialStage(GameState& gameState, PlatformSystem& platformSystem) {

    gameState.progress.isTutorialStage = true;
    gameState.progress.tutorialStep = 0;
    gameState.progress.tutorialStepCompleted = false;
    gameState.progress.tutorialStartPosition = gameState.player.position;
    gameState.progress.tutorialRequiredDistance = 5.0f;  // 移動距離を小さくする
    gameState.ui.showTutorialUI = true;
    gameState.progress.tutorialInputEnabled = true;
    gameState.ui.tutorialMessage = "MOVING FORWARD: PRESS W";
    
    gameState.progress.selectedSecretStarType = GameProgressState::SecretStarType::NONE;
    
    gameState.items.earnedItems = 0;
    gameState.items.totalItems = 3;

    if (!JsonStageLoader::loadStageFromJSON(ResourcePath::getResourcePath("assets/stages/tutorial.json"), gameState, platformSystem)) {
        return;
    }
    
    gameState.progress.tutorialStartPosition = gameState.player.position;
}


std::time_t StageManager::getFileModificationTime(const std::string& filepath) {
#ifdef _WIN32
    struct _stat fileInfo;
    if (_stat(filepath.c_str(), &fileInfo) == 0) {
        return fileInfo.st_mtime;
    }
    std::string altPath = filepath;
    if (altPath.find("../") == 0) {
        altPath = altPath.substr(3);
    } else if (altPath.find("assets/") == 0) {
        altPath = "../" + altPath;
    }
    if (_stat(altPath.c_str(), &fileInfo) == 0) {
        return fileInfo.st_mtime;
    }
#else
    struct stat fileInfo;
    if (stat(filepath.c_str(), &fileInfo) == 0) {
        return fileInfo.st_mtime;
    }
    std::string altPath = filepath;
    if (altPath.find("../") == 0) {
        altPath = altPath.substr(3);
    } else if (altPath.find("assets/") == 0) {
        altPath = "../" + altPath;
    }
    if (stat(altPath.c_str(), &fileInfo) == 0) {
        return fileInfo.st_mtime;
    }
#endif
    return 0;
}

std::string StageManager::getStageFilePath(int stageNumber) {
    auto it = stageFilePaths.find(stageNumber);
    if (it != stageFilePaths.end()) {
        std::ifstream file(it->second);
        if (file.good()) {
            file.close();
            return it->second;
        }
        file.close();
    }
    
    std::vector<std::string> alternativePaths;
    switch (stageNumber) {
        case 0: alternativePaths = {"assets/stages/stage_selection.json", "../assets/stages/stage_selection.json"}; break;
        case 1: alternativePaths = {"assets/stages/stage1.json", "../assets/stages/stage1.json"}; break;
        case 2: alternativePaths = {"assets/stages/stage2.json", "../assets/stages/stage2.json"}; break;
        case 3: alternativePaths = {"assets/stages/stage3.json", "../assets/stages/stage3.json"}; break;
        case 4: alternativePaths = {"assets/stages/stage4.json", "../assets/stages/stage4.json"}; break;
        case 5: alternativePaths = {"assets/stages/stage5.json", "../assets/stages/stage5.json"}; break;
        case 6: alternativePaths = {"assets/stages/tutorial.json", "../assets/stages/tutorial.json"}; break;
        default: return "";
    }
    
    for (const auto& path : alternativePaths) {
        std::ifstream file(path);
        if (file.good()) {
            file.close();
            return path;
        }
        file.close();
    }
    
    return "";
}

void StageManager::updateCurrentStageFileInfo(int stageNumber) {
    currentStageFilePath = getStageFilePath(stageNumber);
    if (!currentStageFilePath.empty()) {
        lastFileModificationTime = getFileModificationTime(currentStageFilePath);
    } else {
        currentStageFilePath = "";
        lastFileModificationTime = 0;
    }
}

bool StageManager::checkAndReloadStage(GameState& gameState, PlatformSystem& platformSystem) {
    if (currentStageFilePath.empty()) {
        return false;
    }
    
    std::string actualPath = getStageFilePath(currentStage);
    if (actualPath != currentStageFilePath) {
        updateCurrentStageFileInfo(currentStage);
        return false;
    }
    
    std::time_t currentModTime = getFileModificationTime(currentStageFilePath);
    
    if (currentModTime > 0 && currentModTime != lastFileModificationTime && lastFileModificationTime > 0) {
        lastFileModificationTime = currentModTime;
        return loadStage(currentStage, gameState, platformSystem);
    }
    
    if (lastFileModificationTime == 0 && currentModTime > 0) {
        lastFileModificationTime = currentModTime;
    }
    
    return false;
}
