#include "game/wall_manager.h"
#include "game/game_manager.h"



namespace game
{
	WallManager::WallManager(core::EntityManager& entityManager, PhysicsManager& physicsManager, GameManager& gameManager) :
		ComponentManager(entityManager),
		physicsManager_(physicsManager),
		gameManager_(gameManager)

	{
	}

	float WallManager::wallSpeedRatio(const float pos)
	{
		float heightTotal = game::wallMaxHeight + (-game::wallMinHeight);
		float myPos = pos + (-game::wallMinHeight);
		return (1 - myPos / heightTotal);
	}

	void WallManager::FixedUpdate(sf::Time dt)
	{
		for (core::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
		{
			if (entityManager_.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::DESTROYED)))
			{
				continue;
			}
			if (entityManager_.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::WALL)))
			{
				auto& wall = components_[entity];
				auto wallEntity = physicsManager_.GetBody(entity);
				wallEntity.velocity += core::Vec2f(0.0f, game::gravity * (static_cast<float>(wall.walltype) / 100.0f) * wallSpeedRatio(wallEntity.position.y));


			}
		}
	}
}