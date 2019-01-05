#ifndef BATTLE_BATTLEVIEW_H_INCLUDED
#define BATTLE_BATTLEVIEW_H_INCLUDED

#include <vector>
#include "entity.h"

namespace battle {


// Observe the current state of the battle
struct BattleView {
    std::vector<Entity*> allies;
    std::vector<Entity*> enemies;
};


}

#endif // BATTLE_BATTLEVIEW_H_INCLUDED
