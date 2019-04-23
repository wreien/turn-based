#ifndef BATTLE_BATTLEVIEW_H_INCLUDED
#define BATTLE_BATTLEVIEW_H_INCLUDED

#include <vector>

namespace battle {


class Entity;

// Observe the current state of the battle
// TODO: make more const correct (anyone can modify atm)
struct BattleView {
    std::vector<Entity*> allies;
    std::vector<Entity*> enemies;
};


}

#endif // BATTLE_BATTLEVIEW_H_INCLUDED
