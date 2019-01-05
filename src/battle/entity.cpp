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
    , stats{ getEntityStats(kind, level) }
    , skills{ getEntitySkills(kind, level) }
    , controller{ std::make_unique<NullController>() }
{
}

Entity::Entity(std::filesystem::path file)
    : kind{ "<" + file.filename().native() + ">" }
    , stats{ getEntityStats(file) }
    , skills{ getEntitySkills(file) }
    , controller{ std::make_unique<NullController>() }
{
}

Stats Entity::getStats() const noexcept {
    // TODO: apply equipment bonuses, status effects, etc.
    return stats;
}

std::vector<SkillRef> Entity::getSkills() const {
    // TODO: apply equipment bonuses, status effects, etc.
    return { std::begin(skills), std::end(skills) };
}


}
