#ifndef BATTLE_BATTLESYSTEM_H_INCLUDED
#define BATTLE_BATTLESYSTEM_H_INCLUDED

#include <vector>
#include <utility>
#include <memory>
#include <queue>
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

    void pushCombatant(Team team, EntityRef e);

    template <typename... Args>
    void emplaceCombatant(Team team, Args&&... args) {
        auto ref = std::make_shared<Entity>(std::forward<Args>(args)...);
        pushCombatant(team, std::move(ref));
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
    bool isDone() const noexcept;

private:
    using Timepoint = double;
    struct Combatant {
        Team team;
        EntityRef entity;

        /// What time the next turn for this combatant happens
        Timepoint last_turn;
        Timepoint next_turn;

        // TODO: C++20 get rid of this unnecessary constructor
        Combatant(Team team, EntityRef entity, Timepoint last_turn, Timepoint next_turn)
            : team{ team }
            , entity{ entity }
            , last_turn{ last_turn }
            , next_turn{ next_turn }
        {}
    };

    struct CombatantRef {
        std::size_t index;
        std::vector<Combatant>* combatants;

        Combatant& get() const noexcept { return (*combatants)[index]; }
        Combatant* operator->() const noexcept { return &get(); }
        Combatant& operator*() const noexcept { return get(); }
    };

    struct CombatantRefCmp {
        bool operator()(CombatantRef a, CombatantRef b) const noexcept;
    };

    /// The list of all combatants, living and dead
    std::vector<Combatant> combatants = {};

    /// Create a ref referring to the combatant with given index
    CombatantRef newRef(std::size_t index);

    /// List of references to combatants, in order of who goes next
    std::priority_queue<
        CombatantRef, std::vector<CombatantRef>, CombatantRefCmp> turn_order = {};

    /// Takes the current turn at sticks it back into the queue
    void gotoNextTurn() noexcept;

    /// Get the timestep difference
    Timepoint diff(const Entity* e) const noexcept;
};


}

#endif // BATTLE_BATTLESYSTEM_H_INCLUDED
