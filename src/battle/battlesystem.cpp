#include "battlesystem.h"

#include <algorithm>
#include "controller.h"
#include "battleview.h"
#include "../util.h"

namespace battle {

BattleSystem::BattleSystem()
    : combatants{}
    , turn_order{}
    , current_turn{ std::end(turn_order) }
{
}

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

    current_turn = std::begin(turn_order);
}


std::vector<Entity*> BattleSystem::getTeam(Team team) {
    std::vector<Entity*> entities;
    for (auto&& c : combatants)
        if (c.team == team)
            entities.push_back(c.entity.get());
    return entities;
}

std::vector<const Entity*> BattleSystem::getTeam(Team team) const {
    std::vector<const Entity*> entities;
    for (auto&& c : combatants)
        if (c.team == team)
            entities.push_back(c.entity.get());
    return entities;
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
    auto& c = combatants[*current_turn];
    auto& controller = c.entity->getController();

    BattleView view {
        getTeam(c.team == Team::Blue ? Team::Blue : Team::Red),
        getTeam(c.team == Team::Blue ? Team::Red : Team::Blue)
    };

    Action act = controller.go(view);

    bool turnFinished = true;
    using namespace action;
    std::visit(overload{
        [&](Defend){
            // at this stage, do nothing ;)
        },
        [&](Flee){
            // at this stage, do nothing ;)
            turnFinished = false;
        },
        [&](TargetedSkill){
            // TODO: do the skill
        },
        [&](UntargetedSkill){
            // TODO: do the skill
        },
        [&](UserChoice){
            turnFinished = false;
        }
    }, act);

    if (turnFinished)
        gotoNextTurn();
    return { act, *c.entity, c.team };
}

bool BattleSystem::isDone() const {
    // TODO
    return false;
}

}
