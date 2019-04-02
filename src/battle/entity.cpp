#include "entity.h"
#include "controller.h"
#include "messages.h"
#include "config.h"
#include <iterator>
#include <algorithm>


namespace battle {


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
    , stats{ }
    , hp{ }
    , mp{ }
    , tech{ }
    , effects{ }
    , skills{ }
    , controller{ std::make_unique<NullController>() }
{
    std::tie(stats, skills) = getEntityDetails(kind, type, level);
    hp = stats.max_hp;
    mp = stats.max_mp;
    tech = stats.max_tech;
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
