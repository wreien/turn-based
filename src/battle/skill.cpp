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

        operator Entity&() noexcept { return entity; }

        Entity& entity;
        MessageLogger& logger;
    };

    // TODO: noexcept part of type?

    template <typename Ret, typename... Args>
    auto wrap_entity_fn(Ret (Entity::* ptr)(MessageLogger&, Args...)) noexcept {
        return [ptr](EntityLogger& self, Args&&... args) {
            return (self.entity.*ptr)(self.logger, std::forward<Args>(args)...);
        };
    }

    template <typename Ret, typename... Args>
    auto wrap_entity_fn(Ret (Entity::* ptr)(MessageLogger&, Args...) const) noexcept {
        return [ptr](EntityLogger& self, Args&&... args) {
            return (self.entity.*ptr)(self.logger, std::forward<Args>(args)...);
        };
    }

    template <typename Ret, typename... Args>
    auto wrap_entity_fn(Ret (Entity::* ptr)(Args...)) noexcept {
        return [ptr](EntityLogger& self, Args&&... args) {
            return (self.entity.*ptr)(std::forward<Args>(args)...);
        };
    }

    template <typename Ret, typename... Args>
    auto wrap_entity_fn(Ret (Entity::* ptr)(Args...) const) noexcept {
        return [ptr](EntityLogger& self, Args&&... args) {
            return (self.entity.*ptr)(std::forward<Args>(args)...);
        };
    }

    sol::state_view luaState() {
        static sol::state lua = [](){
            using namespace battle;

            sol::state lua;
            lua.open_libraries(
                    sol::lib::base,    // required
                    sol::lib::table,   // inserting into numbered lists
                    sol::lib::math,    // random no's and other math fns
                    sol::lib::package  // require (TODO: do we want this?)
            );

            // prevent accidentally loading weird libraries and make sure we
            // actually get the libraries we *do* want
            lua.script("package.path = './data/?.lua'");

            lua.new_enum<Skill::Method>("method", {
                { "physical", Skill::Method::Physical },
                { "magical",  Skill::Method::Magical },
                { "mixed",    Skill::Method::Mixed },
                { "none",     Skill::Method::None },
            });

            lua.new_enum<Skill::Spread>("spread", {
                { "self",    Skill::Spread::Self },
                { "single",  Skill::Spread::Single },
                { "semiaoe", Skill::Spread::SemiAoE },
                { "aoe",     Skill::Spread::AoE },
                { "field",   Skill::Spread::Field },
            });

            lua.new_enum<Element>("element", {
                { "neutral",   Element::Neutral },
                { "fire",      Element::Fire },
                { "water",     Element::Water },
                { "earth",     Element::Earth },
                { "air",       Element::Air },
                { "light",     Element::Light },
                { "dark",      Element::Dark },
                { "ice",       Element::Ice },
                { "lightning", Element::Lightning },
                { "sand",      Element::Sand },
                { "steam",     Element::Steam },
                { "life",      Element::Life },
                { "metal",     Element::Metal },
            });

            {
                auto metatable = lua.new_usertype<EntityLogger>("Entity",
                    "new", sol::no_constructor);

                metatable["stats"] = sol::readonly_property(
                        wrap_entity_fn(&Entity::getStats));

                metatable["getKind"] = wrap_entity_fn(&Entity::getKind);
                metatable["getType"] = wrap_entity_fn(&Entity::getType);
                metatable["getName"] = wrap_entity_fn(&Entity::getName);

                metatable["drainHP"]   = wrap_entity_fn(&Entity::drain<Pool::HP>);
                metatable["drainMP"]   = wrap_entity_fn(&Entity::drain<Pool::MP>);
                metatable["drainTech"] = wrap_entity_fn(&Entity::drain<Pool::Tech>);

                metatable["restoreHP"]   = wrap_entity_fn(&Entity::restore<Pool::HP>);
                metatable["restoreMP"]   = wrap_entity_fn(&Entity::restore<Pool::MP>);
                metatable["restoreTech"] = wrap_entity_fn(&Entity::restore<Pool::Tech>);

                metatable["getHP"]   = wrap_entity_fn(&Entity::get<Pool::HP>);
                metatable["getMP"]   = wrap_entity_fn(&Entity::get<Pool::MP>);
                metatable["getTech"] = wrap_entity_fn(&Entity::get<Pool::Tech>);
            }

            {
                auto metatable = lua.new_usertype<Stats>("Stats");

                metatable["max_hp"]   = &Stats::max_hp,
                metatable["max_mp"]   = &Stats::max_mp,
                metatable["max_tech"] = &Stats::max_tech,

                metatable["p_atk"] = &Stats::p_atk,
                metatable["p_def"] = &Stats::p_def,
                metatable["m_atk"] = &Stats::m_atk,
                metatable["m_def"] = &Stats::m_def,
                metatable["skill"] = &Stats::skill,
                metatable["evade"] = &Stats::evade,
                metatable["speed"] = &Stats::speed,

                // not *quite* a real property
                metatable["resist"] = sol::overload(
                    &Stats::getResistance, &Stats::setResistance);
            }

            // now that we've set up all the usertypes, we're safe to load the
            // current list of possible skills
            lua.script_file("./data/skill/main.lua",
                [](lua_State*, sol::protected_function_result pfr) -> decltype(pfr) {
                    sol::error err = pfr;
                    throw std::runtime_error(
                        std::string{"error loading skills: "} + err.what());
                }
            );

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

void Skill::DataDeleter::operator()(Data* d) const noexcept {
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
                const std::vector<Entity*>& team) const
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
        // TODO: dedicated error type for skill failures/lua failures
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
    const auto opt = [](sol::table t, const char* c) -> std::optional<int> {
        using namespace std::literals;
        auto val = t[c];
        switch (val.get_type()) {
        case sol::type::number:
            return val;
        case sol::type::none:
            return std::nullopt;
        default:
            throw std::invalid_argument("'"s + c + "' was not a number"s);
        }
    };

    const auto t = data->skill_data;

    name = t["name"];
    description = t["desc"];
    level = t["level"];
    max_level = t["max_level"];

    power = opt(t, "power");
    accuracy = opt(t, "accuracy");
    method = t["method"].get_or(Method::None);
    spread = t["spread"].get_or(Spread::Single);
    element = t["element"].get_or(Element::Neutral);

    hp_cost = opt(t, "hp_cost");
    mp_cost = opt(t, "mp_cost");
    tech_cost = opt(t, "tech_cost");
    // items

    // applied perks
}

}
