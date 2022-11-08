#include <iostream>
#include <game/player_character.h>
#include <game/game_manager.h>

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif
namespace game
{
	PlayerCharacterManager::PlayerCharacterManager(core::EntityManager& entityManager, PhysicsManager& physicsManager, GameManager& gameManager) :
		ComponentManager(entityManager),
		physicsManager_(physicsManager),
		gameManager_(gameManager)

	{
		
	}

	void PlayerCharacterManager::FixedUpdate(sf::Time dt)
	{

#ifdef TRACY_ENABLE
		ZoneScoped;
#endif
		for (PlayerNumber playerNumber = 0; playerNumber < maxPlayerNmb; playerNumber++)
		{
			const auto playerEntity = gameManager_.GetEntityFromPlayerNumber(playerNumber);
			if (!entityManager_.HasComponent(playerEntity,
				static_cast<core::EntityMask>(ComponentType::PLAYER_CHARACTER)))
				continue;
			auto playerBody = physicsManager_.GetBody(playerEntity);
			auto playerBox = physicsManager_.GetBox(playerEntity);
			auto playerCharacter = GetComponent(playerEntity);
			const auto input = playerCharacter.input;

			const bool right = input & PlayerInputEnum::PlayerInput::RIGHT;
			const bool left = input & PlayerInputEnum::PlayerInput::LEFT;
			const bool up = input & PlayerInputEnum::PlayerInput::UP;
			const bool down = input & PlayerInputEnum::PlayerInput::DOWN;



			core::Vec2f accelerationX = ((left ? -1.0f : 0.0f) + (right ? 1.0f : 0.0f)) * core::Vec2f(20.0f, 0.0f);

			if (left ^ right)
			{
				if (playerCharacter.acceleration < 1.5f)
				{
					playerCharacter.acceleration *= 2.0f;
				}
			}
			else
			{
				if (std::abs(playerBody.velocity.x) < 0.01f)
				{
					playerBody.velocity.x = 0.0f;
					playerCharacter.acceleration = 0.1f;
				}
				else
				{
					playerCharacter.acceleration /= 1.1f;
				}
			}

			if (std::abs(playerBody.velocity.x) > playerCharacter.acceleration)
			{
				accelerationX = (playerBody.velocity.x > 0.0f ? -1.0f : +1.0f) * core::Vec2f(20.0f, 0.0f);
			}


			playerBody.velocity += (accelerationX)*dt.asSeconds();

			if (playerBody.velocity.y > 1.0f)
			{
				playerBody.velocity.y += game::gravity * dt.asSeconds();
			}
			else {
				playerBody.velocity.y += game::gravity * 2 * dt.asSeconds();
			}

			//core::LogDebug(fmt::format("Player's acceleration is x("  + std::to_string(playerBody.velocity.x) + ") : y(" + std::to_string(playerBody.velocity.y) + ")\n"));
			//core::LogDebug(fmt::format("Player's position is x("  + std::to_string(playerBody.position.x) + ") : y(" + std::to_string(playerBody.position.y) + ")\n"));

				playerBox.collisionType = CollisionType::DYNAMIC;
			if (playerBody.position.y <= -3.0f)
			{
				playerBody.position.y = -3.0f;
				playerBody.velocity.y = 0.0f;
				playerCharacter.jumpBuffer = 0.0f;
				//playerBox.collisionType = CollisionType::STATIC;
			}
			if (playerBox.hasCollided)
			{
				playerBody.velocity.y = 0;
			}
			if (up && playerCharacter.jumpBuffer < 2.0f)
			{
				playerBox.hasCollided = false;
				playerBody.velocity.y += 30.0f * dt.asSeconds();
				playerCharacter.jumpBuffer += 0.3f;
			}
			else
			{
				playerCharacter.jumpBuffer = 3;
			}


			physicsManager_.SetBox(playerEntity, playerBox);
			physicsManager_.SetBody(playerEntity, playerBody);

			SetComponent(playerEntity, playerCharacter);
		}
	}
}
