#ifndef BATTLE_SKILLDETAILS_H_INCLUDED
#define BATTLE_SKILLDETAILS_H_INCLUDED

#include <string>
#include <optional>
#include <memory>
#include <vector>
#include "element.h"

namespace battle {


class MessageLogger;
class Entity;
struct BattleView;


/// Determines the spread of an attack
enum class SkillSpread {
    Self,    ///< targets source
    Single,  ///< targets one entity
    SemiAoE, ///< targets one entity, with spread to rest of their team
    AoE,     ///< targets a whole team
    Field,   ///< targets the entire battleground
};

/// Determines the way the skill calculates the effect of power
enum class SkillMethod {
    Physical, ///< uses physical stats
    Magical,  ///< uses magical stats
    Mixed,    ///< uses weird stats/some combination of the above
    None,     ///< doesn't use power
};

// TODO: UI category?

/// Immutable structure encapsulating the unchanging parts of a skill
/// This includes: attributes, costs, and the `perform' function.
/// Note: implementation currently in battle/config.cpp
class SkillDetails {
public:
    SkillDetails(const std::string& name, int level);

    // TODO: make copyable? (possible, but do we want to is the question)

    [[nodiscard]] const std::string& getName() const noexcept { return name; }
    [[nodiscard]] const std::string& getDescription() const noexcept { return desc; }
    [[nodiscard]] int getLevel() const noexcept { return level; }
    [[nodiscard]] int getMaxLevel() const noexcept { return max_level; }
    [[nodiscard]] std::optional<int> getHPCost() const noexcept { return hp_cost; }
    [[nodiscard]] std::optional<int> getMPCost() const noexcept { return mp_cost; }
    [[nodiscard]] std::optional<int> getTechCost() const noexcept { return tech_cost; }
    [[nodiscard]] std::optional<int> getPower() const noexcept { return power; }
    [[nodiscard]] std::optional<int> getAccuracy() const noexcept { return accuracy; }
    [[nodiscard]] SkillSpread getSpread() const noexcept { return spread; }
    [[nodiscard]] SkillMethod getMethod() const noexcept { return method; }
    [[nodiscard]] Element getElement() const noexcept { return element; }

private:
    // perform should only be seeable by Skill
    friend class Skill;

    /// Source uses this skill on target (and their team). Log all events to logger.
    /// NOTE: not noexcept! (TODO: return sensible exceptions)
    void perform(MessageLogger& logger,
                 Entity& source, Entity& target,
                 const BattleView& view) const;

    // details
    std::string name;
    std::string desc;
    int level;
    int max_level;

    // costs
    std::optional<int> hp_cost;
    std::optional<int> mp_cost;
    std::optional<int> tech_cost;
    // TODO item cost

    // attributes
    std::optional<int> power;
    std::optional<int> accuracy;
    SkillSpread spread;
    SkillMethod method;
    Element element;

    // manage the lua handle
    struct LuaHandle;
    struct Deleter { void operator()(LuaHandle*) const noexcept; };
    std::unique_ptr<LuaHandle, Deleter> handle;
};

}

#endif // BATTLE_SKILLDETAILS_H_INCLUDED
