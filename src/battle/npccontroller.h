#ifndef BATTLE_NPCCONTROLLER_H_INCLUDED
#define BATTLE_NPCCONTROLLER_H_INCLUDED

#include "controller.h"

namespace battle {

class Entity;

// Generic AI controller for entities
class NPCController : public Controller {
public:
    static constexpr bool nest_controller = false;

    explicit NPCController(Entity& entity);
    [[nodiscard]] virtual Action go(const BattleView& view) override;

private:
    Entity& entity;
};

}

#endif // BATTLE_NPCCONTROLLER_H_INCLUDED
