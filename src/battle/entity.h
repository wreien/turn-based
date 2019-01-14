#ifndef BATTLE_ENTITY_H_INCLUDED
#define BATTLE_ENTITY_H_INCLUDED

#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include <optional>

#include "skill.h"
#include "skillref.h"
#include "stats.h"

namespace battle {


class Controller;

/// An entity in the battle system, player or NPC
class Entity {
public:
    /// Load a particular entity from a file
    /// TODO: is this even necessary? or desirable?
    explicit Entity(std::filesystem::path file);

    /// Create an entity of the given kind at the provided level
    Entity(const std::string& kind, int level);

    /// Set the (new) contoller for the entity
    template <typename ControllerType, typename... Args>
    void assignController(Args&&... args) {
        controller = std::make_unique<ControllerType>(
                *this, std::forward<Args>(args)...);
    }

    /// Retrieve the entity's controller
    [[nodiscard]] Controller& getController() const noexcept {
        return *controller;
    }

    /// Get the kind of the entity
    [[nodiscard]] std::string getKind() const noexcept {
        return kind;
    }

    /// Get the current level of the entity
    [[nodiscard]] constexpr auto getLevel() const noexcept {
        return level;
    }

    /// Get the amount of experience required to level up.
    [[nodiscard]] constexpr auto getExperience() const noexcept {
        return exp_to_next;
    }

    /// Retrieve the entity's stats after any modifiers have been applied
    /// \TODO Return a proxy instead, for efficiency? (premature optimization much)
    [[nodiscard]] Stats getStats() const noexcept;

    /// Get remaining health.
    [[nodiscard]] constexpr auto getHP() const noexcept {
        return hp;
    }

    /// Get remaining mana.
    [[nodiscard]] constexpr auto getMP() const noexcept {
        return mp;
    }

    /// Get remaining tech points.
    [[nodiscard]] constexpr auto getTech() const noexcept {
        return tech;
    }

    /// Retrieve the entity's skills after any modifiers have been applied
    [[nodiscard]] std::vector<SkillRef> getSkills() const;

    /// Applies a status modification to the entity's base stats
    /// TODO: provide some diff about how stats changed?
    void applyStatModifier(StatModifier s);

private:
    /// What kind of entity this is
    std::string kind;

    // housekeeping
    int level;       ///< current level
    int exp_to_next; ///< experience needed to advance to next level

    /// The stats for the entity
    Stats stats;

    // current stats
    int hp;    ///< remaining health
    int mp;    ///< remaining magic
    int tech;  ///< remaining tech

    /// Status modifiers
    /// TODO time limits for mods?
    /// TODO split into permanant/temporary?
    std::vector<StatModifier> mods;

    /// The skill the entity itself owns
    std::vector<Skill> skills;

    /// The current controller for the entity.
    /// Never `nullptr`.
    std::unique_ptr<Controller> controller;

    // TODO: equipment+items
    // TODO: visualiser
};


}


#endif // BATTLE_ENTITY_H_INCLUDED
