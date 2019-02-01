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
#include "messages.h"

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

    /// Destructor; needed for unique_ptr with abstract type
    ~Entity();

    /// Disable copy construction
    Entity(const Entity&) = delete;
    /// Disable copy assignment
    Entity& operator=(const Entity&) = delete;

    struct nest_controller_tag {};

    /// Set the (new) contoller for the entity
    /// Pass Entity::nest_controller_tag{} as the first argument if you
    /// need the first constructor arg to be the old controller
    template <typename ControllerType, typename... Args>
    void assignController(Args&&... args) {
        controller = std::make_unique<ControllerType>(
                *this, std::forward<Args>(args)...);
    }

    template <typename ControllerType, typename... Args>
    void assignController(nest_controller_tag, Args&&... args) {
        controller = std::make_unique<ControllerType>(
                *this, std::move(controller), std::forward<Args>(args)...);
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

    /// Applies a status modification to the entity's base stats
    /// TODO: provide some diff about how stats changed?
    /// TODO: generalise for status effects
    void applyStatModifier(StatModifier s);

    [[nodiscard]] bool isDead() const noexcept {
        return hp <= 0;
    }

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
