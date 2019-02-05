#ifndef BATTLE_STATUSEFFECT_H_INCLUDED
#define BATTLE_STATUSEFFECT_H_INCLUDED

#include "stats.h"
#include <vector>
#include <optional>
#include <string>
#include <string_view>

namespace battle {


/// The length of a status effect
enum class EffectDuration {
    Permanent,  ///< lasts for ever, for example food effects; cannot be cleansed
    Battle,     ///< lasts for the remaining duration of the battle
    Temporary,  ///< lasts for a certain number of turns
};

class StatusEffect {
public:
    /// TODO: status effect levels/tiers? use enum rather than string to identify?
    StatusEffect(std::string name);

    [[nodiscard]] std::string_view getName() const noexcept {
        return name;
    }

    [[nodiscard]] const std::vector<StatModifier>& getMods() const noexcept {
        return mods;
    }

    [[nodiscard]] EffectDuration getEffectDuration() const noexcept;
    [[nodiscard]] std::optional<int> getRemainingTurns() const noexcept;

    /// Call at the end of a turn.
    /// Returns true if the status effect is over and should be removed.
    [[nodiscard]] bool endTurn() noexcept;

    // TODO: "classes" of status effects?

private:
    std::string name;                   ///< the name of the effect
    int num_turns_remaining;            ///< the number of turns remaining
    std::vector<StatModifier> mods;     ///< the stat modifiers in the effect
};


}

#endif // BATTLE_STATUSEFFECT_H_INCLUDED
