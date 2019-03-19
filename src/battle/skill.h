#ifndef BATTLE_SKILL_H_INCLUDED
#define BATTLE_SKILL_H_INCLUDED

#include <string>
#include <vector>
#include <optional>
#include <memory>
#include "element.h"

namespace battle {


class Entity;
class MessageLogger;


/// Encapsulates a skill
/// TODO: single responsibility principle; this class can be split up
class Skill {
public:
    /// Determines the spread of an attack
    /// TODO: field attacks (probably not actually a `skill' in the same way)
    enum class Spread {
        Self,    ///< targets source
        Single,  ///< targets one entity
        SemiAoE, ///< targets one entity, with spread to rest of their team
        AoE,     ///< targets a whole team
    };

    /// Determines the way the skill deals effects
    enum class Method {
        Physical, ///< uses source's physical stats
        Magical,  ///< uses source's magical stats
        Neither,  ///< uses neither source's magical nor physical stats
    };

    // TODO: UI category?

public:
    /// Create the a specified skill
    explicit Skill(const std::string& name);

    // Explicit move constructors for MSVC
    // see https://stackoverflow.com/questions/54421110/
    Skill(Skill&&) = default;
    Skill& operator=(Skill&&) = default;

    /// Get the display name of the skill
    [[nodiscard]] const std::string& getName() const noexcept {
        return name;
    }

    /// Get the high-level description of what the skill does
    [[nodiscard]] const std::string& getDescription() const noexcept {
        return description;
    }

    /// Determines if the skill can currently be used by the provided entity
    [[nodiscard]] bool isUsableBy(const Entity& source) const noexcept;

    /// Process the effects of `source' using this skill on `target' with team `team'
    /// Logs to `logger'
    void use(MessageLogger& logger, Entity& source, Entity& target,
             const std::vector<Entity*>& team) const noexcept;

public:
    // costs
    // TODO: split into component class just with costs and things?

    [[nodiscard]] std::optional<int> getHPCost() const noexcept {
        return hp_cost;
    }

    [[nodiscard]] std::optional<int> getMPCost() const noexcept {
        return mp_cost;
    }

    [[nodiscard]] std::optional<int> getTechCost() const noexcept {
        return tech_cost;
    }

    // TODO item costs

public:
    // attributes
    // TODO: split into component class just with attributes and things?

    /// If applicable, get the power of the skill
    [[nodiscard]] std::optional<int> getPower() const noexcept {
        return power;
    }

    /// If applicable, get the percentage chance of scoring a hit with the skill
    [[nodiscard]] std::optional<int> getAccuracy() const noexcept {
        return accuracy;
    }

    /// Get the attack spread (AOE-ness) of the skill
    [[nodiscard]] Spread getSpread() const noexcept {
        return spread;
    }

    /// Get the attack method (source stats used) of the skill
    [[nodiscard]] Method getMethod() const noexcept {
        return method;
    }

    /// Get the primary element associated with the skill.
    [[nodiscard]] Element getElement() const noexcept {
        return element;
    }

private:
    // pimpl idiom (with custom deleter)
    struct Data;
    struct DataDeleter { void operator()(Data*) const; };
    std::unique_ptr<Data, DataDeleter> data;

    void processCost(MessageLogger& logger, Entity& source) const noexcept;

    // Cache all our info so we don't need to keep polling through Lua for it.
    // Much of this could well be part of the UI loop, too, so it's beneficial
    // to keep it fast.
    //
    // It's also much simpler, though we do need to remember to keep it up to date.
    // Given we only have one non-const function at the moment that's not too hard,
    // and most of the work will be done in `refresh()`

    /// Refresh the cached data for the skill.
    void refresh();

    std::string name;
    std::string description;
    int level;
    int max_level;

    std::optional<int> power;
    std::optional<int> accuracy;
    Spread spread = Spread::Single;
    Method method = Method::Neither;
    Element element = Element::Neutral;

    std::optional<int> hp_cost;
    std::optional<int> mp_cost;
    std::optional<int> tech_cost;
    // std::vector<Item> item_cost;    // TODO

    std::vector<std::string> perks_applied;
};


}


#endif // BATTLE_SKILL_H_INCLUDED
