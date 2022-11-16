#pragma once
#include <SFML/System/Time.hpp>

#include "game_globals.h"

namespace game
{

/**
 * \brief PlayerCharacter is a struct that holds information about the player character (when they can shoot again, their current input, and their current health).
 */
struct PlayerCharacter
{

    float shootingTime = 0.0f;
    PlayerInput input = 0u;
    PlayerNumber playerNumber = INVALID_PLAYER;
    short health = playerHealth;
    float invincibilityTime = 0.0f;
    float acceleration = 0.0f;
    float jumpBuffer = 0.0f;
    int jumpCooldownCount = 0;
    bool isOnGround = false;

    float summForceX = 0.0f;
    float summForceY = 0.0f;

    bool otherPlayerIsOnLeft = false;
    bool otherPlayerIsOnRight = false;
    float collisionAccelerationOverTime = 0.0f;
};

class PhysicsManager;

class GameManager;

/**
 * \brief PlayerCharacterManager is a ComponentManager that holds all the PlayerCharacter in the game.
 */
class PlayerCharacterManager : public core::ComponentManager<PlayerCharacter, static_cast<core::EntityMask>(ComponentType::PLAYER_CHARACTER)>
{
public:
    explicit PlayerCharacterManager(core::EntityManager& entityManager, PhysicsManager& physicsManager, GameManager& gameManager);
    void FixedUpdate(sf::Time dt);

private:
    PhysicsManager& physicsManager_;
    GameManager& gameManager_;
};
}
