
#include "game/game_manager.h"

#include "utils/log.h"

#include "maths/basic.h"
#include "utils/conversion.h"

#include <fmt/format.h>
#include <imgui.h>
#include <chrono>



#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#endif

namespace game
{

GameManager::GameManager() :
    transformManager_(entityManager_),
    rollbackManager_(*this, entityManager_)
{
    playerEntityMap_.fill(core::INVALID_ENTITY);

    // 1 for floor 2/3 for walls
    for (int i = 0; i<3; i++)
    {
	    const auto entity = entityManager_.CreateEntity();

    	transformManager_.AddComponent(entity);
    	transformManager_.SetPosition(entity, core::Vec2f::one());
    	transformManager_.SetScale(entity, core::Vec2f(1.0f, 1.0f));
    
    	rollbackManager_.SpawnArena(entity, i);
    }
}



std::pair<core::Entity, core::Entity> GameManager::SpawnFullPlayer(PlayerNumber playerNumber, core::Vec2f position)
{
    if (GetEntityFromPlayerNumber(playerNumber) != core::INVALID_ENTITY)
        exit(0);
    core::LogDebug("[GameManager] Spawning new player");
    const auto entity = entityManager_.CreateEntity();
    playerEntityMap_[playerNumber] = entity;

    const auto spawnerEntity = entityManager_.CreateEntity();
    transformManager_.AddComponent(spawnerEntity);


    transformManager_.AddComponent(entity);
    transformManager_.SetPosition(entity, position);
    
    rollbackManager_.SpawnPlayer(playerNumber, entity, spawnerEntity, position);
    return std::make_pair(entity, spawnerEntity);
}

core::Entity GameManager::GetEntityFromPlayerNumber(PlayerNumber playerNumber) const
{
    return playerEntityMap_[playerNumber];
}


void GameManager::SetPlayerInput(PlayerNumber playerNumber, PlayerInput playerInput, std::uint32_t inputFrame)
{
    if (playerNumber == INVALID_PLAYER)
        return;

    rollbackManager_.SetPlayerInput(playerNumber, playerInput, inputFrame);

}
void GameManager::Validate(Frame newValidateFrame)
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    if (rollbackManager_.GetCurrentFrame() < newValidateFrame)
    {
        rollbackManager_.StartNewFrame(newValidateFrame);
    }
    rollbackManager_.ValidateFrame(newValidateFrame);
}

core::Entity GameManager::SpawnWall(PlayerNumber, core::Vec2f position)
{
    const core::Entity entity = entityManager_.CreateEntity();

    transformManager_.AddComponent(entity);
    transformManager_.SetPosition(entity, position);
    transformManager_.SetScale(entity, core::Vec2f(1.5, 1.5));
    transformManager_.SetRotation(entity, core::Degree(0.0f));
    rollbackManager_.SpawnWall(entity, position);
    return entity;
}

PlayerNumber GameManager::CheckWinner() const
{
    int alivePlayer = 0;
    PlayerNumber winner = INVALID_PLAYER;
    const auto& playerManager = rollbackManager_.GetPlayerCharacterManager();
    for (core::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
    {
        if (!entityManager_.HasComponent(entity, static_cast<core::EntityMask>(ComponentType::PLAYER_CHARACTER)))
            continue;
        const auto& player = playerManager.GetComponent(entity);
        if (player.health > 0)
        {
            alivePlayer++;
            winner = player.playerNumber;
        }
    }

    return alivePlayer == 1 ? winner : INVALID_PLAYER;
}

void GameManager::WinGame(PlayerNumber winner)
{
    winner_ = winner;
}

ClientGameManager::ClientGameManager(PacketSenderInterface& packetSenderInterface) :
    GameManager(),
    packetSenderInterface_(packetSenderInterface),
    spriteManager_(entityManager_, transformManager_)
{
}

void ClientGameManager::Begin()
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    //load textures
    if (!shipTexture_.loadFromFile("data/sprites/ship.png"))
    {
        core::LogError("Could not load ship sprite");
    }
    if (!wallTexture_.loadFromFile("data/sprites/wall.png"))
    {
        core::LogError("Could not load wall sprite");
    }
    if (!wallSpawnerTexture_.loadFromFile("data/sprites/wallSpawner.png"))
    {
        core::LogError("Could not load wallspawner sprite");
    }
    //load fonts
    if (!font_.loadFromFile("data/fonts/8-bit-hud.ttf"))
    {
        core::LogError("Could not load font");
    }
    textRenderer_.setFont(font_);
    starBackground_.Init();
}

