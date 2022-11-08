#include <iostream>
#include <game/rollback_manager.h>
#include <game/game_manager.h>
#include "utils/assert.h"
#include <utils/log.h>
#include <fmt/format.h>

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif

namespace game
{

	RollbackManager::RollbackManager(GameManager& gameManager, core::EntityManager& entityManager) :
		gameManager_(gameManager), entityManager_(entityManager),
		currentTransformManager_(entityManager),
		currentPhysicsManager_(entityManager), currentPlayerManager_(entityManager, currentPhysicsManager_, gameManager_),
		currentBulletManager_(entityManager, gameManager), currentWallManager_(entityManager, currentPhysicsManager_, gameManager_),
		lastValidatePhysicsManager_(entityManager),
		lastValidatePlayerManager_(entityManager, lastValidatePhysicsManager_, gameManager_), lastValidateBulletManager_(entityManager, gameManager),
		lastValidateWallManager_(entityManager, lastValidatePhysicsManager_, gameManager_)
	{
		for (auto& input : inputs_)
		{
			std::fill(input.begin(), input.end(), '\0');
		}
		currentPhysicsManager_.RegisterTriggerListener(*this);
	}

	void RollbackManager::SimulateToCurrentFrame()
	{

#ifdef TRACY_ENABLE
		ZoneScoped;
#endif
		const auto currentFrame = gameManager_.GetCurrentFrame();
		const auto lastValidateFrame = gameManager_.GetLastValidateFrame();
		//Destroying all created Entities after the last validated frame
		for (const auto& createdEntity : createdEntities_)
		{
			if (createdEntity.createdFrame > lastValidateFrame)
			{
				entityManager_.DestroyEntity(createdEntity.entity);
			}
		}
		createdEntities_.clear();
		//Remove DESTROY flags
		for (core::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
		{
			if (entityManager_.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::DESTROYED)))
			{
				entityManager_.RemoveComponent(entity, static_cast<core::EntityMask>(ComponentType::DESTROYED));
			}
		}

		//Revert the current game state to the last validated game state
		currentBulletManager_.CopyAllComponents(lastValidateBulletManager_.GetAllComponents());
		currentPhysicsManager_.CopyAllComponents(lastValidatePhysicsManager_);
		currentWallManager_.CopyAllComponents(lastValidateWallManager_.GetAllComponents());
		currentPlayerManager_.CopyAllComponents(lastValidatePlayerManager_.GetAllComponents());

		for (Frame frame = lastValidateFrame + 1; frame <= currentFrame; frame++)
		{
			testedFrame_ = frame;
			//Copy player inputs to player manager
			for (PlayerNumber playerNumber = 0; playerNumber < maxPlayerNmb; playerNumber++)
			{
				const auto playerInput = GetInputAtFrame(playerNumber, frame);
				const auto playerEntity = gameManager_.GetEntityFromPlayerNumber(playerNumber);
				if (playerEntity == core::INVALID_ENTITY)
				{
					core::LogWarning(fmt::format("Invalid Entity in {}:line {}", __FILE__, __LINE__));
					continue;
				}
				auto playerCharacter = currentPlayerManager_.GetComponent(playerEntity);
				playerCharacter.input = playerInput;
				currentPlayerManager_.SetComponent(playerEntity, playerCharacter);
			}
			//Simulate one frame of the game
			currentBulletManager_.FixedUpdate(sf::seconds(fixedPeriod));
			currentPlayerManager_.FixedUpdate(sf::seconds(fixedPeriod));
			currentPhysicsManager_.FixedUpdate(sf::seconds(fixedPeriod));
			currentWallManager_.FixedUpdate(sf::seconds(fixedPeriod));
		}
		//Copy the physics states to the transforms
		for (core::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
		{
			if (!entityManager_.HasComponent(entity,
				static_cast<core::EntityMask>(core::ComponentType::BODY2D) |
				static_cast<core::EntityMask>(core::ComponentType::TRANSFORM)))
				continue;
			const auto& body = currentPhysicsManager_.GetBody(entity);
			currentTransformManager_.SetPosition(entity, body.position);
			currentTransformManager_.SetRotation(entity, body.rotation);
		}
	}
	void RollbackManager::SetPlayerInput(PlayerNumber playerNumber, PlayerInput playerInput, Frame inputFrame)
	{
		//Should only be called on the server
		if (currentFrame_ < inputFrame)
		{
			StartNewFrame(inputFrame);
		}
		inputs_[playerNumber][currentFrame_ - inputFrame] = playerInput;
		if (lastReceivedFrame_[playerNumber] < inputFrame)
		{
			lastReceivedFrame_[playerNumber] = inputFrame;
			//Repeat the same inputs until currentFrame
			for (size_t i = 0; i < currentFrame_ - inputFrame; i++)
			{
				inputs_[playerNumber][i] = playerInput;
			}
		}
	}

