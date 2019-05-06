#ifndef BATTLE_PLAYERCONTROLLER_H_INCLUDED
#define BATTLE_PLAYERCONTROLLER_H_INCLUDED

#include <vector>
#include <optional>
#include "controller.h"
#include "skillref.h"

namespace battle {

class Entity;

/// Options that a user has to choose from for their action
struct UserOptions {
    bool defend;                  ///< whether the entity can defend
    bool flee;                    ///< whether the entity can flee

    std::vector<SkillRef> skills; ///< the skills an entity can select from

    // TODO: items; split up skills here?
};

/// The controller for interfacing human interaction with an entity
class PlayerController : public Controller {
public:
    static constexpr bool nest_controller = false;

    explicit PlayerController(Entity& entity);
    [[nodiscard]] virtual Action go(const BattleView& view) override;

    // TODO: pass any extra information needed to work out what's possible here
    [[nodiscard]] UserOptions options() const;
    void choose(const Action& act);

    [[nodiscard]] constexpr const Entity& getEntity() const noexcept {
        return entity;
    }

private:
    Entity& entity;
    std::optional<Action> choice;
};

}

#endif // BATTLE_PLAYERCONTROLLER_H_INCLUDED
