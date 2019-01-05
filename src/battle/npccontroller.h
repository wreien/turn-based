#ifndef BATTLE_NPCCONTROLLER_H_INCLUDED
#define BATTLE_NPCCONTROLLER_H_INCLUDED

#include "controller.h"
#include "entity.h"

namespace battle {

// Generic AI controller for entities
class NPCController : public Controller {
public:
    explicit NPCController(Entity& entity);
    [[nodiscard]] virtual Action go(const BattleView& view) override;

private:
    Entity& entity;
};

}

#endif // BATTLE_NPCCONTROLLER_H_INCLUDED
