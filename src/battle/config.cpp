#include "battle/battlesystem.h"
#include "battle/entity.h"
#include "battle/skill.h"
#include "battle/skilldetails.h"
#include "battle/stats.h"
#include "util/random.h"

#include <type_traits>
#include <cmath>

#define SOL_CHECK_ARGUMENTS 1
#include <sol/sol.hpp>

// helpers to set up the lua environment
namespace {

    using namespace battle;

    struct EntityLogger {
        EntityLogger(Entity* e, BattleSystem* s, MessageLogger* l)
            : entity{ e }, system{ s }, logger{ l }
        {}

        operator Entity&() noexcept { return *entity; }

        Stats& getStats() {
            if (!stat_cache)
                stat_cache = entity->getStats();
            return *stat_cache;
        }

        Entity* entity;
        BattleSystem* system;
        MessageLogger* logger;

    private:
        std::optional<Stats> stat_cache;
    };

    template <typename T, typename... Args>
    struct same_as_first : std::false_type {};
    template <typename T, typename... Args>
    struct same_as_first<T, T, Args...> : std::true_type {};
    template <typename T, typename... Args>
    inline constexpr bool same_as_first_v = same_as_first<T, Args...>::value;

    template <typename Ret, typename... Args>
    auto wrapper_helper(Ret (Entity::* ptr)(Args...) const noexcept) {
        return [ptr](const EntityLogger& self, Args&&... args) {
            if constexpr (same_as_first_v<MessageLogger, Args...>)
                return (self.entity->*ptr)(*self.logger, std::forward<Args>(args)...);
            else
                return (self.entity->*ptr)(std::forward<Args>(args)...);
        };
    }

    // wrap a function as a "property"
    // this should be a read-only, and either takes no arguments or just a logger
    template <typename Ret, typename... Args>
    auto wrap_entity_property(Ret (Entity::* ptr)(Args...) const noexcept) {
        static_assert(sizeof...(Args) <= 1);
        return sol::readonly_property(wrapper_helper(ptr));
    }

    // wrap a non-property function as an actual function
    template <typename FnPtr>
    auto wrap_entity_function(FnPtr ptr) {
        static_assert(std::is_member_function_pointer_v<std::decay_t<FnPtr>>);
        return wrapper_helper(ptr);
    }

    // wrap_entity_function but explicitly for `drain' and `restore' functions
    // provides automatic conversion in case we get a floating-point number
    auto wrap_drain_restore(void (Entity::* ptr)(MessageLogger& logger, int amt)) {
        return sol::overload(
            [ptr](EntityLogger& self, int amt) {
                (self.entity->*ptr)(*self.logger, amt);
            },
            [ptr](EntityLogger& self, double amt) {
                (self.entity->*ptr)(*self.logger, std::lround(amt));
            }
        );
    }


    void loadEntityLoggerMetatable(sol::state_view& lua) {
        auto metatable = lua.new_usertype<EntityLogger>("logged_entity",
            "new", sol::no_constructor);

        auto get_id_field = [](auto field) {
            return sol::readonly_property(
                [field](const EntityLogger& el){ return el.entity->getID().*field; }
            );
        };

        metatable["kind"] = get_id_field(&EntityID::kind);
        metatable["type"] = get_id_field(&EntityID::type);
        metatable["name"] = get_id_field(&EntityID::name);

        metatable["stats"] = sol::readonly_property(&EntityLogger::getStats);

        metatable["drainHealth"] = wrap_drain_restore(&Entity::drain<Pool::Health>);
        metatable["drainMana"]   = wrap_drain_restore(&Entity::drain<Pool::Mana>);
        metatable["drainTech"]   = wrap_drain_restore(&Entity::drain<Pool::Tech>);

        metatable["restoreHealth"] = wrap_drain_restore(&Entity::restore<Pool::Health>);
        metatable["restoreMana"]   = wrap_drain_restore(&Entity::restore<Pool::Mana>);
        metatable["restoreTech"]   = wrap_drain_restore(&Entity::restore<Pool::Tech>);

        metatable["level"]      = wrap_entity_property(&Entity::getLevel);
        metatable["experience"] = wrap_entity_property(&Entity::getExperience);

        metatable["is_dead"] = wrap_entity_property(&Entity::isDead);

        // need to do this because GCC has a bug;
        // see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=64194
        auto getHealth = &Entity::get<Pool::Health>;
        auto getMana = &Entity::get<Pool::Mana>;
        auto getTech = &Entity::get<Pool::Tech>;
        metatable["health"] = wrap_entity_property(getHealth);
        metatable["mana"] = wrap_entity_property(getMana);
        metatable["tech"] = wrap_entity_property(getTech);

        metatable["getTeam"] = [](EntityLogger& el) {
            // create a new logged entity for every living member in the team
            const auto members = el.system->teamMembersOf(el);

            std::vector<EntityLogger> v;
            for (Entity* e : members)
                if (!e->isDead())
                    v.emplace_back(e, el.system, el.logger);
            return v;
        };

        metatable["getDeadTeam"] = [](EntityLogger& el) {
            // create a new logged entity for every dead member in the team
            const auto members = el.system->teamMembersOf(el);

            std::vector<EntityLogger> v;
            for (Entity* e : members)
                if (e->isDead())
                    v.emplace_back(e, el.system, el.logger);
            return v;
        };
    }

