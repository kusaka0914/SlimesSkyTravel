#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <GLFW/glfw3.h>
#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <windows.h>
#endif
#include <iostream>
#include <chrono>
#include <thread>
#include <memory>
#include <map>
#include <glm/glm.hpp>
#include "../gfx/opengl_renderer.h"
#include "../core/states/game_state.h"
#include "../game/stage_manager.h"
#include "../game/save_manager.h"
#include "../game/platform_system.h"
#include "../physics/physics_system.h"
#include "../io/input_system.h"
#include "../io/audio_manager.h"
#include "../core/constants/game_constants.h"
#include "../gfx/camera_system.h"
#include "../core/utils/input_utils.h"
#include "../core/utils/ui_config_manager.h"
#include "../game/online_leaderboard_manager.h"
#include "game_loop.h"
#include "tutorial_manager.h"

int main(int argc, char* const argv[]) {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* videoMode = glfwGetVideoMode(primaryMonitor);
    
    GLFWwindow* window = glfwCreateWindow(videoMode->width, videoMode->height, 
        "Slime's Sky Travel", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    
    #ifdef _WIN32
        HWND hwnd = glfwGetWin32Window(window);
        if (hwnd) {
            LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
            style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
            SetWindowLongPtr(hwnd, GWL_STYLE, style);
            
            SetWindowPos(hwnd, HWND_TOP, 0, 0, videoMode->width, videoMode->height, 
                        SWP_FRAMECHANGED | SWP_NOZORDER);
            
            ShowWindow(hwnd, SW_SHOWMAXIMIZED);
            SetForegroundWindow(hwnd);
        }
    #else
        glfwSetWindowMonitor(window, primaryMonitor, 0, 0, videoMode->width, videoMode->height, videoMode->refreshRate);
    #endif
    
    glfwPollEvents();
    
    glfwShowWindow(window);
    glfwFocusWindow(window);
    
    glfwSetCharCallback(window, nullptr);
    
    auto renderer = std::make_unique<gfx::OpenGLRenderer>();
    if (!renderer->initialize(window)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    
    auto uiRenderer = std::make_unique<gfx::UIRenderer>();
    auto gameStateUIRenderer = std::make_unique<gfx::GameStateUIRenderer>();
    
    GameState gameState;
    
    initializeGameState(gameState);
    
    SaveManager::loadGameData(gameState);
    
    int initialStage = 6;
    bool debugEnding = false;
    
    if (argc > 1) {
        if (strcmp(argv[1], "-e") == 0 || strcmp(argv[1], "--ending") == 0) {
            debugEnding = true;
        } else {
            int requestedStage = std::atoi(argv[1]);
            if (requestedStage >= 0 && requestedStage <= 5) {
                initialStage = requestedStage;
            } else {
                printf("Invalid stage number: %d. Using default stage %d\n", requestedStage, initialStage);
            }
        }
    }
    glfwSetWindowUserPointer(window, &gameState);
    
    glfwSetCursorPosCallback(window, InputSystem::mouse_callback);
    glfwSetScrollCallback(window, InputSystem::scroll_callback);
    
    PlatformSystem platformSystem;
    StageManager stageManager;
    
    io::AudioManager audioManager;
    if (!audioManager.initialize()) {
        std::cerr << "Failed to initialize audio system" << std::endl;
    }
    
    UIConfig::UIConfigManager::getInstance().loadConfig("assets/config/ui_config.json");
    
    OnlineLeaderboardManager::loadConfigFromFile();
    
    if (debugEnding) {
        gameState.ui.isEndingSequence = true;
        gameState.ui.showStaffRoll = true;
        gameState.ui.staffRollTimer = 0.0f;
        gameState.ui.showTitleScreen = false;
    } else {
        gameState.ui.showTitleScreen = true;
    }

    std::map<int, InputUtils::KeyState> keyStates;
    for (int key : {GLFW_KEY_0, GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4, GLFW_KEY_5, GLFW_KEY_6, 
                    GLFW_KEY_LEFT, GLFW_KEY_RIGHT, GLFW_KEY_ENTER, GLFW_KEY_R, GLFW_KEY_T,
                    GLFW_KEY_F, GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_SPACE}) {
        keyStates[key] = InputUtils::KeyState();
    }

    auto startTime = std::chrono::high_resolution_clock::now();
    
    auto resetStageStartTime = [&startTime, &gameState]() {
        startTime = std::chrono::high_resolution_clock::now();
        gameState.progress.timeLimitApplied = false;
    };
    
    GameLoop::run(window, gameState, stageManager, platformSystem, renderer, uiRenderer, gameStateUIRenderer, keyStates, resetStageStartTime, startTime, audioManager);
    
    renderer->cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}
