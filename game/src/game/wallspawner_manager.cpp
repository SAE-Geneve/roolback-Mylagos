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
			if (entityManager_.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::WALLSPAWNER)) && entityManager_.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::PLAYER_CHARACTER)))
			{
				auto& spawner = components_[entity];

				auto spawnerBody = physicsManager_.GetBody(entity);
				spawnerBody.velocity.x += (5+spawner.verticalVelocity) * dt.asSeconds();
				

					//spawnerBody.position = (core::Vec2f(0.0f, 0.0f));
				float ooo = dt.asSeconds();
					ooo++;

				physicsManager_.SetBody(entity, spawnerBody);
			}
		}
	}
}