void ClientGameManager::Update(sf::Time dt)
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    if (state_ & STARTED)
    {
        rollbackManager_.SimulateToCurrentFrame();
        //Copy rollback transform position to our own
        for (core::Entity entity = 0; entity < entityManager_.GetEntitiesSize(); entity++)
        {
            if (entityManager_.HasComponent(entity,
                static_cast<core::EntityMask>(ComponentType::PLAYER_CHARACTER) |
                static_cast<core::EntityMask>(core::ComponentType::SPRITE)))

            if (entityManager_.HasComponent(entity, static_cast<core::EntityMask>(core::ComponentType::TRANSFORM)))
            {
                transformManager_.SetPosition(entity, rollbackManager_.GetTransformManager().GetPosition(entity));
                transformManager_.SetScale(entity, rollbackManager_.GetTransformManager().GetScale(entity));
            }
        }
    }
    fixedTimer_ += dt.asSeconds();
    while (fixedTimer_ > fixedPeriod)
    {
        FixedUpdate();
        fixedTimer_ -= fixedPeriod;
    }
}

void ClientGameManager::End()
{
}

void ClientGameManager::SetWindowSize(sf::Vector2u windowsSize)
{
    windowSize_ = windowsSize;
    const sf::FloatRect visibleArea(0.0f, 0.0f,
        static_cast<float>(windowSize_.x),
        static_cast<float>(windowSize_.y));
    originalView_ = sf::View(visibleArea);
    spriteManager_.SetWindowSize(sf::Vector2f(windowsSize));
    spriteManager_.SetCenter(sf::Vector2f(windowsSize) / 2.0f);
    auto& currentPhysicsManager = rollbackManager_.GetCurrentPhysicsManager();
    currentPhysicsManager.SetCenter(sf::Vector2f(windowsSize) / 2.0f);
    currentPhysicsManager.SetWindowSize(sf::Vector2f(windowsSize));


}