    void loadStatsMetatable(sol::state_view& lua) {
        auto metatable = lua.new_usertype<Stats>("stats");

        metatable["max_health"] = &Stats::max_health;
        metatable["max_mealth"] = &Stats::max_mana;
        metatable["max_tech"] = &Stats::max_tech;

        metatable["p_atk"] = &Stats::p_atk;
        metatable["p_def"] = &Stats::p_def;
        metatable["m_atk"] = &Stats::m_atk;
        metatable["m_def"] = &Stats::m_def;
        metatable["skill"] = &Stats::skill;
        metatable["evade"] = &Stats::evade;
        metatable["react"] = &Stats::react;

        // not *quite* a real property
        metatable["resists"] = sol::overload(
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

    void loadMessageTypes(sol::state_view& lua) {
        // TODO: redesign how this is done - maybe put this in "skill"?
        auto msg = lua["message"].get_or_create<sol::table>();
        msg["miss"] = [](const EntityLogger& el) -> Message {
            return message::Miss{ *el.entity };
        };
        msg["critical"] = [](const EntityLogger& el) -> Message {
            return message::Critical{ *el.entity };
        };
        msg["notify"] = [](std::string s) -> Message {
            return message::Notification{ std::move(s) };
        };
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
                sol::lib::math,    // math fns
                sol::lib::package  // require (TODO: do we want this?)
            );

            // prevent accidentally loading weird libraries and make sure we
            // actually get the libraries we *do* want
            lua.script("package.path = './data/?.lua'");

            // create random functions using my generators for both reals and ints
            lua.set_function("random", sol::overload(
                [] { return util::random(0.0, 1.0); },
                [](long max) { return util::random(1, max); },
                [](long min, long max) { return util::random(min, max); }
            ));
            lua.set_function("randf", sol::overload(
                [] { return util::random(0.0, 1.0); },
                [](double max) { return util::random(0.0, max); },
                [](double min, double max) { return util::random(min, max); }
            ));
            // replace default random with my integer variant
            lua["math"]["random"] = lua["random"];

            // load types and metatables
            loadSkillEnums(lua);
            loadElements(lua);
            loadEntityLoggerMetatable(lua);
            loadStatsMetatable(lua);
            loadMessageTypes(lua);

            // set default logging global
            lua.script(R"(
                log_error = function(...)
                    error("using 'log' outside of execution context")
                end
                log = log_error
            )");

            // actually load the config files
            loadLuaPackages(lua);

            return lua;
        }();

        return lua;
    }

    /// RAII wrapper for setting the log function in lua
    /// This way log calls the right thing while giving errors when it's not
    /// supposed to be available
    struct ScopedLogger {
        // set the logging function
        template <typename Fn>
        ScopedLogger(Fn&& fn) {
            lua()["log"] = std::forward<Fn>(fn);
        }

        // clear the logging function when done
        ~ScopedLogger() {
            lua().script("log = log_error\n");
        }

        // disallow copying
        ScopedLogger(const ScopedLogger&) = delete;
        ScopedLogger& operator=(const ScopedLogger&) = delete;
    };

    template <typename Fn>
    [[nodiscard]] auto set_log(Fn&& fn) {
        return ScopedLogger(std::forward<Fn>(fn));
    }
}

// SkillDetails implementation
namespace battle {

    struct SkillDetails::LuaHandle {
        LuaHandle(const std::string& name, int level) {
            auto lua = ::lua();
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
        max_level = t["max_level"];

        hp_cost = opt(t, "health_cost");
        mp_cost = opt(t, "mana_cost");
        tp_cost = opt(t, "tech_cost");
        // items

        power = opt(t, "power");
        accuracy = opt(t, "accuracy");
        method = t["method"];
        spread = t["spread"];
        element = t["element"];
    }

    void SkillDetails::perform(MessageLogger& logger,
            Entity& source, Entity& target,
            BattleSystem& system) const
    {
        auto log = set_log([&logger](const Message& m) { logger.appendMessage(m); });

        ::EntityLogger src{ &source, &system, &logger };
        ::EntityLogger tgt{ &target, &system, &logger };

        sol::protected_function perform = handle->data["perform"];
        auto ret = perform(handle->data, src, tgt, log);
        if (!ret.valid()) {
            sol::error err = ret;
            // TODO: dedicated error type for failures here
            throw std::runtime_error(err.what());
        }

        // TODO: return some metadata on what happened? (e.g. miss, etc.)
    }

}
