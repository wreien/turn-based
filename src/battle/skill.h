#ifndef BATTLE_SKILL_H_INCLUDED
#define BATTLE_SKILL_H_INCLUDED

#include <memory>
#include <string>
#include <vector>
#include "battle/skilldetails.h"

namespace battle {


class Entity;
class MessageLogger;
class BattleSystem;


/// Encapsulates a skill
class Skill {
public:
    /// Create the a specified skill
    explicit Skill(const std::string& name, int level = 1);

    /// Determines if the skill can currently be used by the provided entity
    [[nodiscard]] bool isUsableBy(const Entity& source) const noexcept;

    /// Process the effects of `source' using this skill on `target';
    /// logs to logger, and includes the view of the battle
    void use(MessageLogger& logger,
             Entity& source, Entity& target,
             BattleSystem& system) const;

    const SkillDetails& getDetails() const noexcept {
        return details;
    }

private:
    void processCost(MessageLogger& logger, Entity& source) const noexcept;

    SkillDetails details;
    std::vector<std::string> perks_applied;
};


}


#endif // BATTLE_SKILL_H_INCLUDED
