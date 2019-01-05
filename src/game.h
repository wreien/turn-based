#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED


namespace sf {
    class RenderWindow;
}


// encapsulates the game as whole
class Game {
public:
    Game(unsigned int width, unsigned int height);

    void processInput();
    void render(sf::RenderWindow& window) const;

    // data
    bool is_done = false;
    unsigned int window_width;
    unsigned int window_height;
};


#endif // GAME_H_INCLUDED
