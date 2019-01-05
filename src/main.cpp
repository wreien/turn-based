#include <SFML/Graphics.hpp>
#include <SFML/GpuPreference.hpp>
#include "game.h"

int main() {
    const auto mode = sf::VideoMode::getDesktopMode();
    sf::RenderWindow window { mode, "Turn Based Demo", sf::Style::Fullscreen };

    Game game { mode.width, mode.height };

    while (window.isOpen() && !game.is_done) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::Resized) {
                game.window_width  = event.size.width;
                game.window_height = event.size.height;
            }
        }

        game.processInput();
        game.render(window);
    }

    window.close();
    return 0;
}

// recommend usage of dedicated graphics on dual-card systems
SFML_DEFINE_DISCRETE_GPU_PREFERENCE
