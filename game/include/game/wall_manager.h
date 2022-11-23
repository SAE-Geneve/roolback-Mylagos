#pragma once
#include <SFML/System/Time.hpp>

#include "game_globals.h"

namespace game
{
	enum class WallType
	{
		WallStatic = 0,
		WallArena = 1,
		WallSimple = 100
	};

	/**
	 * \brief Wall is a struct that contains info about a Wall.
	 */
	struct Wall
	{
		bool isOnGround = false;
		bool hasCollided = false;
		PlayerNumber playerNumber = INVALID_PLAYER;
		WallType wallType = WallType::WallStatic;

	};

	class PhysicsManager;

	class GameManager;

	/**
	 * \brief WallManager is a ComponentManager that holds all the Walls in one place.
	 */
	class WallManager : public core::ComponentManager<Wall, static_cast<core::EntityMask>(ComponentType::WALL)>
	{
	public:
		explicit WallManager(core::EntityManager& entityManager, PhysicsManager& physicsManager, GameManager& gameManager);
		void FixedUpdate(sf::Time dt);
	private:
		/**
		 * \brief The lower the wall get the higher this func returns (0 to 1)
		 */
		static float wallSpeedRatio(const float pos);
		PhysicsManager& physicsManager_;
		GameManager& gameManager_;
	};
}