/**
 * @file stage_manager.h
 * @brief ステージ管理システム
 * @details ステージの読み込み、解放、進行状況の管理を行います。
 */
#pragma once

#include "../core/states/game_state.h"
#include "platform_system.h"
#include <vector>
#include <functional>
#include <string>
#include <ctime>
#include <map>

/**
 * @brief ステージデータの構造体
 * @details ステージの基本情報を保持します。
 */
struct StageData {
    int stageNumber;
    std::function<void(GameState&, PlatformSystem&)> generateFunction;
    bool isUnlocked;
    bool isCompleted;
};

/**
 * @brief ステージ管理システム
 * @details ステージの読み込み、解放、進行状況の管理を行います。
 */
class StageManager {
public:
    StageManager();
    ~StageManager() = default;
    
    /**
     * @brief ステージを初期化する
     * @details 全ステージのデータを初期化します。
     */
    void initializeStages();
    
    /**
     * @brief ステージを読み込む
     * @details 指定されたステージ番号のステージを読み込みます。
     * 
     * @param stageNumber ステージ番号
     * @param gameState ゲーム状態
     * @param platformSystem プラットフォームシステム
     * @return 読み込み成功時true
     */
    bool loadStage(int stageNumber, GameState& gameState, PlatformSystem& platformSystem);
    
    /**
     * @brief ステージをクリア済みにする
     * @details 指定されたステージ番号のステージをクリア済みにマークします。
     * 
     * @param stageNumber ステージ番号
     */
    void completeStage(int stageNumber);
    
    /**
     * @brief 現在のステージ番号を取得する
     * @return 現在のステージ番号
     */
    int getCurrentStage() const { return currentStage; }
    
    /**
     * @brief 総ステージ数を取得する
     * @return 総ステージ数
     */
    int getTotalStages() const { return stages.size(); }
    
    /**
     * @brief ステージが解放されているか確認する
     * @param stageNumber ステージ番号
     * @return 解放されている場合true
     */
    bool isStageUnlocked(int stageNumber) const;
    
    /**
     * @brief ステージがクリア済みか確認する
     * @param stageNumber ステージ番号
     * @return クリア済みの場合true
     */
    bool isStageCompleted(int stageNumber) const;
    
    /**
     * @brief ステージデータを取得する
     * @param stageNumber ステージ番号
     * @return ステージデータへのポインタ（存在しない場合はnullptr）
     */
    const StageData* getStageData(int stageNumber) const;
    
    /**
     * @brief 指定ステージに移動する
     * @details 指定されたステージ番号のステージに移動します。
     * 
     * @param stageNumber ステージ番号
     * @param gameState ゲーム状態
     * @param platformSystem プラットフォームシステム
     * @return 移動成功時true
     */
    bool goToStage(int stageNumber, GameState& gameState, PlatformSystem& platformSystem);
    
    /**
     * @brief ステージファイルの変更を確認してリロードする
     * @details ステージファイルが変更されていた場合、自動的にリロードします。
     * 
     * @param gameState ゲーム状態
     * @param platformSystem プラットフォームシステム
     * @return リロードが発生した場合true
     */
    bool checkAndReloadStage(GameState& gameState, PlatformSystem& platformSystem);
    
    /**
     * @brief 現在のステージファイルパスを取得する
     * @details エディタ用に現在のステージのJSONファイルパスを取得します。
     * 
     * @return 現在のステージファイルパスへのconst参照
     */
    const std::string& getCurrentStageFilePath() const { return currentStageFilePath; }
    
private:
    std::vector<StageData> stages;
    int currentStage;
    
    std::string currentStageFilePath;
    std::time_t lastFileModificationTime;
    std::map<int, std::string> stageFilePaths;
    
    std::time_t getFileModificationTime(const std::string& filepath);
    std::string getStageFilePath(int stageNumber);
    void updateCurrentStageFileInfo(int stageNumber);
    
    static void generateStageSelectionField(GameState& gameState, PlatformSystem& platformSystem);
    static void generateStage1(GameState& gameState, PlatformSystem& platformSystem);
    static void generateStage2(GameState& gameState, PlatformSystem& platformSystem);
    static void generateStage3(GameState& gameState, PlatformSystem& platformSystem);
    static void generateStage4(GameState& gameState, PlatformSystem& platformSystem);
    static void generateStage5(GameState& gameState, PlatformSystem& platformSystem);
    static void generateTutorialStage(GameState& gameState, PlatformSystem& platformSystem);
    
    bool generateStageFromConfig(int stageNumber, GameState& gameState, PlatformSystem& platformSystem);
};
