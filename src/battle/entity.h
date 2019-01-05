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

    /// Retrieve the entity's stats after any modifiers have been applied
    /// \TODO Return a proxy instead, for efficiency?
    [[nodiscard]] Stats getStats() const noexcept;

    /// Retrieve the entity's skills after any modifiers have been applied
    [[nodiscard]] std::vector<SkillRef> getSkills() const;

private:
    /// What kind of entity this is
    std::string kind;

    /// The stats for the entity
    Stats stats;
    // calculated stats here? They need to be modifiable somehow -- or do they?
    // maybe steal some ideas from skill modifications - think more.

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
