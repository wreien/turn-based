#ifndef BATTLE_MESSAGES_H_INCLUDED
#define BATTLE_MESSAGES_H_INCLUDED

#include <vector>
#include <variant>
#include <string>
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
        messages.emplace_back(std::forward<M>(m));
    }

    /// Inspect the message log
    [[nodiscard]] const std::vector<Message>& inspect() const noexcept {
        return messages;
    }

    /// Destructively retrieves the message log
    [[nodiscard]] std::vector<Message> pop() noexcept {
        return std::exchange(messages, decltype(messages){});
    }

private:
    std::vector<Message> messages;
};


}

#endif // BATTLE_MESSAGES_H_INCLUDED
