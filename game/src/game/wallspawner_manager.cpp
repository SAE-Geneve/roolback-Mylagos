#include "game/wallspawner_manager.h"
#include "game/game_manager.h"

namespace game
{
	WallSpawnerManager::WallSpawnerManager(core::EntityManager& entityManager, PhysicsManager& physicsManager,
		GameManager& gameManager, PlayerCharacterManager& playerCharacterManager) :
		ComponentManager(entityManager),
		physicsManager_(physicsManager),
		gameManager_(gameManager),
		playerCharacterManager_(playerCharacterManager)
	{}


	void WallSpawnerManager::FixedUpdate(const sf::Time dt)
	{
		for (core::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
		{
			const bool isWallSpawner = entityManager_.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::WALLSPAWNER));
			if (!isWallSpawner) continue;

			auto& wallSpawner = GetComponent(entity);

			auto& wallBody = physicsManager_.GetBody(entity);

			auto& player = playerCharacterManager_.GetComponent(wallSpawner.playerEntity);

			const auto inputs = player.input;

			const bool down = inputs & PlayerInputEnum::PlayerInput::BUILD;
			const bool seven = inputs & PlayerInputEnum::PlayerInput::BM_LEFT;
			const bool nine = inputs & PlayerInputEnum::PlayerInput::BM_RIGHT;
			
			if (wallSpawner.movementTimeLocker == 0.0f && (seven || nine))
			{
					wallBody.position.x += ((seven ? -1.0f : 0.0f) + (nine ? 1.0f : 0.0f)) * 0.5f;
					wallSpawner.movementTimeLocker+=dt.asSeconds();
			}

			if(wallSpawner.movementTimeLocker!=0.0f)
			{
				wallSpawner.movementTimeLocker += dt.asSeconds();
				if(wallSpawner.movementTimeLocker >= 0.25f)
				{
					wallSpawner.movementTimeLocker = 0.0f;
				}
			}
			

			if(wallSpawner.buildTimeLocker == 0.0f && down)
			{
				gameManager_.SpawnWall(player.playerNumber, wallBody.position);
				wallSpawner.buildTimeLocker += dt.asSeconds();
			}

			if (wallSpawner.buildTimeLocker != 0.0f)
			{
				wallSpawner.buildTimeLocker += dt.asSeconds();
				if (wallSpawner.buildTimeLocker >= 1.0f)
				{
					wallSpawner.buildTimeLocker = 0.0f;
				}
			}

		}
	}
}
