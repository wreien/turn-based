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

enum class StatusEffectId {
    AttackBoost,
    DefenseBreak,
};

/// Representation of a status effect applied to an entity
///
/// An effect has a type (StatusEffectId) and a duration (EffectDuration);
/// the type determines the duration. Status effects are both effects that
/// manipulate an entity's pools (regen, poison, etc.) and effects that
/// manipulates an entity's stats.
class StatusEffect {
public:
    /// Get the status effect for the given identifier.
    /// TODO: add tiers of status effects? allow strength/duration boosts, etc?
    StatusEffect(StatusEffectId id);

    /// Get the display name for the status effect.
    [[nodiscard]] std::string getName() const noexcept;

    /// Get the status modifiers applied by the effect.
    [[nodiscard]] const std::vector<StatModifier>& getMods() const noexcept {
        return mods;
    }

    /// Get the duration category for the status effect.
    [[nodiscard]] EffectDuration getEffectDuration() const noexcept;

    /// Get the number of turns left for the status effect, if applicable.
    /// (See getEffectDuration)
    [[nodiscard]] std::optional<int> getRemainingTurns() const noexcept;

    /// Call at the end of a turn.
    void endTurn() noexcept;

    /// Returns true if the status effect is still being applied
    [[nodiscard]] bool isActive() const noexcept;

private:
    StatusEffectId id;              ///< the type of effect
    int num_turns_remaining;        ///< the number of turns remaining
    std::vector<StatModifier> mods; ///< the stat modifiers in the effect
};


}

#endif // BATTLE_STATUSEFFECT_H_INCLUDED
