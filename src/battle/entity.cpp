#include "entity.h"
#include "controller.h"

namespace battle {


// TODO: entity factory functions
namespace {
    Stats getEntityStats(const std::string& kind, int level) {
        (void)kind;
        (void)level;
        return {};
    }

    Stats getEntityStats(std::filesystem::path file) {
        (void)file;
        return {};
    }

    std::vector<Skill> getEntitySkills(const std::string& kind, int level) {
        (void)kind;
        (void)level;
        return {};
    }

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

Stats Entity::getStats() const noexcept {
    // TODO: apply equipment bonuses, status effects, etc.
    return stats;
}

std::vector<SkillRef> Entity::getSkills() const {
    // TODO: apply equipment bonuses, status effects, etc.
    return { std::begin(skills), std::end(skills) };
}

// TODO: cap/mod hp/mp/tech as appropriate
void Entity::applyStatModifier(StatModifier s) {
    mods.push_back(s);
}

}
