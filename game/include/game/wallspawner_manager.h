#pragma once
#include <SFML/System/Time.hpp>

#include "game_globals.h"

namespace game
{
	class PhysicsManager;
	class GameManager;
	class PlayerCharacterManager;

	/**
	 * \Wall Spawner is a struct that contains info the player needs to spawn walls. (Position, begin and end)
	 */
	struct WallSpawner
	{
		core::Entity playerEntity = core::INVALID_ENTITY;
		float movementTimeLocker = 0.0f;
		float buildTimeLocker = 0.0f;
		PlayerNumber playerNumber = INVALID_PLAYER;
	};

	/**
	 * \brief WallSpawnerManager is a ComponentManager that will move all the WallSpawners.
	 * The wall spawners are the cursors at the top of the game that will indicate where a wall will be spawned to the player.
	 */
	class WallSpawnerManager : public core::ComponentManager<WallSpawner,
		static_cast<core::EntityMask>(ComponentType::WALLSPAWNER)>
	{
	public:
		explicit WallSpawnerManager(
			core::EntityManager& entityManager,
			PhysicsManager& physicsManager,
			GameManager& gameManager,
			PlayerCharacterManager& playerCharacterManager_);
		void FixedUpdate(sf::Time dt);
	private:
		PhysicsManager& physicsManager_;
		GameManager& gameManager_;
		PlayerCharacterManager& playerCharacterManager_;
	};
}