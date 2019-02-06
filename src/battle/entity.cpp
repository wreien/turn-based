#include "entity.h"
#include "controller.h"
#include "messages.h"
#include <iterator>

namespace battle {


// TODO: entity factory functions
namespace {
    Stats getEntityStats(const std::string& kind, int level) {
        // The raddest scaling you'll ever see
        if (kind.find("good") != std::string::npos) {
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
        } else if (kind.find("evil") != std::string::npos) {
            return {
                27 * level, // hp
                3 * level,  // mp
                10,         // tech
                2 * level,  // p_atk
                2 * level,  // p_def
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

    // TODO
    Stats getEntityStats(std::filesystem::path file) {
        (void)file;
        return {};
    }

    std::vector<Skill> getEntitySkills(const std::string& kind, int level) {
        (void)kind;
        (void)level;
        std::vector<Skill> skills;
        skills.emplace_back("attack");
        skills.emplace_back("heal");
        skills.emplace_back("attack boost");
        skills.emplace_back("defense break");
        return skills;
    }

    // TODO
    std::vector<Skill> getEntitySkills(std::filesystem::path file) {
        (void)file;
        return {};
    }
}


// A no-effort controller that simply defends every turn
struct NullController : public Controller {
    virtual Action go(const BattleView&) {
        return action::Defend{};
    }
};


Entity::Entity(const std::string& kind, int level)
    : kind{ kind }
    , level{ level }
    , exp_to_next{ 0 } // TODO
    , stats{ getEntityStats(kind, level) }
    , hp{ stats.max_hp }
    , mp{ stats.max_mp }
    , tech{ stats.max_tech }
    , effects{ }
    , skills{ getEntitySkills(kind, level) }
    , controller{ std::make_unique<NullController>() }
{
}

Entity::Entity(std::filesystem::path file)
    : kind{ "<" + file.filename().string() + ">" }
    , level{ 0 }
    , exp_to_next{ 0 } // TODO
    , stats{ getEntityStats(file) }
    , hp{ stats.max_hp }
    , mp{ stats.max_mp }
    , tech{ stats.max_tech }
    , effects{ }
    , skills{ getEntitySkills(file) }
    , controller{ std::make_unique<NullController>() }
{
    // TODO: update level here
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
void Entity::applyStatusEffect(MessageLogger&, StatusEffect s) {
    effects.push_back(std::move(s));
}

// TODO: cap/mod hp/mp/tech as appropriate
void Entity::processTurnEnd(MessageLogger&) noexcept {
    auto it = std::remove_if(std::begin(effects), std::end(effects),
                             [](auto&& e){ return e.endTurn(); });
    effects.erase(it, std::end(effects));
}


}
