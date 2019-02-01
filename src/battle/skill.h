#ifndef BATTLE_SKILL_H_INCLUDED
#define BATTLE_SKILL_H_INCLUDED

#include <string>
#include <string_view>
#include <optional>
#include <memory>
#include <vector>
#include <unordered_map>
#include <type_traits>
#include <algorithm>
#include "stats.h"

namespace battle {


class Entity;
class MessageLogger;

namespace skill {

    /// Determines the spread of an attack
    /// TODO: field attacks (probably not actually a `skill' in the same way)
    enum class Spread {
        Self,    ///< targets source
        Single,  ///< targets one entity
        SemiAoE, ///< targets one entity, with spread to rest of their team
        AoE,     ///< targets a whole team
    };

    struct CostHook;
    struct CheckHook;
    struct ModifierHook;
    struct EffectHook;
    struct PostHook;

    /// Identifies a hook; allows for replacement or deletion by this id
    template <typename Hook>
    class HookID {
    public:
        using HookType = Hook;

        constexpr HookID(std::string_view name) noexcept
            : hash{ std::hash<std::string_view>{}(name) }
        {
            static_assert(std::is_same_v<Hook, CostHook>
                       || std::is_same_v<Hook, CheckHook>
                       || std::is_same_v<Hook, ModifierHook>
                       || std::is_same_v<Hook, EffectHook>
                       || std::is_same_v<Hook, PostHook>,
                          "Must provide a valid hook type");
        }

        friend constexpr bool operator==(HookID lhs, HookID rhs) noexcept {
            return lhs.hash == rhs.hash;
        }
        friend constexpr bool operator!=(HookID lhs, HookID rhs) noexcept {
            return !(lhs == rhs);
        }

    private:
        std::size_t hash;
    };

    /// Determine if a given type is a hook
    template <typename T, typename = void> struct is_hook : std::false_type {};
    template <typename T> struct is_hook<T, std::enable_if_t<
        // need to make sure only *one* of the following is true
        // relying on equivalence between `bool' and `1', and integer promotion rules
        std::is_base_of_v<CostHook, T>
      + std::is_base_of_v<CheckHook, T>
      + std::is_base_of_v<ModifierHook, T>
      + std::is_base_of_v<EffectHook, T>
      + std::is_base_of_v<PostHook, T>
     == 1
    >> : std::true_type {};
    template <typename T> inline constexpr auto is_hook_v = is_hook<T>::value;

    template <typename T, typename = void> struct has_power : std::false_type {};
    template <typename T> struct has_power<T, std::enable_if_t<
        std::is_base_of_v<EffectHook, T>
     && std::is_same_v<decltype(std::declval<T>().power), int>
    >> : std::true_type {};
    template <typename T> inline constexpr auto has_power_v = has_power<T>::value;

    template <typename T, typename = void> struct has_accuracy : std::false_type {};
    template <typename T> struct has_accuracy<T, std::enable_if_t<
        std::is_base_of_v<CheckHook, T>
     && std::is_same_v<decltype(std::declval<T>().accuracy), int>
    >> : std::true_type {};
    template <typename T> inline constexpr auto has_accuracy_v = has_accuracy<T>::value;

    /// Skill hook to apply pre-use costs to the skill.
    /// Allows a skill to disallow choice if the requirements are unavailable.
    struct CostHook {
        CostHook(std::string_view name) : id{ name } {}
        virtual ~CostHook() = default;
        const HookID<CostHook> id;

        /// Check if the entity can pay the given cost.
        [[nodiscard]] virtual bool
            canPay(const Entity& source) const noexcept = 0;

        /// Make the entity pay the cost.
        virtual void pay(MessageLogger& logger, Entity& source) const noexcept = 0;
    };

    /// Skill hook to determine if the skill hit a given target.
    struct CheckHook {
        CheckHook(std::string_view name) : id{ name } {}
        virtual ~CheckHook() = default;
        const HookID<CheckHook> id;

        /// Check if the skill hit the given target.
        [[nodiscard]] virtual bool didHit(MessageLogger& logger,
               const Entity& source, const Entity& target) const noexcept = 0;
    };

    /// Skill hook to calculate damage modifiers for the skill.
    struct ModifierHook {
        ModifierHook(std::string_view name) : id{ name } {}
        virtual ~ModifierHook() = default;
        const HookID<ModifierHook> id;

        /// Calculate the damage modifier for the skill.
        /// Should return an integer representing the percentage change;
        /// for example, 50 means "50% more damage".
        [[nodiscard]] virtual int mod(MessageLogger& logger,
                const Entity& source, const Entity& target) const noexcept = 0;
    };

    /// Skill hook to actually do things.
    /// Note that this can involve affecting both the target _and_ yourself.
    /// This hook gets called for every target; if you want to apply something
    /// to the source only at the end of the skill's effects, use PostHook.
    struct EffectHook {
        EffectHook(std::string_view name) : id{ name } {}
        virtual ~EffectHook() = default;
        const HookID<EffectHook> id;

