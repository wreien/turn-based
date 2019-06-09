#include "battle/battlesystem.h"

#include <algorithm>

#include "battle/battleview.h"
#include "battle/controller.h"
#include "battle/entity.h"
#include "overload.h"

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


std::vector<Entity*> BattleSystem::teamMembersOf(Team team) noexcept {
    std::vector<Entity*> entities;
    for (auto&& c : combatants)
        if (c.team == team)
            entities.push_back(c.entity.get());
    return entities;
}

std::vector<const Entity*> BattleSystem::teamMembersOf(Team team) const noexcept {
    std::vector<const Entity*> entities;
    for (auto&& c : combatants)
        if (c.team == team)
            entities.push_back(c.entity.get());
    return entities;
}

Team BattleSystem::teamOf(const Entity& e) const {
    auto it = std::find_if(
        std::begin(combatants), std::end(combatants),
        [&e](auto&& c){ return c.entity.get() == &e; }
    );
    if (it == std::end(combatants))
        throw std::invalid_argument("BattleSystem::teamOf: entity not found");
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
    // skip dead people
    auto& c = combatants[*current_turn];
    TurnInfo info { true, false, nullptr, {} };
    if (c.entity->isDead()) {
        gotoNextTurn();
        return info;
    }

    BattleView view {
        teamMembersOf(c.team == Team::Blue ? Team::Blue : Team::Red),
        teamMembersOf(c.team == Team::Blue ? Team::Red : Team::Blue)
    };

    auto& controller = c.entity->getController();
    Action act = controller.go(view);

    std::visit(overload{
        [&info,c](action::Defend){
            // at this stage, do nothing ;)
            info.messages.appendMessage(message::Defended{ *c.entity });
            info.turn_finished = true;
        },
        [&info,c](action::Flee){
            // at this stage, do nothing ;)
            info.messages.appendMessage(message::Fled{ *c.entity, true });
            info.turn_finished = true;
        },
        [&info,c,this](action::Skill& s){
            auto it = std::find_if(
                std::begin(combatants), std::end(combatants),
                [&s](auto&& c){ return c.entity.get() == &s.target; }
            );
            if (it != std::end(combatants)) {
                s.skill->use(info.messages, *c.entity, *it->entity, *this);
                info.turn_finished = true;
            } else {
                throw std::invalid_argument(
                        "BattleSystem::doTurn/Skill: entity not found");
            }
        },
        [&info](action::UserChoice user) {
            info.turn_finished = false;
            info.need_user_input = true;
            info.controller = &user.controller;
        }
    }, act);

    if (info.turn_finished) {
        if (!c.entity->isDead())
            c.entity->processTurnEnd(info.messages);
        gotoNextTurn();
    }

    return info;
}

bool BattleSystem::isDone() const {
    const auto is_dead = [](const Entity* e){ return e->isDead(); };

    const auto red  = teamMembersOf(Team::Red);
    const auto blue = teamMembersOf(Team::Blue);

    return std::all_of(std::begin(red), std::end(red), is_dead)
        || std::all_of(std::begin(blue), std::end(blue), is_dead);
}

}
