/**
 * @file camera_state.h
 * @brief カメラの状態を管理する構造体
 * @details カメラの角度、距離、マウス入力、視点モードなどの状態を保持します。
 */
#pragma once

/**
 * @brief カメラの状態を管理する構造体
 * @details カメラの角度、距離、マウス入力、視点モードなどの状態を保持します。
 */
struct CameraState {
    float yaw = 90.0f;
    float pitch = 0.0f;
    float distance = 30.0f;
    bool firstMouse = true;
    float lastMouseX = 640.0f;
    float lastMouseY = 360.0f;
    
    bool isFirstPersonView = false;
    bool isFirstPersonMode = false;
};

