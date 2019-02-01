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

    /// An enemy defended
    struct Defended {
        const Entity& entity;
    };

    /// An enemy bravely ran away
    struct Fled {
        const Entity& entity; ///< the entity leading the charge
        bool succeeded;       ///< whether they succeeded or not
    };

    // etc. for other things (e.g. status effects, weather)

    /// For other messages
    struct Notification {
        std::string message;
    };

}

using Message = std::variant< message::SkillUsed
                            , message::PoolChanged
                            , message::Defended
                            , message::Fled
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
