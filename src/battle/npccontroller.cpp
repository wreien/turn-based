#include "battle/npccontroller.h"

#include "battle/battleview.h"
#include "battle/entity.h"
#include "battle/skilldetails.h"
#include "util/random.h"

namespace battle {

NPCController::NPCController(Entity& entity)
    : entity{ entity }
{
}

Action NPCController::go(const BattleView& view) {
    // TODO: improve the AI over just doing entirely random things
    const auto skills = entity.getSkills();
    if (skills.empty() || util::random(1.0) < 0.2)
        return action::Defend{};

    const auto& choice = util::random(skills);
    const auto& details = choice->getDetails();

    switch (details.getSpread()) {
    case SkillSpread::Self:
    case SkillSpread::Field:
        return action::Skill{ choice, entity };

    case SkillSpread::Single:
    case SkillSpread::SemiAoE:
    case SkillSpread::AoE:
        return action::Skill{ choice, *util::random(view.enemies) };
    }

    // shouldn't ever get here; just shut up GCC
    return action::Defend {};
}

}
