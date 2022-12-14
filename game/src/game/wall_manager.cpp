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

	void WallManager::FixedUpdate(const sf::Time dt)
	{
		for (core::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
		{
			if (entityManager_.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::WALL)))
			{
				auto& wall = components_[entity];
				auto wallBody = physicsManager_.GetBody(entity);


				if (wall.wallType == WallType::WallStatic || wall.wallType == WallType::WallArena)
				{
					continue;
				}

				wallBody.velocity += core::Vec2f(0.0f, game::gravity * (static_cast<float>(wall.wallType) / 100.0f) * (0.5f + wallSpeedRatio(wallBody.position.y))) * dt.asSeconds();


				physicsManager_.SetBody(entity, wallBody);

			}
		}
	}
}