        /// Apply effects to the source and target.
        /// `mod` represents the overall damage multiplier
        /// (e.g. 1.2 means "20% more damage")
        virtual void apply(MessageLogger& logger,
                Entity& source, Entity& target, double mod) const noexcept = 0;
    };

    /// Skill hook to run after everything else is done.
    /// Can be used to do final effects on the source.
    struct PostHook {
        PostHook(std::string_view name) : id{ name } {}
        virtual ~PostHook() = default;
        const HookID<PostHook> id;

        /// Apply effects to the source after everything else is done.
        virtual void apply(MessageLogger& logger, Entity& source) const noexcept = 0;
    };

}


/// Encapsulates a skill
class Skill {
public:
    /// Create the a specified skill
    explicit Skill(const std::string& name);

    // Explicit move constructors for MSVC
    // see https://stackoverflow.com/questions/54421110/
    Skill(Skill&&) = default;
    Skill& operator=(Skill&&) = default;

    /// Get the display name of the skill
    [[nodiscard]] std::string_view getName() const noexcept {
        return name;
    }

    /// Determines if the skill can currently be used by the provided entity
    [[nodiscard]] bool isUsableBy(const Entity& source) const noexcept;

    /// Process the effects of `source' using this skill on `target' with team `team'
    /// Logs to `logger'
    void use(MessageLogger& logger, Entity& source, Entity& target,
             const std::vector<Entity*>& team) const noexcept;

    /// Get the attack spread (AOE-ness) of the skill
    [[nodiscard]] skill::Spread getSpread() const noexcept;

    /// If applicable, get the power of the skill
    [[nodiscard]] std::optional<int> getPower() const noexcept;

    /// If applicable, get the percentage chance of scoring a hit with the skill
    [[nodiscard]] std::optional<int> getAccuracy() const noexcept;

    /// Add a hook to the given hook list
    /// Requires "Hook" to be a valid hook type
    template <typename Hook>
    void addHook(Hook&& hook) {
        removeHook(hook.id); // don't have two of the same hook
        auto& hook_list = getHookList<Hook>();
        auto ptr = std::make_unique<std::remove_reference_t<Hook>>(
                std::forward<Hook>(hook));
        // TODO: handle this more nicely than sticking it awkwardly in here
        if constexpr (skill::has_power_v<Hook>)
            power_sources[ptr.get()] = hook.power;
        if constexpr (skill::has_accuracy_v<Hook>)
            accuracy_sources[ptr.get()] = hook.accuracy;
        hook_list.push_back(std::move(ptr));
    }

    /// Remove a hook from the hook lists, if it exists
    template <typename Hook>
    void removeHook(skill::HookID<Hook> id) {
        auto& hook_list = getHookList<Hook>();
        auto to_erase = std::remove_if(   // reminder: remove_if is a terrible name
            std::begin(hook_list), std::end(hook_list),
            [id](auto&& hook) { return hook->id == id; }
        );
        // TODO: handle this more nicely than sticking it awkwardly in here
        if constexpr (std::is_same_v<skill::EffectHook, Hook>) {
            std::for_each(to_erase, std::end(hook_list), [&](auto&& x){
                power_sources.erase(x.get());
            });
        }
        if constexpr (std::is_same_v<skill::CheckHook, Hook>) {
            std::for_each(to_erase, std::end(hook_list), [&](auto&& x){
                accuracy_sources.erase(x.get());
            });
        }
        hook_list.erase(to_erase, std::end(hook_list));
    }

private:
    std::string name;    ///< the name of the skill
    skill::Spread spread; ///< the spread of the skill

    ///< the hooks that contribute to the skill's power
    std::unordered_map<skill::EffectHook*, int> power_sources;

    ///< the hooks that contribute to the skill's accuracy
    std::unordered_map<skill::CheckHook*, int> accuracy_sources;

    std::vector<std::unique_ptr<skill::CostHook>> cost_hooks;
    std::vector<std::unique_ptr<skill::CheckHook>> check_hooks;
    std::vector<std::unique_ptr<skill::ModifierHook>> mod_hooks;
    std::vector<std::unique_ptr<skill::EffectHook>> effect_hooks;
    std::vector<std::unique_ptr<skill::PostHook>> post_hooks;

    /// Get the appropriate list of hooks depending on the hook type
    template <typename Hook>
    [[nodiscard]] constexpr auto& getHookList() noexcept {
        using namespace skill;
        static_assert(skill::is_hook_v<Hook>);
        if constexpr (std::is_base_of_v<CostHook, Hook>)
            return cost_hooks;
        if constexpr (std::is_base_of_v<CheckHook, Hook>)
            return check_hooks;
        if constexpr (std::is_base_of_v<ModifierHook, Hook>)
            return mod_hooks;
        if constexpr (std::is_base_of_v<EffectHook, Hook>)
            return effect_hooks;
        if constexpr (std::is_base_of_v<PostHook, Hook>)
            return post_hooks;
    }

    /// Execute the skill with given source on the specified target.
    /// Also provides `orig' representing the original target (for AOE).
    void executeSkill(MessageLogger& logger,
            Entity& source, Entity& target, const Entity& orig) const noexcept;
};


}


#endif // BATTLE_SKILL_H_INCLUDED
