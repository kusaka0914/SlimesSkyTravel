/**
 * @file game_loop.h
 * @brief ゲームループ
 * @details メインゲームループと各種画面処理（タイトル、Ready、ステージクリアなど）を管理します。
 */
#pragma once

#include <GLFW/glfw3.h>
#include <memory>
#include <map>
#include <chrono>
#include <functional>
#include <glm/glm.hpp>
#include "../game/game_state.h"
#include "../game/stage_manager.h"
#include "../game/platform_system.h"
#include "../gfx/opengl_renderer.h"
#include "../gfx/ui_renderer.h"
#include "../gfx/game_state_ui_renderer.h"
#include "../gfx/camera_system.h"
#include "../core/utils/input_utils.h"
#include "../io/audio_manager.h"

namespace GameLoop {
    /**
     * @brief ゲームループの実行
     * @details メインゲームループを実行し、更新処理と描画処理を統合的に管理します。
     * 
     * @param window GLFWウィンドウ
     * @param gameState ゲーム状態
     * @param stageManager ステージマネージャー
     * @param platformSystem プラットフォームシステム
     * @param renderer OpenGLレンダラー
     * @param uiRenderer UIレンダラー
     * @param gameStateUIRenderer ゲーム状態UIレンダラー
     * @param keyStates キー状態マップ
     * @param resetStageStartTime ステージ開始時間をリセットする関数
     * @param startTime 開始時刻
     * @param audioManager オーディオマネージャー
     */
    void run(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
             PlatformSystem& platformSystem,
             std::unique_ptr<gfx::OpenGLRenderer>& renderer,
             std::unique_ptr<gfx::UIRenderer>& uiRenderer,
             std::unique_ptr<gfx::GameStateUIRenderer>& gameStateUIRenderer,
             std::map<int, InputUtils::KeyState>& keyStates,
             std::function<void()> resetStageStartTime,
             std::chrono::high_resolution_clock::time_point& startTime,
             io::AudioManager& audioManager);
    
    /**
     * @brief Ready画面の処理
     * @details Ready画面の表示と入力処理を管理します。
     * 
     * @param window GLFWウィンドウ
     * @param gameState ゲーム状態
     * @param stageManager ステージマネージャー
     * @param platformSystem プラットフォームシステム
     * @param renderer OpenGLレンダラー
     * @param gameStateUIRenderer ゲーム状態UIレンダラー
     * @param keyStates キー状態マップ
     * @param resetStageStartTime ステージ開始時間をリセットする関数
     * @param audioManager オーディオマネージャー
     * @param deltaTime デルタタイム
     */
    void handleReadyScreen(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                          PlatformSystem& platformSystem,
                          std::unique_ptr<gfx::OpenGLRenderer>& renderer,
                          std::unique_ptr<gfx::GameStateUIRenderer>& gameStateUIRenderer,
                          std::map<int, InputUtils::KeyState>& keyStates,
                          std::function<void()> resetStageStartTime,
                          io::AudioManager& audioManager,
                          float deltaTime);
    
    /**
     * @brief カウントダウンの処理
     * @details カウントダウン表示と入力処理を管理します。
     * 
     * @param window GLFWウィンドウ
     * @param gameState ゲーム状態
     * @param stageManager ステージマネージャー
     * @param platformSystem プラットフォームシステム
     * @param renderer OpenGLレンダラー
     * @param gameStateUIRenderer ゲーム状態UIレンダラー
     * @param keyStates キー状態マップ
     * @param resetStageStartTime ステージ開始時間をリセットする関数
     * @param deltaTime デルタタイム
     */
    void handleCountdown(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                        PlatformSystem& platformSystem,
                        std::unique_ptr<gfx::OpenGLRenderer>& renderer,
                        std::unique_ptr<gfx::GameStateUIRenderer>& gameStateUIRenderer,
                        std::map<int, InputUtils::KeyState>& keyStates,
                        std::function<void()> resetStageStartTime,
                        float deltaTime);
    
    /**
     * @brief エンディングシーケンスの処理
     * @details エンディングシーケンスの表示と入力処理を管理します。
     * 
     * @param window GLFWウィンドウ
     * @param gameState ゲーム状態
     * @param stageManager ステージマネージャー
     * @param platformSystem プラットフォームシステム
     * @param renderer OpenGLレンダラー
     * @param gameStateUIRenderer ゲーム状態UIレンダラー
     * @param keyStates キー状態マップ
     * @param deltaTime デルタタイム
     */
    void handleEndingSequence(GLFWwindow* window, GameState& gameState, StageManager& stageManager, 
                             PlatformSystem& platformSystem,
                             std::unique_ptr<gfx::OpenGLRenderer>& renderer,
                             std::unique_ptr<gfx::GameStateUIRenderer>& gameStateUIRenderer,
                             std::map<int, InputUtils::KeyState>& keyStates,
                             float deltaTime);
    
    /**
     * @brief ステージ選択エリアの処理
     * @details ステージ選択エリアでの入力処理を管理します。
     * 
     * @param window GLFWウィンドウ
     * @param gameState ゲーム状態
     * @param stageManager ステージマネージャー
     * @param platformSystem プラットフォームシステム
     * @param resetStageStartTime ステージ開始時間をリセットする関数
     */
    void handleStageSelectionArea(GLFWwindow* window, GameState& gameState, StageManager& stageManager,
                                 PlatformSystem& platformSystem, std::function<void()> resetStageStartTime,
                                 std::map<int, InputUtils::KeyState>& keyStates, float deltaTime);
}