void ClientGameManager::Draw(sf::RenderTarget& target)
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    UpdateCameraView();
    target.setView(cameraView_);

    starBackground_.Draw(target);
    spriteManager_.Draw(target);

    if(drawPhysics_)
    {
        auto& currentPhysicsManager = rollbackManager_.GetCurrentPhysicsManager();
        currentPhysicsManager.Draw(target);
    }

    // Draw texts on screen
    target.setView(originalView_);
    if (state_ & FINISHED)
    {
        if (winner_ == GetPlayerNumber())
        {
            const std::string winnerText = fmt::format("You won!");
            textRenderer_.setFillColor(sf::Color::White);
            textRenderer_.setString(winnerText);
            textRenderer_.setCharacterSize(32);
            const auto textBounds = textRenderer_.getLocalBounds();
            textRenderer_.setPosition(static_cast<float>(windowSize_.x) / 2.0f - textBounds.width / 2.0f,
                static_cast<float>(windowSize_.y) / 2.0f - textBounds.height / 2.0f);
            target.draw(textRenderer_);
        }
        else if (winner_ != INVALID_PLAYER)
        {
            const std::string winnerText = fmt::format("P{} won!", winner_ + 1);
            textRenderer_.setFillColor(sf::Color::White);
            textRenderer_.setString(winnerText);
            textRenderer_.setCharacterSize(32);
            const auto textBounds = textRenderer_.getLocalBounds();
            textRenderer_.setPosition(static_cast<float>(windowSize_.x) / 2.0f - textBounds.width / 2.0f,
                static_cast<float>(windowSize_.y) / 2.0f - textBounds.height / 2.0f);
            target.draw(textRenderer_);
        }
        else
        {
            const std::string errorMessage = fmt::format("Error with other players");
            textRenderer_.setFillColor(sf::Color::Red);
            textRenderer_.setString(errorMessage);
            textRenderer_.setCharacterSize(32);
            const auto textBounds = textRenderer_.getLocalBounds();
            textRenderer_.setPosition(static_cast<float>(windowSize_.x) / 2.0f - textBounds.width / 2.0f,
                static_cast<float>(windowSize_.y) / 2.0f - textBounds.height / 2.0f);
            target.draw(textRenderer_);
        }
    }
    if (!(state_ & STARTED))
    {
        if (startingTime_ != 0)
        {
            using namespace std::chrono;
            unsigned long long ms = duration_cast<milliseconds>(
                system_clock::now().time_since_epoch()
                ).count();
            if (ms < startingTime_)
            {
                const std::string countDownText = fmt::format("Starts in {}", ((startingTime_ - ms) / 1000 + 1));
                textRenderer_.setFillColor(sf::Color::White);
                textRenderer_.setString(countDownText);
                textRenderer_.setCharacterSize(32);
                const auto textBounds = textRenderer_.getLocalBounds();
                textRenderer_.setPosition(static_cast<float>(windowSize_.x) / 2.0f - textBounds.width / 2.0f,
                    static_cast<float>(windowSize_.y) / 2.0f - textBounds.height / 2.0f);
                target.draw(textRenderer_);
            }
        }
    }
    else
    {
        std::string health;
        const auto& playerManager = rollbackManager_.GetPlayerCharacterManager();
        for (PlayerNumber playerNumber = 0; playerNumber < maxPlayerNmb; playerNumber++)
        {
            const auto playerEntity = GetEntityFromPlayerNumber(playerNumber);
            if (playerEntity == core::INVALID_ENTITY)
            {
                continue;
            }
            health += fmt::format("P{} health: {} ", playerNumber + 1, playerManager.GetComponent(playerEntity).health);
        }
        textRenderer_.setFillColor(sf::Color::White);
        textRenderer_.setString(health);
        textRenderer_.setPosition(10, 10);
        textRenderer_.setCharacterSize(20);
        target.draw(textRenderer_);
    }

}

void ClientGameManager::SetClientPlayer(PlayerNumber clientPlayer)
{
    clientPlayer_ = clientPlayer;
}

void game::ClientGameManager::DrawArena()
{
	const core::Entity wall = entityManager_.CreateEntity();

    transformManager_.AddComponent(wall);
    transformManager_.SetScale(wall, core::Vec2f::one() * 4);

    spriteManager_.AddComponent(wall);
    spriteManager_.SetTexture(wall, shipTexture_);
    spriteManager_.SetOrigin(wall, sf::Vector2f(shipTexture_.getSize()) / 2.0f);
}

void ClientGameManager::SpawnPlayer(PlayerNumber playerNumber, core::Vec2f position)
{
    core::LogDebug(fmt::format("Spawn player: {}", playerNumber));

    auto pair = SpawnFullPlayer(playerNumber, position);
    const auto entity = pair.first;
    
    spriteManager_.AddComponent(entity);
    spriteManager_.SetTexture(entity, shipTexture_);
    spriteManager_.SetOrigin(entity, sf::Vector2f(shipTexture_.getSize()) / 2.0f);
    spriteManager_.SetColor(entity, playerColors[playerNumber]);

    
}

core::Entity game::ClientGameManager::SpawnWall(PlayerNumber playerNumber, core::Vec2f position)
{
    core::LogDebug(fmt::format("Spawn wall from player: {}", playerNumber));
    
    const auto entity = GameManager::SpawnWall(playerNumber, position);
    
	return entity;
}

