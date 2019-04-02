#ifndef BATTLE_SKILL_H_INCLUDED
#define BATTLE_SKILL_H_INCLUDED

#include <string>
#include <vector>
#include <memory>
#include "skilldetails.h"

namespace battle {


class Entity;
class MessageLogger;


/// Encapsulates a skill
/// TODO: single responsibility principle; this class can be split up
class Skill {
public:
    /// Create the a specified skill
    explicit Skill(const std::string& name, int level = 1);

    // Explicit move constructors for MSVC
    // see https://stackoverflow.com/questions/54421110/
    Skill(Skill&&) = default;
    Skill& operator=(Skill&&) = default;

    /// Determines if the skill can currently be used by the provided entity
    [[nodiscard]] bool isUsableBy(const Entity& source) const noexcept;

    /// Process the effects of `source' using this skill on `target' with team `team'
    /// Logs to `logger'
    void use(MessageLogger& logger, Entity& source, Entity& target,
             const std::vector<Entity*>& team) const;

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
