#include "entity.h"
#include "controller.h"
#include "messages.h"
#include "config.h"
#include <iterator>
#include <algorithm>


namespace battle {


// we'll put the controller's destructor here, since it doesn't have a CPP file
// of its own, to prevent creation of bonus virtual tables
// TODO: double check this is actually needed: this is a half-formed memory
Controller::~Controller() {}


// A no-effort controller that simply defends every turn
struct NullController : public Controller {
    static constexpr bool nest_controller = false;

    virtual Action go(const BattleView&) {
        return action::Defend{};
    }
};


Entity::Entity(EntityID id, int level, Stats stats, std::vector<Skill>&& skills)
    : id { std::move(id) }
    , level{ level }
    , exp_to_next{ 0 }
    , stats{ stats }
    , hp{ this->stats.max_hp }
    , mp{ this->stats.max_mp }
    , tech{ this->stats.max_tech }
    , effects{ }
    , skills{ std::move(skills) }
    , controller{ std::make_unique<NullController>() }
{
}

Entity::~Entity() = default;

void Entity::restoreController(std::unique_ptr<Controller>&& ctrl) noexcept {
    controller = std::move(ctrl);
}

std::vector<SkillRef> Entity::getSkills() const {
    // TODO: apply equipment bonuses, status effects, etc.
    std::vector<SkillRef> refs;
    std::copy_if(std::begin(skills), std::end(skills),
                 std::back_inserter(refs),
                 [this](const Skill& s) { return s.isUsableBy(*this); });
    return refs;
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
