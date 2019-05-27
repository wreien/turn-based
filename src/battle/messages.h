#ifndef BATTLE_MESSAGES_H_INCLUDED
#define BATTLE_MESSAGES_H_INCLUDED

#include <optional>
#include <string>
#include <variant>
#include <vector>
#include "skillref.h"
#include "stats.h"

namespace battle {


class Entity;

/// Collate different message types
/// TODO: make references pointers (or std::reference_wrapper)?
namespace message {

    /// A skill was used on a target
    struct SkillUsed {
        SkillRef skill;       ///< the skill that was used
        const Entity& source; ///< who used the skill
        const Entity& target; ///< the primary target of the skill
    };

    /// A skill missed the target
    struct Miss {
        const Entity& entity; ///< who did the skill miss
    };

    /// A skill scored a critical hit
    struct Critical {
        const Entity& entity; ///< who received the crit
    };

    /// One of an entity's visible pools was changed
    struct PoolChanged {
        const Entity& entity; ///< the entity in question
        Pool pool;            ///< the affected pool
        int old_value;        ///< the old value of the pool
        int new_value;        ///< the new value of the pool
    };

    /// An entity was afflicted or cured of a status effect
    /// (I don't really want to pass the whole effect, but we don't really need it)
    struct StatusEffect {
        const Entity& entity; ///< the entity in question
        std::string effect;   ///< the name of the effect (TODO: enum id?)
        bool applied;         ///< whether the effect was applied or removed
    };

    /// An entity defended
    struct Defended {
        const Entity& entity;
    };

    /// An entity bravely ran away
    struct Fled {
        const Entity& entity; ///< the entity leading the charge
        bool succeeded;       ///< whether they succeeded or not
    };

    /// An entity has fallen gloriously
    struct Died {
        const Entity& entity; ///< the entity of renown
        // TODO gained xp, money, phat l00t, etc.
    };

    // etc. for other things (e.g. status effects, weather)

    /// For other messages
    struct Notification {
        std::string message;
    };

}

using Message = std::variant< message::SkillUsed
                            , message::Miss
                            , message::Critical
                            , message::PoolChanged
                            , message::StatusEffect
                            , message::Defended
                            , message::Fled
                            , message::Died
                            , message::Notification
                            >;

///
class MessageLogger {
public:
    /// Adds a new message to the log
    template <typename M>
    void appendMessage(M&& m) noexcept {
        if constexpr (std::is_same_v<std::remove_cv_t<M>, message::SkillUsed>)
            skill_used.emplace(m); // copy, not move
        messages.emplace_back(std::forward<M>(m));
    }

    /// Get a constant iterator to the start of the messages
    [[nodiscard]] decltype(auto) begin() const noexcept {
        return messages.cbegin();
    }

    /// Get a constant iterator to the end of the messages
    [[nodiscard]] decltype(auto) end() const noexcept {
        return messages.cend();
    }

    /// Get the last skill in the message log, if any
    ///
    /// This should always be the skill with its affects being calculated;
    /// that is, calculate all skill effects before performing any other skills
    [[nodiscard]] std::optional<message::SkillUsed> skill() const noexcept {
        return skill_used;
    }

private:
    std::optional<message::SkillUsed> skill_used;
    std::vector<Message> messages;
};


}

#endif // BATTLE_MESSAGES_H_INCLUDED
