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

			core::LogDebug(std::to_string(wallBody.position.y));

			const bool down = inputs & PlayerInputEnum::PlayerInput::BUILD;
			const bool seven = inputs & PlayerInputEnum::PlayerInput::BM_LEFT;
			const bool nine = inputs & PlayerInputEnum::PlayerInput::BM_RIGHT;

			wallBody.velocity.x += ((seven ? -1.0f : 0.0f) + (nine ? 1.0f : 0.0f)) * 20.0f * dt.asSeconds();


			if (down)
			{
				if (!player.isbuilding)
				{



					player.isbuilding = true;
				}

			}
			else
			{
				if (player.isbuilding)
				{
					gameManager_.SpawnWall(player.playerNumber, player.wallSpawnPosition);
					player.isbuilding = false;
				}
			}
		}
	}
}
