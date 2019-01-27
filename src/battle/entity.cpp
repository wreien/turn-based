#include "entity.h"
#include "controller.h"

namespace battle {


// TODO: entity factory functions
namespace {
    Stats getEntityStats(const std::string& kind, int level) {
        // The raddest scaling you'll ever see
        if (kind.find("good") != std::string::npos) {
            return {
                5 * level,  // hp
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
                3 * level,  // hp
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
    , mods{ }
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
    , mods{ }
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
    return calculateModifiedStats(stats, mods);
}

// TODO: cap/mod hp/mp/tech as appropriate
void Entity::applyStatModifier(StatModifier s) {
    mods.push_back(s);
}

}
