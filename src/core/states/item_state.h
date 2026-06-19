/**
 * @file item_state.h
 * @brief アイテムシステムの状態を管理する構造体
 * @details アイテムの位置、回転、収集状態、収集数などの情報を保持します。
 */
#pragma once

#include <vector>
#include <glm/glm.hpp>

/**
 * @brief アイテムの状態を管理する構造体
 * @details アイテムの位置、サイズ、色、収集状態、ID、アニメーション情報を保持します。
 */
struct Item {
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 size = glm::vec3(0.5f, 0.5f, 0.5f);
    glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f);
    bool isCollected = false;
    int itemId = 0;
    float rotationAngle = 0.0f;
    float bobHeight = 0.0f;
    float bobTimer = 0.0f;
};

/**
 * @brief アイテムシステムの状態を管理する構造体
 */
struct ItemState {
    std::vector<Item> items;
    int collectedItems = 0;
    int requiredItems = 3;
    int earnedItems = 0;
    int totalItems = 3;
};

