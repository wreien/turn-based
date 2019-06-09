#ifndef BATTLE_CONTROLLER_H_INCLUDED
#define BATTLE_CONTROLLER_H_INCLUDED

#include <vector>
#include "battle/action.h"

namespace battle {


struct BattleView;

// Interface for getting actions from an entity
class Controller {
public:
    virtual ~Controller() = 0;  // defined in entity.cpp

    // Given the current state of the battlefield, make a decision as to what to do
    //
    // Any action except `action::UserChoice' ends the parent entity's turn.
    [[nodiscard]] virtual Action go(const BattleView& view) = 0;
};


}

#endif // BATTLE_CONTROLLER_H_INCLUDED
