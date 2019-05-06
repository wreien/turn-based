#include "config.h"
#include "skilldetails.h"
#include "skill.h"
#include "entity.h"
#include "stats.h"
#include "battlesystem.h"

#include <type_traits>

#define SOL_CHECK_ARGUMENTS 1
#include <sol/sol.hpp>

// helpers to set up the lua environment
namespace battle::config {

    struct EntityLogger {
        EntityLogger(Entity* e, BattleSystem* s, MessageLogger* l)
            : entity{ e }, system{ s }, logger{ l }
        {}

        operator Entity&() noexcept { return *entity; }

        Entity* entity;
        BattleSystem* system;
        MessageLogger* logger;
    };

    // TODO: noexcept part of type (but can I really be bothered...)

    template <typename Ret, typename... Args>
    auto wrap_entity_fn(Ret (Entity::* ptr)(MessageLogger&, Args...)) {
        return [ptr](EntityLogger& self, Args&&... args) {
            return (self.entity->*ptr)(*self.logger, std::forward<Args>(args)...);
        };
    }

    template <typename Ret, typename... Args>
    auto wrap_entity_fn(Ret (Entity::* ptr)(MessageLogger&, Args...) const) {
        return [ptr](EntityLogger& self, Args&&... args) {
            return (self.entity->*ptr)(*self.logger, std::forward<Args>(args)...);
        };
    }

    template <typename Ret, typename... Args>
    auto wrap_entity_fn(Ret (Entity::* ptr)(Args...)) {
        return [ptr](EntityLogger& self, Args&&... args) {
            return (self.entity->*ptr)(std::forward<Args>(args)...);
        };
    }

    template <typename Ret, typename... Args>
    auto wrap_entity_fn(Ret (Entity::* ptr)(Args...) const) {
        return [ptr](EntityLogger& self, Args&&... args) {
            return (self.entity->*ptr)(std::forward<Args>(args)...);
        };
    }

    void loadEntityLoggerMetatable(sol::state_view& lua) {
        auto metatable = lua.new_usertype<EntityLogger>("logged_entity",
            "new", sol::no_constructor);

        metatable["stats"] = sol::readonly_property(
                wrap_entity_fn(&Entity::getStats));

        metatable["getKind"] = [](EntityLogger& el){ return el.entity->getID().kind; };
        metatable["getType"] = [](EntityLogger& el){ return el.entity->getID().type; };
        metatable["getName"] = [](EntityLogger& el){ return el.entity->getID().name; };

        metatable["getLevel"]      = wrap_entity_fn(&Entity::getLevel);
        metatable["getExperience"] = wrap_entity_fn(&Entity::getExperience);

        metatable["drainHP"]   = wrap_entity_fn(&Entity::drain<Pool::HP>);
        metatable["drainMP"]   = wrap_entity_fn(&Entity::drain<Pool::MP>);
        metatable["drainTech"] = wrap_entity_fn(&Entity::drain<Pool::Tech>);

        metatable["restoreHP"]   = wrap_entity_fn(&Entity::restore<Pool::HP>);
        metatable["restoreMP"]   = wrap_entity_fn(&Entity::restore<Pool::MP>);
        metatable["restoreTech"] = wrap_entity_fn(&Entity::restore<Pool::Tech>);

        metatable["getHP"]   = wrap_entity_fn(&Entity::get<Pool::HP>);
        metatable["getMP"]   = wrap_entity_fn(&Entity::get<Pool::MP>);
        metatable["getTech"] = wrap_entity_fn(&Entity::get<Pool::Tech>);

        metatable["getTeam"] = [](EntityLogger& el) {
            // create a new logged entity for every member in the team
            const auto team = el.system->getTeam(*el.entity);
            const auto members = el.system->getEntities(team);

            std::vector<EntityLogger> v;
            v.reserve(members.size());
            for (Entity* e : members)
                v.emplace_back(e, el.system, el.logger);
            return v;
        };
    }

