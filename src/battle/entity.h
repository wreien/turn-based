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
#include "statuseffect.h"
#include "messages.h"

namespace battle {


class Controller;

/// Identifies an entity type
///
/// `kind` and `type` are enough to determine the stats for an entity uniquely,
/// and as such the equality comparison for this class does not take `name`
/// into account.
struct EntityID {
    std::string kind; ///< the 'class' of entity; top level specifier
    std::string type; ///< the 'species' of entity; bottom level specifier
    std::string name; ///< a unique descriptor for the particular entity

    friend bool operator==(const EntityID& lhs, const EntityID& rhs) noexcept {
        return lhs.kind == rhs.kind && lhs.type == rhs.type;
    }
    friend bool operator!=(const EntityID& lhs, const EntityID& rhs) noexcept {
        return !(lhs == rhs);
    }
};

/// An entity in the battle system, player or NPC
class Entity {
public:
    /// Construct an entity, with all the requisite info
    /// (note that Skill is a move-only type, so we propagate that here)
    Entity(EntityID id, int level, Stats stats, std::vector<Skill>&& skills);

    /// Destructor; needed for unique_ptr with abstract type
    ~Entity();

    /// Disable copy construction
    Entity(const Entity&) = delete;
    /// Disable copy assignment
    Entity& operator=(const Entity&) = delete;

    /// Create the (new) contoller for the entity
    /// If ControllerType's `nest_controller` member is true, then passes
    /// the old controller to its constructor.
    template <typename ControllerType, typename... Args>
    void assignController(Args&&... args) {
        if constexpr (ControllerType::nest_controller) {
            controller = std::make_unique<ControllerType>(
                *this, std::move(controller), std::forward<Args>(args)...);
        } else {
            controller = std::make_unique<ControllerType>(
                *this, std::forward<Args>(args)...);
        }
    }

    /// Restore an existing controller as the main one
    /// TODO: redesign controllers so this function isn't public
    void restoreController(std::unique_ptr<Controller>&& ctrl) noexcept;

    /// Retrieve the entity's controller
    [[nodiscard]] Controller& getController() const noexcept {
        return *controller;
    }

    /// Get the identifier for the entity
    const EntityID& getID() const noexcept {
        return id;
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

    /// Get the remaining amount of the specified pool
    template <Pool pool>
    [[nodiscard]] auto get() const noexcept {
        return getPoolRef<pool>();
    }

    /// Get the current maximum value of the specified pool
    template <Pool pool>
    [[nodiscard]] auto getMax() const noexcept {
        if constexpr (pool == Pool::HP)
            return getStats().max_hp;
        else if constexpr (pool == Pool::MP)
            return getStats().max_mp;
        else if constexpr (pool == Pool::Tech)
            return getStats().max_tech;
    }

    /// Drain one of the entity's pools
    /// Note that negative amounts are clamped; you cannot 'accidentally' heal
    /// TODO: is this actually what we want?
    template <Pool pool>
    void drain(MessageLogger& logger, int amt) noexcept {
        auto& p = getPoolRef<pool>();
        auto old = p;
        p -= std::max(amt, 0);
        if (p < 0) p = 0;
        logger.appendMessage(message::PoolChanged{ *this, pool, old, p });
        if constexpr (pool == Pool::HP)
            if (p == 0) logger.appendMessage(message::Died{ *this });
    }

    /// Restore one of the entity's pools
    /// Note that negative amounts are clamped; you cannot 'accidentally' heal
    /// TODO: is this actually what we want?
    template <Pool pool>
    void restore(MessageLogger& logger, int amt) noexcept {
        auto& p = getPoolRef<pool>();
        auto old = p;
        const auto s = getMax<pool>();
        p += std::max(amt, 0);
        if (p > s) p = s;
        logger.appendMessage(message::PoolChanged{ *this, pool, old, p });
    }

    /// Retrieve the entity's skills after any modifiers have been applied
    [[nodiscard]] std::vector<SkillRef> getSkills() const;

    /// Applies a status effect as part of the base stats
    /// TODO: provide some diff about how stats changed?
    void applyStatusEffect(MessageLogger& logger, StatusEffect s);

    /// Get the status effects currently afflicting the entity
    [[nodiscard]] const auto& getAppliedStatusEffects() const noexcept {
        return effects;
    }

    [[nodiscard]] bool isDead() const noexcept {
        return hp <= 0;
    }

    /// Handle any processes that happen after the entity's turn.
    /// For example: buffs wearing off, poison damage, regen effects, etc.
    void processTurnEnd(MessageLogger& logger) noexcept;

private:
    template <Pool pool>
    constexpr auto& getPoolRef() noexcept {
        if constexpr (pool == Pool::HP)
            return hp;
        else if constexpr (pool == Pool::MP)
            return mp;
        else if constexpr (pool == Pool::Tech)
            return tech;
    }

    template <Pool pool>
    constexpr const auto& getPoolRef() const noexcept {
        if constexpr (pool == Pool::HP)
            return hp;
        else if constexpr (pool == Pool::MP)
            return mp;
        else if constexpr (pool == Pool::Tech)
            return tech;
    }

    /// What kind of entity this is
    EntityID id;

    // housekeeping
    int level;       ///< current level
    int exp_to_next; ///< experience needed to advance to next level

    /// The stats for the entity
    Stats stats;

    // current stats
    int hp;    ///< remaining health
    int mp;    ///< remaining magic
    int tech;  ///< remaining tech

    /// Status effects
    std::vector<StatusEffect> effects;

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