	void RollbackManager::StartNewFrame(Frame newFrame)
	{

#ifdef TRACY_ENABLE
		ZoneScoped;
#endif
		if (currentFrame_ > newFrame)
			return;
		const auto delta = newFrame - currentFrame_;
		if (delta == 0)
		{
			return;
		}
		for (auto& inputs : inputs_)
		{
			for (auto i = inputs.size() - 1; i >= delta; i--)
			{
				inputs[i] = inputs[i - delta];
			}

			for (Frame i = 0; i < delta; i++)
			{
				inputs[i] = inputs[delta];
			}
		}
		currentFrame_ = newFrame;
		//std::cout << "ooooo\n\n\n";
	}

	void RollbackManager::ValidateFrame(Frame newValidateFrame)
	{

#ifdef TRACY_ENABLE
		ZoneScoped;
#endif
		const auto lastValidateFrame = gameManager_.GetLastValidateFrame();
		//We check that we got all the inputs
		for (PlayerNumber playerNumber = 0; playerNumber < maxPlayerNmb; playerNumber++)
		{
			if (GetLastReceivedFrame(playerNumber) < newValidateFrame)
			{
				gpr_assert(false, "We should not validate a frame if we did not receive all inputs!!!");
				return;
			}
		}
		//Destroying all created Entities after the last validated frame
		for (const auto& createdEntity : createdEntities_)
		{
			if (createdEntity.createdFrame > lastValidateFrame)
			{
				entityManager_.DestroyEntity(createdEntity.entity);
			}
		}
		createdEntities_.clear();
		//Remove DESTROYED flag
		for (core::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
		{
			if (entityManager_.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::DESTROYED)))
			{
				entityManager_.RemoveComponent(entity, static_cast<core::EntityMask>(ComponentType::DESTROYED));
			}

		}
		createdEntities_.clear();

		//We use the current game state as the temporary new validate game state
		currentBulletManager_.CopyAllComponents(lastValidateBulletManager_.GetAllComponents());
		currentPhysicsManager_.CopyAllComponents(lastValidatePhysicsManager_);
		currentPlayerManager_.CopyAllComponents(lastValidatePlayerManager_.GetAllComponents());
		currentWallManager_.CopyAllComponents(lastValidateWallManager_.GetAllComponents());

