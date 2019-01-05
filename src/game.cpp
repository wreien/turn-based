#include "game.h"

#include <cmath>
#include <array>

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

Game::Game(unsigned int width, unsigned int height)
    : window_width{ width }
    , window_height{ height }
{
    // construct
}

void Game::processInput() {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
        is_done = true;
    // TODO
}

void Game::render(sf::RenderWindow& window) const {
    window.clear();

    window.display();
}
