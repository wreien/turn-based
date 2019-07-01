#include "battle/battlesystem.h"

#include <algorithm>

#include "battle/battleview.h"
#include "battle/controller.h"
#include "battle/entity.h"
#include "util/overload.h"

namespace battle {

BattleSystem::BattleSystem(const std::vector<EntityRef>& blues,
                           const std::vector<EntityRef>& reds) {
    combatants.reserve(blues.size() + reds.size());

    const auto push = [this](Team team, const EntityRef& e) {
        auto dt = diff(e.get());
        combatants.emplace_back(team, e, 0, dt);
        turn_order.push(newRef(combatants.size() - 1));
    };

    for (const auto& e : blues)
        push(Team::Blue, e);
    for (const auto& e : reds)
        push(Team::Red, e);
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

bool BattleSystem::CombatantRefCmp::operator()
        (CombatantRef a, CombatantRef b) const noexcept
{
    auto& ca = a.get();
    auto& cb = b.get();
    if (ca.next_turn == cb.next_turn)
        return a.index > b.index;
    return ca.next_turn > cb.next_turn;
}

BattleSystem::CombatantRef BattleSystem::newRef(std::size_t index) {
    return CombatantRef{ index, &combatants };
}

BattleSystem::Timepoint BattleSystem::diff(const Entity* e) const noexcept {
    return 100.0 / e->getStats().react;
}

void BattleSystem::pushCombatant(Team team, EntityRef e) {
    auto dt = diff(e.get());
    auto now = turn_order.empty() ? 0 : turn_order.top()->next_turn;
    combatants.emplace_back(team, std::move(e), now, now + dt);
    turn_order.push(newRef(combatants.size() - 1));
}

void BattleSystem::gotoNextTurn() noexcept {
    auto ref = turn_order.top();
    turn_order.pop();
    ref->last_turn  = ref->next_turn;
    ref->next_turn += diff(ref->entity.get());
    turn_order.push(ref);
}


// Actually run the game

TurnInfo BattleSystem::doTurn() {
    // skip dead people
    Combatant& c = *turn_order.top();
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

    std::visit(util::overload{
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

bool BattleSystem::isDone() const noexcept {
    const auto is_dead = [](const Entity* e){ return e->isDead(); };

    const auto red  = teamMembersOf(Team::Red);
    const auto blue = teamMembersOf(Team::Blue);

    return std::all_of(std::begin(red), std::end(red), is_dead)
        || std::all_of(std::begin(blue), std::end(blue), is_dead);
}

}
