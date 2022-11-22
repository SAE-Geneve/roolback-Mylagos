#include "game/wallspawner_manager.h"
#include "game/game_manager.h"



namespace game
{

	WallSpawnerManager::WallSpawnerManager(core::EntityManager& entityManager, PhysicsManager& physicsManager, GameManager& gameManager) :
		ComponentManager(entityManager),
		physicsManager_(physicsManager),
		gameManager_(gameManager)

	{
	}


	void WallSpawnerManager::FixedUpdate(const sf::Time dt)
	{
		for (core::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
		{
			if (entityManager_.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::WALLSPAWNER)))
			{
				auto& spawner = components_[entity];

				core::LogDebug("ooo");
				auto spawnerBody = physicsManager_.GetBody(entity);
				spawnerBody.velocity.y += 5 * dt.asSeconds();
				if (spawner.i == 0)
				{
					spawner.i++;
					spawnerBody.position = (core::Vec2f(0.0f, 0.0f));
				}

				physicsManager_.SetBody(entity, spawnerBody);
			}
		}
	}
}