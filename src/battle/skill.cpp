#include "skill.h"
#include "entity.h"
#include "messages.h"

#include <type_traits>
#include <string_view>

#define SOL_CHECK_ARGUMENTS 1
#include <sol/sol.hpp>

using battle::Entity;
using battle::MessageLogger;

// helpers to interface to the skill stuff
namespace {

    struct EntityLogger {
        EntityLogger(Entity& e, MessageLogger& l)
            : entity{ e }, logger{ l }
        {}

        operator Entity&() { return entity; }

        Entity& entity;
        MessageLogger& logger;
    };

    // TODO: noexcept part of type?

    template <typename Ret, typename... Args>
    auto wrap_fn(Ret (Entity::* ptr)(MessageLogger&, Args...)) {
        return [ptr](EntityLogger& self, Args&&... args) {
            return (self.entity.*ptr)(self.logger, std::forward<Args>(args)...);
        };
    }

    template <typename Ret, typename... Args>
    auto wrap_fn(Ret (Entity::* ptr)(MessageLogger&, Args...) const) {
        return [ptr](EntityLogger& self, Args&&... args) {
            return (self.entity.*ptr)(self.logger, std::forward<Args>(args)...);
        };
    }

    template <typename Ret, typename... Args>
    auto wrap_fn(Ret (Entity::* ptr)(Args...)) {
        return [ptr](EntityLogger& self, Args&&... args) {
            return (self.entity.*ptr)(std::forward<Args>(args)...);
        };
    }

    template <typename Ret, typename... Args>
    auto wrap_fn(Ret (Entity::* ptr)(Args...) const) {
        return [ptr](EntityLogger& self, Args&&... args) {
            return (self.entity.*ptr)(std::forward<Args>(args)...);
        };
    }

    sol::state_view luaState() {
        static sol::state lua = [](){
            using namespace battle;
            using namespace std::literals;

            sol::state lua;
            lua.open_libraries(
                    sol::lib::base,    // required
                    sol::lib::table,   // inserting into numbered lists
                    sol::lib::math,    // random no's and other math fns
                    sol::lib::package  // require (TODO: do we want this?)
            );
            lua.script("package.path = './lua/?.lua'");

            lua.new_enum("method", {
                std::make_pair( "physical"sv, Skill::Method::Physical ),
                std::make_pair( "magical"sv, Skill::Method::Magical ),
                std::make_pair( "mixed"sv, Skill::Method::Neither ),
                std::make_pair( "none"sv, Skill::Method::Neither )
            });

            lua.new_usertype<EntityLogger>("Entity",
                "new", sol::no_constructor,

                "stats", sol::readonly_property( wrap_fn(&Entity::getStats) ),

                "drainHP",   wrap_fn(&Entity::drain<Pool::HP>),
                "drainMP",   wrap_fn(&Entity::drain<Pool::MP>),
                "drainTech", wrap_fn(&Entity::drain<Pool::Tech>),

                "restoreHP",   wrap_fn(&Entity::restore<Pool::HP>),
                "restoreMP",   wrap_fn(&Entity::restore<Pool::MP>),
                "restoreTech", wrap_fn(&Entity::restore<Pool::Tech>)
            );
            lua.new_usertype<Stats>("Stats",
                "max_hp",   &Stats::max_hp,
                "max_mp",   &Stats::max_mp,
                "max_tech", &Stats::max_tech,

                "p_atk", &Stats::p_atk,
                "p_def", &Stats::p_def,
                "m_atk", &Stats::m_atk,
                "m_def", &Stats::m_def,
                "skill", &Stats::skill,
                "evade", &Stats::evade,
                "speed", &Stats::speed,

                "resist", sol::overload(
                    &Stats::getResistance,
                    &Stats::setResistance
                )
            );

            lua.script_file("lua/skills/main.lua");

            return lua;
        }();

        return lua;
    }

}

namespace battle {

struct Skill::Data {
    Data(const std::string& name) {
        auto lua = luaState();
        sol::protected_function fn = lua["Skills"]["get"];
        auto ret = fn(lua["Skills"], name);
        if (ret.valid())
            skill_data = ret;
        else {
            sol::error err = ret;
            throw std::invalid_argument(err.what());
        }
    }

    sol::table skill_data;
};

void Skill::DataDeleter::operator()(Data* d) const {
    delete d;
}

Skill::Skill(const std::string& name)
    : data{ new Data{ name } }
{
    refresh();
}

bool Skill::isUsableBy(const Entity& source) const noexcept {
    // note: std::nullopt < x for all x
    if (hp_cost > source.get<Pool::HP>())
        return false;
    if (mp_cost > source.get<Pool::MP>())
        return false;
    if (tech_cost > source.get<Pool::Tech>())
        return false;
    // TODO items
    return true;
}

void Skill::use(MessageLogger& logger,
                Entity& source,
                Entity& target,
                const std::vector<Entity*>& team)
    const noexcept
{
    logger.appendMessage(message::SkillUsed{ *this, source, target });
    processCost(logger, source);

    EntityLogger s{ source, logger };
    EntityLogger t{ target, logger };
    (void) team; // TODO

    sol::protected_function perform = data->skill_data["perform"];
    auto ret = perform(data->skill_data, s, t);
    if (!ret.valid()) {
        sol::error err = ret;
        throw std::runtime_error(err.what());
    }
}

void Skill::processCost(MessageLogger& logger, Entity& source) const noexcept {
    if (hp_cost) source.drain<Pool::HP>(logger, *hp_cost);
    if (mp_cost) source.drain<Pool::MP>(logger, *mp_cost);
    if (tech_cost) source.drain<Pool::Tech>(logger, *tech_cost);
    // TODO items
}

void Skill::refresh() {
    const auto opt = [](sol::table t, const char* c) {
        sol::optional<int> val = t[c];
        if (val)
            return std::optional<int>{ *val };
        else
            return std::optional<int>{ std::nullopt };
    };

    const auto t = data->skill_data;

    name = t["name"];
    description = t["desc"];
    level = t["level"];
    max_level = t["max_level"];

    power = opt(t, "power");
    accuracy = opt(t, "accuracy");
    method = t["method"].get_or(Method::Neither);
    spread = t["spread"].get_or(Spread::Single);
    element = t["element"].get_or(Element::Neutral);

    hp_cost = opt(t, "hp_cost");
    mp_cost = opt(t, "mp_cost");
    tech_cost = opt(t, "tech_cost");
    // items

    // applied perks
}

}