		//We simulate the frames until the new validated frame
		for (Frame frame = lastValidateFrame_ + 1; frame <= newValidateFrame; frame++)
		{
			testedFrame_ = frame;
			//Copy the players inputs into the player manager
			for (PlayerNumber playerNumber = 0; playerNumber < maxPlayerNmb; playerNumber++)
			{
				const auto playerInput = GetInputAtFrame(playerNumber, frame);
				const auto playerEntity = gameManager_.GetEntityFromPlayerNumber(playerNumber);
				auto playerCharacter = currentPlayerManager_.GetComponent(playerEntity);
				playerCharacter.input = playerInput;
				currentPlayerManager_.SetComponent(playerEntity, playerCharacter);
			}
			//We simulate one frame
			currentBulletManager_.FixedUpdate(sf::seconds(fixedPeriod));
			currentPlayerManager_.FixedUpdate(sf::seconds(fixedPeriod));
			currentPhysicsManager_.FixedUpdate(sf::seconds(fixedPeriod));
			currentWallManager_.FixedUpdate(sf::seconds(fixedPeriod));
		}
		//Definitely remove DESTROY entities
		for (core::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
		{
			if (entityManager_.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::DESTROYED)))
			{
				entityManager_.DestroyEntity(entity);
			}
		}
		//Copy back the new validate game state to the last validated game state
		lastValidateBulletManager_.CopyAllComponents(currentBulletManager_.GetAllComponents());
		lastValidatePlayerManager_.CopyAllComponents(currentPlayerManager_.GetAllComponents());
		lastValidatePhysicsManager_.CopyAllComponents(currentPhysicsManager_);
		lastValidateWallManager_.CopyAllComponents(currentWallManager_.GetAllComponents());
		lastValidateFrame_ = newValidateFrame;
		createdEntities_.clear();
	}
	void RollbackManager::ConfirmFrame(Frame newValidateFrame, const std::array<PhysicsState, maxPlayerNmb>& serverPhysicsState)
	{

#ifdef TRACY_ENABLE
		ZoneScoped;
#endif
		ValidateFrame(newValidateFrame);
		for (PlayerNumber playerNumber = 0; playerNumber < maxPlayerNmb; playerNumber++)
		{
			const PhysicsState lastPhysicsState = GetValidatePhysicsState(playerNumber);
			if (serverPhysicsState[playerNumber] != lastPhysicsState)
			{
				gpr_assert(false, fmt::format("Physics State are not equal for player {} (server frame: {}, client frame: {}, server: {}, client: {})",
					playerNumber + 1,
					newValidateFrame,
					lastValidateFrame_,
					serverPhysicsState[playerNumber],
					lastPhysicsState));
			}
		}
	}
	PhysicsState RollbackManager::GetValidatePhysicsState(PlayerNumber playerNumber) const
	{
		PhysicsState state = 0;
		const core::Entity playerEntity = gameManager_.GetEntityFromPlayerNumber(playerNumber);
		const auto& playerBody = lastValidatePhysicsManager_.GetBody(playerEntity);

		const auto pos = playerBody.position;
		const auto* posPtr = reinterpret_cast<const PhysicsState*>(&pos);
		//Adding position
		for (size_t i = 0; i < sizeof(core::Vec2f) / sizeof(PhysicsState); i++)
		{
			state += posPtr[i];
		}

		//Adding velocity
		const auto velocity = playerBody.velocity;
		const auto* velocityPtr = reinterpret_cast<const PhysicsState*>(&velocity);
		for (size_t i = 0; i < sizeof(core::Vec2f) / sizeof(PhysicsState); i++)
		{
			state += velocityPtr[i];
		}
		return state;
	}

	void RollbackManager::SpawnArena(core::Entity entity, int i)
	{

			RigidBody body;
			Box box;
			Wall wall;
			auto pos = core::Vec2f::one();
			if (i == 0)
			{
				body.position = core::Vec2f(1.0f, -3.0f - 0.5f);
				box.extends = core::Vec2f(100.0f, 0.24f);
				pos = core::Vec2f(1.0f, -3.0f - 0.5f);

			}
			else if (i == 3)
			{
				body.position = core::Vec2f(1.0f,  - 2.5f);
				box.extends = core::Vec2f(100.0f, 0.24f);
				pos = core::Vec2f(1.0f, -3.0f - 0.5f);

			}
			else
			{
				body.position = core::Vec2f((3.0f) * (1.0f - 2.0f * static_cast<float>(i % 2)), 1.0f);
				box.extends = core::Vec2f(0.24f, 100.0f);
				pos = core::Vec2f(( 3.0f) * (1.0f - 2.0f * static_cast<float>(i % 2)), 1.0f);
			}

			wall.wallType = WallType::WallDouble;
			wall.remainingTime = 500.0f;
			box.collisionType = CollisionType::STATIC;
			box.layer = CollisionLayer::WALL;
			box.collideWithSame = false;

			wall.wallType = WallType::WallDouble;
			wall.remainingTime = 500.0f;
			currentWallManager_.AddComponent(entity);
			currentWallManager_.SetComponent(entity, wall);

			currentPhysicsManager_.AddBody(entity);
			currentPhysicsManager_.SetBody(entity, body);
			currentPhysicsManager_.AddBox(entity);
			currentPhysicsManager_.SetBox(entity, box);

			lastValidatePhysicsManager_.AddBody(entity);
			lastValidatePhysicsManager_.SetBody(entity, body);
			lastValidatePhysicsManager_.AddBox(entity);
			lastValidatePhysicsManager_.SetBox(entity, box);

			currentTransformManager_.AddComponent(entity);
			currentTransformManager_.SetPosition(entity, pos);
			currentTransformManager_.SetScale(entity, core::Vec2f::one());
	}


	void RollbackManager::SpawnPlayer(PlayerNumber playerNumber, core::Entity entity, core::Vec2f position)
	{

#ifdef TRACY_ENABLE
		ZoneScoped;
#endif
		RigidBody playerBody;
		playerBody.position = position;
		Box playerBox;
		playerBox.extends = core::Vec2f::one() * 0.25f;
		playerBox.layer = CollisionLayer::PLAYER;
		playerBox.collideWithSame = true;

		PlayerCharacter playerCharacter;
		playerCharacter.playerNumber = playerNumber;

		currentPlayerManager_.AddComponent(entity);
		currentPlayerManager_.SetComponent(entity, playerCharacter);

		currentPhysicsManager_.AddBody(entity);
		currentPhysicsManager_.SetBody(entity, playerBody);
		currentPhysicsManager_.AddBox(entity);
		currentPhysicsManager_.SetBox(entity, playerBox);

		lastValidatePlayerManager_.AddComponent(entity);
		lastValidatePlayerManager_.SetComponent(entity, playerCharacter);

		lastValidatePhysicsManager_.AddBody(entity);
		lastValidatePhysicsManager_.SetBody(entity, playerBody);
		lastValidatePhysicsManager_.AddBox(entity);
		lastValidatePhysicsManager_.SetBox(entity, playerBox);

		currentTransformManager_.AddComponent(entity);
		currentTransformManager_.SetPosition(entity, position);
		currentTransformManager_.SetRotation(entity, core::Degree(45.0f));
	}

	PlayerInput RollbackManager::GetInputAtFrame(PlayerNumber playerNumber, Frame frame) const
	{
		gpr_assert(currentFrame_ - frame < inputs_[playerNumber].size(),
			"Trying to get input too far in the past");
		return inputs_[playerNumber][currentFrame_ - frame];
	}



	void RollbackManager::OnTrigger(core::Entity entity1, core::Entity entity2)
	{
		const std::function<void(const PlayerCharacter&, core::Entity, const Bullet&, core::Entity)> ManageCollision =
			[this](const auto& player, auto playerEntity, const auto& bullet, auto bulletEntity)
		{
			if (player.playerNumber != bullet.playerNumber)
			{
				gameManager_.DestroyBullet(bulletEntity);
				//lower health point
				auto playerCharacter = currentPlayerManager_.GetComponent(playerEntity);
				if (playerCharacter.invincibilityTime <= 0.0f)
				{
					core::LogDebug(fmt::format("Player {} is hit by bullet", playerCharacter.playerNumber));
					--playerCharacter.health;
					playerCharacter.invincibilityTime = playerInvincibilityPeriod;
				}
				currentPlayerManager_.SetComponent(playerEntity, playerCharacter);
			}
		};
		if (entityManager_.HasComponent(entity1, static_cast<core::EntityMask>(ComponentType::PLAYER_CHARACTER)) &&
			entityManager_.HasComponent(entity2, static_cast<core::EntityMask>(ComponentType::BULLET)))
		{
			const auto& player = currentPlayerManager_.GetComponent(entity1);
			const auto& bullet = currentBulletManager_.GetComponent(entity2);
			ManageCollision(player, entity1, bullet, entity2);

		}
		if (entityManager_.HasComponent(entity2, static_cast<core::EntityMask>(ComponentType::PLAYER_CHARACTER)) &&
			entityManager_.HasComponent(entity1, static_cast<core::EntityMask>(ComponentType::BULLET)))
		{
			const auto& player = currentPlayerManager_.GetComponent(entity2);
			const auto& bullet = currentBulletManager_.GetComponent(entity1);
			ManageCollision(player, entity2, bullet, entity1);
		}
	}

	void RollbackManager::SpawnBullet(PlayerNumber playerNumber, core::Entity entity, core::Vec2f position, core::Vec2f velocity)
	{
		createdEntities_.push_back({ entity, testedFrame_ });

		RigidBody bulletBody;
		bulletBody.position = position;
		bulletBody.velocity = velocity;
		Box bulletBox;
		bulletBox.extends = core::Vec2f::one() * bulletScale * 0.5f;

		currentBulletManager_.AddComponent(entity);
		currentBulletManager_.SetComponent(entity, { bulletPeriod, playerNumber });

		currentPhysicsManager_.AddBody(entity);
		currentPhysicsManager_.SetBody(entity, bulletBody);
		currentPhysicsManager_.AddBox(entity);
		currentPhysicsManager_.SetBox(entity, bulletBox);

		currentTransformManager_.AddComponent(entity);
		currentTransformManager_.SetPosition(entity, position);
		currentTransformManager_.SetScale(entity, core::Vec2f::one() * bulletScale);
		currentTransformManager_.SetRotation(entity, core::Degree(0.0f));
	}

	void RollbackManager::DestroyEntity(core::Entity entity)
	{

#ifdef TRACY_ENABLE
		ZoneScoped;
#endif
		//we don't need to save a bullet that has been created in the time window
		if (std::find_if(createdEntities_.begin(), createdEntities_.end(), [entity](auto newEntity)
			{
				return newEntity.entity == entity;
			}) != createdEntities_.end())
		{
			entityManager_.DestroyEntity(entity);
			return;
		}
			entityManager_.AddComponent(entity, static_cast<core::EntityMask>(ComponentType::DESTROYED));
	}
}
