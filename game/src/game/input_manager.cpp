#include "game/input_manager.h"
#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/Mouse.hpp"

namespace game
{
    
PlayerInput GetPlayerInput(int index)
{
    switch(index)
    {
    case 0:
    {
        PlayerInput clientInput1 = 0;
        clientInput1 = clientInput1 | (sf::Keyboard::isKeyPressed(sf::Keyboard::A) ?
                PlayerInputEnum::PlayerInput::LEFT : PlayerInputEnum::PlayerInput::NONE);
        clientInput1 = clientInput1 | (sf::Keyboard::isKeyPressed(sf::Keyboard::D) ?
                PlayerInputEnum::PlayerInput::RIGHT : PlayerInputEnum::PlayerInput::NONE);
        clientInput1 = clientInput1 | (sf::Keyboard::isKeyPressed(sf::Keyboard::W) ?
                PlayerInputEnum::PlayerInput::UP : PlayerInputEnum::PlayerInput::NONE);
        clientInput1 = clientInput1 | (sf::Keyboard::isKeyPressed(sf::Keyboard::S) ?
                PlayerInputEnum::PlayerInput::BUILD : PlayerInputEnum::PlayerInput::NONE);
        clientInput1 = clientInput1 | (sf::Keyboard::isKeyPressed(sf::Keyboard::Q) ?
            PlayerInputEnum::PlayerInput::BM_LEFT : PlayerInputEnum::PlayerInput::NONE);
        clientInput1 = clientInput1 | (sf::Keyboard::isKeyPressed(sf::Keyboard::E) ?
            PlayerInputEnum::PlayerInput::BM_RIGHT : PlayerInputEnum::PlayerInput::NONE);
        


        return clientInput1;
    }
    case 1:
    {
        PlayerInput clientInput2 = 0;
        clientInput2 = clientInput2 | (sf::Keyboard::isKeyPressed(sf::Keyboard::J) ?
                PlayerInputEnum::PlayerInput::LEFT : PlayerInputEnum::PlayerInput::NONE);
        clientInput2 = clientInput2 | (sf::Keyboard::isKeyPressed(sf::Keyboard::L) ?
                PlayerInputEnum::PlayerInput::RIGHT : PlayerInputEnum::PlayerInput::NONE);
        clientInput2 = clientInput2 | (sf::Keyboard::isKeyPressed(sf::Keyboard::I) ?
                PlayerInputEnum::PlayerInput::UP : PlayerInputEnum::PlayerInput::NONE);
        clientInput2 = clientInput2 | (sf::Keyboard::isKeyPressed(sf::Keyboard::K) ?
                PlayerInputEnum::PlayerInput::BUILD : PlayerInputEnum::PlayerInput::NONE);
        clientInput2 = clientInput2 | (sf::Keyboard::isKeyPressed(sf::Keyboard::U) ?
            PlayerInputEnum::PlayerInput::BM_LEFT : PlayerInputEnum::PlayerInput::NONE);
        clientInput2 = clientInput2 | (sf::Keyboard::isKeyPressed(sf::Keyboard::O) ?
            PlayerInputEnum::PlayerInput::BM_RIGHT : PlayerInputEnum::PlayerInput::NONE);
        

        return clientInput2;
    }
    default:
        break;
    }
    return 0;
   
}
} // namespace game
