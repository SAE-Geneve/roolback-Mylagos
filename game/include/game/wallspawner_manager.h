#pragma once
#include <SFML/System/Time.hpp>

#include "game_globals.h"

namespace game
{
	/**
	 * \Wall Spawner is a struct that contains info the player needs to spawn walls. (Position, begin and end)
	 */
	struct WallSpawner
	{
		
		PlayerNumber playerNumber = INVALID_PLAYER;
		core::Vec2f initialPosition{};
		core::Vec2f finalPosition{};
		int i = 0;
	};

	class PhysicsManager;

	class GameManager;

	/**
	 * \brief WallSpawnerManager is a ComponentManager that will move all the WallSpawners.
	 * It will automatically destroy the Bullet when remainingTime is over.
	 */
	class WallSpawnerManager : public core::ComponentManager<WallSpawner, static_cast<core::EntityMask>(ComponentType::WALLSPAWNER)>
	{
	public:
		explicit WallSpawnerManager(core::EntityManager& entityManager, PhysicsManager& physicsManager, GameManager& gameManager);
		void FixedUpdate(sf::Time dt);
	private:
		PhysicsManager& physicsManager_;
		GameManager& gameManager_;
	};
}