void ClientGameManager::FixedUpdate()
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    if (!(state_ & STARTED))
    {
        if (startingTime_ != 0)
        {
            using namespace std::chrono;
            const auto ms = duration_cast<duration<unsigned long long, std::milli>>(
                system_clock::now().time_since_epoch()
                ).count();
            if (ms > startingTime_)
            {
                state_ = state_ | STARTED;
            }
            else
            {
                return;
            }
        }
        else
        {
            return;
        }
    }
    if (state_ & FINISHED)
    {
        return;
    }

    //We send the player inputs when the game started
    const auto playerNumber = GetPlayerNumber();
    if (playerNumber == INVALID_PLAYER)
    {
        //We still did not receive the spawn player packet, but receive the start game packet
        core::LogWarning(fmt::format("Invalid Player Entity in {}:line {}", __FILE__, __LINE__));
        return;
    }
    const auto& inputs = rollbackManager_.GetInputs(playerNumber);
    auto playerInputPacket = std::make_unique<PlayerInputPacket>();
    playerInputPacket->playerNumber = playerNumber;
    playerInputPacket->currentFrame = core::ConvertToBinary(currentFrame_);
    for (size_t i = 0; i < playerInputPacket->inputs.size(); i++)
    {
        if (i > currentFrame_)
        {
            break;
        }

        playerInputPacket->inputs[i] = inputs[i];
    }
    packetSenderInterface_.SendUnreliablePacket(std::move(playerInputPacket));


    currentFrame_++;
    rollbackManager_.StartNewFrame(currentFrame_);
}


void ClientGameManager::SetPlayerInput(PlayerNumber playerNumber, PlayerInput playerInput, std::uint32_t inputFrame)
{
    if (playerNumber == INVALID_PLAYER)
        return;
    GameManager::SetPlayerInput(playerNumber, playerInput, inputFrame);
}

void ClientGameManager::StartGame(unsigned long long int startingTime)
{
    core::LogDebug(fmt::format("Start game at starting time: {}", startingTime));
    startingTime_ = startingTime;
}

void ClientGameManager::DrawImGui()
{
    ImGui::Text(state_ & STARTED ? "Game has started" : "Game has not started");
    if (startingTime_ != 0)
    {
        ImGui::Text("Starting Time: %llu", startingTime_);
        using namespace std::chrono;
        unsigned long long ms = duration_cast<milliseconds>(
            system_clock::now().time_since_epoch()
            ).count();
        ImGui::Text("Current Time: %llu", ms);
    }
    ImGui::Checkbox("Draw Physics", &drawPhysics_);
}

void ClientGameManager::ConfirmValidateFrame(Frame newValidateFrame,
    const std::array<PhysicsState, maxPlayerNmb>& physicsStates)
{
    if (newValidateFrame < rollbackManager_.GetLastValidateFrame())
    {
        core::LogWarning(fmt::format("New validate frame is too old"));
        return;
    }
    for (PlayerNumber playerNumber = 0; playerNumber < maxPlayerNmb; playerNumber++)
    {
        if (rollbackManager_.GetLastReceivedFrame(playerNumber) < newValidateFrame)
        {
            
            core::LogWarning(fmt::format("Trying to validate frame {} while playerNumber {} is at input frame {}, client player {}",
                newValidateFrame,
                playerNumber + 1,
                rollbackManager_.GetLastReceivedFrame(playerNumber),
                GetPlayerNumber()+1));
            

            return;
        }
    }
    rollbackManager_.ConfirmFrame(newValidateFrame, physicsStates);
}

void ClientGameManager::WinGame(PlayerNumber winner)
{
    GameManager::WinGame(winner);
    state_ = state_ | FINISHED;
}

void ClientGameManager::UpdateCameraView()
{
    if ((state_ & STARTED) != STARTED)
    {
        cameraView_ = originalView_;
        return;
    }

    cameraView_ = originalView_;
    float currentZoom = 1.0f;
    cameraView_.zoom(currentZoom);

}
}
