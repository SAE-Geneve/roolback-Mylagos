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

			playerBox.collideWithSame = true;
			playerBox.collisionType = CollisionType::DYNAMIC;



			if (up && playerCharacter.isOnGround)
			{
				playerCharacter.jumpBuffer++;
				if (playerCharacter.jumpBuffer == 1)
				{
					core::LogDebug("eee");
					playerBody.velocity.y += 120.0f * dt.asSeconds();
				}
				else if (playerCharacter.jumpBuffer > 0 && playerCharacter.jumpBuffer < 10)
				{
					playerBody.velocity.y += game::gravity * dt.asSeconds();
					playerBody.velocity.y += 20 * dt.asSeconds();
				}
			}
			else if (!up || playerCharacter.jumpBuffer > 20)
			{
				playerCharacter.isOnGround = false;
				playerCharacter.jumpCooldownCount = 0;
				playerCharacter.jumpBuffer = 0;
			}
			if (playerBody.velocity.y < 0.01f && playerBody.velocity.y > -0.01f)
			{
				playerCharacter.jumpCooldownCount += 1;
				if (playerCharacter.jumpCooldownCount > 2)
				{
					playerCharacter.isOnGround = true;
				}
			}

			if (!playerCharacter.isOnGround || playerCharacter.jumpBuffer != 0)
			{
				if (playerBody.velocity.y > 1.0f)
				{
					playerBody.velocity.y += game::gravity * dt.asSeconds();
				}
				else {
					playerBody.velocity.y += game::gravity * 2 * dt.asSeconds();
				}
			}




			playerCharacter.summForceX = 0.0f;

			left&& playerBody.velocity.x > 0 ? playerBody.velocity.x = 0 : 0.0f;
			right&& playerBody.velocity.x < 0 ? playerBody.velocity.x = 0 : 0.0f;


			playerCharacter.summForceX += ((left ? -1.0f : 0.0f) + (right ? 1.0f : 0.0f)) * 20.0f;

			//Multiply by drag
			playerBody.velocity.x *= 0.85f;

			playerBody.velocity.x += playerCharacter.summForceX * dt.asSeconds();

			


			physicsManager_.SetBox(playerEntity, playerBox);
			physicsManager_.SetBody(playerEntity, playerBody);

			SetComponent(playerEntity, playerCharacter);
		}
	}
}
