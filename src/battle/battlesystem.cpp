#include "battlesystem.h"

#include <algorithm>
#include "controller.h"
#include "battleview.h"
#include "entity.h"
#include "../overload.h"

namespace battle {

BattleSystem::BattleSystem(const std::vector<EntityRef>& blues,
                           const std::vector<EntityRef>& reds)
    : combatants{}
    , turn_order{}
    , current_turn{ std::end(turn_order) }
{
    combatants.reserve(blues.size() + reds.size());
    turn_order.reserve(blues.size() + reds.size());

    CombatantRef r = 0;
    for (const auto& e : blues) {
        combatants.emplace_back(Team::Blue, e);
        turn_order.push_back(r++);
    }
    for (const auto& e : reds) {
        combatants.emplace_back(Team::Red, e);
        turn_order.push_back(r++);
    }

    sortTurnOrder();
}


std::vector<Entity*> BattleSystem::getEntities(Team team) {
    std::vector<Entity*> entities;
    for (auto&& c : combatants)
        if (c.team == team)
            entities.push_back(c.entity.get());
    return entities;
}

std::vector<const Entity*> BattleSystem::getEntities(Team team) const {
    std::vector<const Entity*> entities;
    for (auto&& c : combatants)
        if (c.team == team)
            entities.push_back(c.entity.get());
    return entities;
}

Team BattleSystem::getTeam(const Entity& e) const {
    auto it = std::find_if(
        std::begin(combatants), std::end(combatants),
        [&e](auto&& c){ return c.entity.get() == &e; }
    );
    if (it == std::end(combatants))
        throw std::invalid_argument("BattleSystem::getTeam: entity not found");
    return it->team;
}


// Turn order business

void BattleSystem::sortTurnOrder() {
    auto sortfn = [&c = combatants](CombatantRef a, CombatantRef b) {
        auto aspd = c[a].entity->getStats().speed;
        auto bspd = c[b].entity->getStats().speed;
        if (aspd != bspd)
            return aspd > bspd;
        if (c[a].team != c[b].team)
            return static_cast<int>(c[a].team) < static_cast<int>(c[b].team);
        return a < b; // yes, ref compare - want order in the master list
    };
    std::sort(std::begin(turn_order), std::end(turn_order), sortfn);
    current_turn = std::begin(turn_order);
}

void BattleSystem::addEntryToTurnOrder() {
    auto dist = std::distance(std::begin(turn_order), current_turn);
    turn_order.push_back(turn_order.size());
    current_turn = std::begin(turn_order) + dist;
}

void BattleSystem::gotoNextTurn() {
    if (++current_turn == std::end(turn_order))
        sortTurnOrder();
}


// Actually run the game

TurnInfo BattleSystem::doTurn() {
    MessageLogger logger;
    TurnInfo info { false, nullptr, {} };

    auto& c = combatants[*current_turn];
    auto& controller = c.entity->getController();

    BattleView view {
        getEntities(c.team == Team::Blue ? Team::Blue : Team::Red),
        getEntities(c.team == Team::Blue ? Team::Red : Team::Blue)
    };

    Action act = controller.go(view);

    bool turnFinished = false;
    std::visit(overload{
        [&](action::Defend){
            // at this stage, do nothing ;)
            logger.appendMessage(message::Defended{ *c.entity });
            turnFinished = true;
        },
        [&](action::Flee){
            // at this stage, do nothing ;)
            logger.appendMessage(message::Fled{ *c.entity, true });
            turnFinished = true;
        },
        [&](action::Skill& s){
            // TODO: get the results
            auto it = std::find_if(
                std::begin(combatants), std::end(combatants),
                [&s](auto&& c){ return c.entity.get() == &s.target; }
            );
            if (it != std::end(combatants)) {
                s.skill->use(logger, *c.entity, *it->entity, getEntities(it->team));
                turnFinished = true;
            } else {
                throw std::invalid_argument(
                        "BattleSystem::doTurn/Skill: entity not found");
            }
        },
        [&](action::UserChoice user) {
            info.need_user_input = true;
            info.controller = &user.controller;
            turnFinished = false;
        }
    }, act);

    if (turnFinished) {
        if (!c.entity->isDead())
            c.entity->processTurnEnd(logger);
        gotoNextTurn();
    }

    info.messages = logger.pop();
    return info;
}

bool BattleSystem::isDone() const {
    // TODO
    return false;
}

}
