#include "entity.h"
#include "controller.h"
#include "messages.h"
#include <iterator>
#include <algorithm>

namespace battle {


// TODO: entity factory functions
namespace {
    Stats getEntityStats(const std::string& kind, const std::string& type, int level) {
        // The raddest scaling you'll ever see
        if (kind == "default" && type == "good") {
            return {
                35 * level, // hp
                2 * level,  // mp
                10,         // tech
                3 * level,  // p_atk
                2 * level,  // p_def
                2 * level,  // m_atk
                3 * level,  // m_def
                5,          // skill
                5,          // evade
                5 + level,  // speed
                { 0 }       // resist
            };
        } else if (kind == "default" && type == "evil") {
            return {
                27 * level, // hp
                3 * level,  // mp
                10,         // tech
                2 * level,  // p_atk
                1 * level,  // p_def
                3 * level,  // m_atk
                2 * level,  // m_def
                5,          // skill
                5,          // evade
                6 + level,  // speed
                { 0 }       // resist
            };
        } else {
            // who even cares
            return {};
        }
    }

    std::vector<Skill> getEntitySkills(
            const std::string& kind,
            const std::string& type
            ,int level)
    {
        (void)kind;
        (void)type;
        (void)level;
        std::vector<Skill> skills;
        skills.emplace_back("attack");
        // skills.emplace_back("heal");
        // skills.emplace_back("attack boost");
        // skills.emplace_back("defense break");
        return skills;
    }
}


// A no-effort controller that simply defends every turn
struct NullController : public Controller {
    virtual Action go(const BattleView&) {
        return action::Defend{};
    }
};


Entity::Entity(std::string kind_, std::string type_, std::string name_, int level)
    : kind{ std::move(kind_) }
    , type{ std::move(type_) }
    , name{ std::move(name_) }
    , level{ level }
    , exp_to_next{ 0 } // TODO
    , stats{ getEntityStats(kind, type, level) }
    , hp{ stats.max_hp }
    , mp{ stats.max_mp }
    , tech{ stats.max_tech }
    , effects{ }
    , skills{ getEntitySkills(kind, type, level) }
    , controller{ std::make_unique<NullController>() }
{
}

Entity::~Entity() = default;

std::vector<SkillRef> Entity::getSkills() const {
    // TODO: apply equipment bonuses, status effects, etc.
    return { std::begin(skills), std::end(skills) };
}

Stats Entity::getStats() const noexcept {
    // TODO: apply equipment bonuses, etc.
    // TODO: cache results?
    std::vector<StatModifier> mods;
    for (auto&& e : effects) {
        const auto& effect_mods = e.getMods();
        std::copy(std::begin(effect_mods), std::end(effect_mods),
                  std::back_inserter(mods));
    }

    return calculateModifiedStats(stats, mods);
}

// TODO: cap/mod hp/mp/tech as appropriate
void Entity::applyStatusEffect(MessageLogger& logger, StatusEffect s) {
    logger.appendMessage(message::StatusEffect{ *this, s.getName(), true });
    effects.emplace_back(std::move(s));
}

// TODO: cap/mod hp/mp/tech as appropriate
void Entity::processTurnEnd(MessageLogger& logger) noexcept {
    // move effects being removed to the end
    auto it = std::partition(std::begin(effects), std::end(effects), [](auto&& e) {
        // TODO parse logger and don't call end turn on the effect if the effect
        // was just applied (should make more natural durations for effects)
        e.endTurn();
        return e.isActive();
    });
    // process effects being removed
    std::for_each(it, std::end(effects), [&](auto&& e) {
        logger.appendMessage(message::StatusEffect{
            *this, e.getName(), false
        });
    });
    // remove effects being, uh, removed
    effects.erase(it, std::end(effects));
}


}
