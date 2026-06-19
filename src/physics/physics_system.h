/**
 * @file physics_system.h
 * @brief 物理システム
 * @details 物理演算と衝突判定を統合的に管理します。
 */
#pragma once

#include "../core/states/game_state.h"

/**
 * @brief 物理システム
 * @details 物理演算と衝突判定を統合的に管理します。
 */
class PhysicsSystem {
public:
    /**
     * @brief 足場衝突判定
     * @details プレイヤーと足場の衝突を判定します。
     * 
     * @param gameState ゲーム状態
     * @param playerPos プレイヤー位置
     * @param playerSize プレイヤーサイズ
     * @return 衝突している場合true
     */
    static bool checkPlatformCollision(const GameState& gameState, const glm::vec3& playerPos, const glm::vec3& playerSize);
};
