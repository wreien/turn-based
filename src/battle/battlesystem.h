#ifndef BATTLE_BATTLESYSTEM_H_INCLUDED
#define BATTLE_BATTLESYSTEM_H_INCLUDED

#include <vector>
#include <utility>
#include <memory>
#include "battle/messages.h"

namespace battle {

class Entity;
class PlayerController;

/// Which team the entity is on.
/// Order determines turn priority in case of a tie.
enum class Team {
    Blue,  ///< players
    Red,   ///< enemies
};

/// Contains information about the happenings of the last turn
struct TurnInfo {
    bool turn_finished; ///< whether the current entity's turn finished
    bool need_user_input; ///< whether we now need user input
    PlayerController* controller; ///< the user's controller (if needing user input)
    MessageLogger messages; ///< what happened since the last turn
};

/// Manages and runs the battle; the game loop, if you will.
class BattleSystem {
public:
    /// TODO: do I really want shared_ptr? -- this question can only
    /// be resolved once I work out how I'm interfacing the battle system
    /// with an outside game.
    using EntityRef = std::shared_ptr<Entity>;

    explicit BattleSystem(const std::vector<EntityRef>& blues,
                          const std::vector<EntityRef>& reds);

    // no copying
    BattleSystem(const BattleSystem&) = delete;
    BattleSystem& operator=(const BattleSystem&) = delete;

    void pushCombatant(Team team, EntityRef e) {
        addEntryToTurnOrder();
        combatants.emplace_back(team, e);
    }

    template <typename... Args>
    void emplaceCombatant(Team team, Args&&... args) {
        addEntryToTurnOrder();
        combatants.emplace_back(
            team,
            std::make_shared<Entity>(std::forward<Args>(args)...)
        );
    }

    /// Get the entities that are part of the specified team.
    [[nodiscard]] std::vector<Entity*> teamMembersOf(Team team) noexcept;
    [[nodiscard]] std::vector<const Entity*> teamMembersOf(Team team) const noexcept;

    [[nodiscard]] auto teamMembersOf(const Entity& e)
        { return teamMembersOf(teamOf(e)); }
    [[nodiscard]] auto teamMembersOf(const Entity& e) const
        { return teamMembersOf(teamOf(e)); }

    /// Get the team the specified entity belongs to
    Team teamOf(const Entity& e) const;

    /// Progress the battle.
    TurnInfo doTurn();

    /// Has the battle finished yet?
    bool isDone() const;

private:
    struct Combatant {
        Team team;
        EntityRef entity;

        Combatant(Team team, EntityRef entity)
            : team{ team }, entity{ entity }
        {}
    };

    /// Sort the turn order in order of speed.
    /// Points current_turn to the start of the new order.
    void sortTurnOrder();

    /// Adds a new entry to the turn order.
    /// Does not invalidate current_turn.
    void addEntryToTurnOrder();

    void gotoNextTurn();

    std::vector<Combatant> combatants;

    using CombatantRef = std::size_t;
    std::vector<CombatantRef> turn_order;
    std::vector<CombatantRef>::iterator current_turn;
};


}

#endif // BATTLE_BATTLESYSTEM_H_INCLUDED