    void loadStatsMetatable(sol::state_view& lua) {
        auto metatable = lua.new_usertype<Stats>("stats");

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

    void loadElements(sol::state_view& lua) {
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
    }

    void loadSkillEnums(sol::state_view& lua) {
        lua.new_enum<SkillMethod>("method", {
            { "physical", SkillMethod::Physical },
            { "magical",  SkillMethod::Magical },
            { "mixed",    SkillMethod::Mixed },
            { "none",     SkillMethod::None },
        });

        lua.new_enum<SkillSpread>("spread", {
            { "self",    SkillSpread::Self },
            { "single",  SkillSpread::Single },
            { "semiaoe", SkillSpread::SemiAoE },
            { "aoe",     SkillSpread::AoE },
            { "field",   SkillSpread::Field },
        });
    }

    void loadLuaPackages(sol::state_view& lua) {
        // now that we've set up all the usertypes, we're safe to load the
        // current list of possible skills
        lua.script_file("./data/skill/main.lua",
            [](lua_State*, sol::protected_function_result pfr) -> decltype(pfr) {
                sol::error err = pfr;
                throw std::runtime_error(
                    std::string{"error loading skill list: "} + err.what());
            }
        );
    }

    sol::state_view lua() {
        static sol::state lua = [](){
            sol::state lua;

            // load base lua libraries
            lua.open_libraries(
                sol::lib::base,    // required
                sol::lib::table,   // inserting into numbered lists
                sol::lib::math,    // random numbers and other math fns
                sol::lib::package  // require (TODO: do we want this?)
            );

            // prevent accidentally loading weird libraries and make sure we
            // actually get the libraries we *do* want
            lua.script("package.path = './data/?.lua'");

            // load types and metatables
            loadSkillEnums(lua);
            loadElements(lua);
            loadEntityLoggerMetatable(lua);
            loadStatsMetatable(lua);

            // actually load the config files
            loadLuaPackages(lua);

            return lua;
        }();

        return lua;
    }

}

// Entity configuration
namespace battle {

    std::tuple<Stats, std::vector<Skill>>
    getEntityDetails(const std::string& kind, const std::string& type, int level) {
        // TODO actually lua this
        // TODO even if not, can't wait for designated initializers (C++20)
        std::vector<Skill> skills;
        if (kind == "default" && type == "good") {
            Stats stats{
                27 * level,    // hp
                4 + level,     // mp
                10 + level,    // tech
                3 + level,     // p_atk
                4 + level / 2, // p_def
                2 + level / 2, // m_atk
                2 + level,     // m_def
                100,           // skill
                0,             // evade
                4 + level,     // speed
                { 0 },         // resist
            };
            skills.emplace_back("attack");

            return std::make_tuple(stats, std::move(skills));
        } else if (kind == "default" && type == "evil") {
            Stats stats{
                24 * level,    // hp
                4 + level,     // mp
                10 + level,    // tech
                3 + level / 2, // p_atk
                5 + level,     // p_def
                4 + level,     // m_atk
                2 + level / 2, // m_def
                80,            // skill
                20,            // evade
                6 + level,     // speed
                { 0 },         // resist
            };

            return std::make_tuple(stats, std::move(skills));
        }
        throw std::invalid_argument(kind + ":" + type + " is not a real entity type");
    }

}

// SkillDetails implementation
namespace battle {

    struct SkillDetails::LuaHandle {
        LuaHandle(const std::string& name, int level) {
            auto lua = config::lua();
            sol::protected_function fn = lua["skill"]["list"][name];
            if (!fn.valid())
                throw std::invalid_argument("unknown skill: " + name);
            sol::protected_function_result pfr = fn(level);
            if (pfr.valid()) {
                data = pfr;
            } else {
                sol::error err = pfr;
                throw std::invalid_argument("error getting skill " + name + " (level " +
                        std::to_string(level) + ") details: " + err.what());
            }
        }

        sol::table data;
    };

    void SkillDetails::Deleter::operator()(LuaHandle* handle) const noexcept {
        delete handle;
    }

    SkillDetails::SkillDetails(const std::string& name_, int level_)
        : name{ name_ }, level{ level_ }, handle{ new LuaHandle{ name_, level_ } }
    {
        const auto opt = [](sol::table t, const char* c) -> std::optional<int> {
            auto val = t[c];
            switch (val.get_type()) {
            case sol::type::number:
                return val;
            case sol::type::none:
                return std::nullopt;
            default:
                throw std::invalid_argument(
                    "'" + std::string{c} + "' was not a number'");
            }
        };

        const auto& t = handle->data;

        desc = t["desc"];
        max_level = t["max_level"].get_or(1);

        hp_cost = opt(t, "hp_cost");
        mp_cost = opt(t, "mp_cost");
        tech_cost = opt(t, "tech_cost");
        // items

        power = opt(t, "power");
        accuracy = opt(t, "accuracy");
        method = t["method"].get_or(SkillMethod::None);
        spread = t["spread"].get_or(SkillSpread::Single);
        element = t["element"].get_or(Element::Neutral);
    }

    void SkillDetails::perform(MessageLogger& logger,
            Entity& source, Entity& target,
            BattleSystem& system) const
    {
        config::EntityLogger src{ &source, &system, &logger };
        config::EntityLogger tgt{ &target, &system, &logger };

        sol::protected_function perform = handle->data["perform"];
        auto ret = perform(handle->data, src, tgt);
        if (!ret.valid()) {
            sol::error err = ret;
            // TODO: dedicated error type for failures here
            throw std::runtime_error(err.what());
        }

        // TODO: return some metadata on what happened? (e.g. miss, etc.)
    }

}
