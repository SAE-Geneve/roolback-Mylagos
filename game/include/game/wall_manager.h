#pragma once
#include <SFML/System/Time.hpp>

#include "game_globals.h"

namespace game
{
	enum class WallType
	{
		STATIC = 0,
		WallSimple = 100,
		WallDouble = 125,
		WallTriple = 150
	};

	/**
	 * \brief Bullet is a struct that contains info about a player bullet (when it will be destroyed and whose player it is).
	 */
	struct Wall
	{
		float remainingTime = 0.0f;
		PlayerNumber playerNumber = INVALID_PLAYER;
		WallType wallType = WallType::STATIC;

		/**
		 * \brief The lower the wall get the higher this func returns (0 to 1)
		 */
	};

	class PhysicsManager;

	class GameManager;

	/**
	 * \brief BulletManager is a ComponentManager that holds all the Bullet in one place.
	 * It will automatically destroy the Bullet when remainingTime is over.
	 */
	class WallManager : public core::ComponentManager<Wall, static_cast<core::EntityMask>(ComponentType::WALL)>
	{
	public:
		explicit WallManager(core::EntityManager& entityManager, PhysicsManager& physicsManager, GameManager& gameManager);
		void FixedUpdate(sf::Time dt);
	private:
		static float wallSpeedRatio(const float pos);
		PhysicsManager& physicsManager_;
		GameManager& gameManager_;
	};
}