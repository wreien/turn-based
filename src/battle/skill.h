#ifndef BATTLE_SKILL_H_INCLUDED
#define BATTLE_SKILL_H_INCLUDED

#include <string>
#include <optional>
#include <memory>
#include <vector>
#include <type_traits>
#include <algorithm>
#include <variant>
#include "element.h"

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

    /// Determines the way the skill deals effects
    enum class Method {
        Physical, ///< uses source's physical stats
        Magical,  ///< uses source's magical stats
        Neither,  ///< uses neither source's magical nor physical stats
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
        static_assert(std::is_same_v<Hook, CostHook>
                   || std::is_same_v<Hook, CheckHook>
                   || std::is_same_v<Hook, ModifierHook>
                   || std::is_same_v<Hook, EffectHook>
                   || std::is_same_v<Hook, PostHook>,
                      "Must provide a valid hook type");

        using HookType = Hook;

        constexpr HookID(uint64_t id) noexcept : id{ id } {}
        constexpr HookID(std::string_view id) noexcept
            : id{ std::hash<std::string_view>{}(id) }
        {}

        friend constexpr bool operator==(HookID lhs, HookID rhs) noexcept {
            return lhs.id == rhs.id;
        }
        friend constexpr bool operator!=(HookID lhs, HookID rhs) noexcept {
            return !(lhs == rhs);
        }

    private:
        uint64_t id;
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

    /// Skill hook to apply pre-use costs to the skill.
    /// Allows a skill to disallow choice if the requirements are unavailable.
    struct CostHook {
        CostHook(HookID<CostHook> id) : id{ id } {}
        virtual ~CostHook() = default;
        const HookID<CostHook> id;

        /// Check if the entity can pay the given cost.
        [[nodiscard]] virtual bool canPay(const Entity& source) const noexcept = 0;

        /// Make the entity pay the cost.
        virtual void pay(MessageLogger& logger, Entity& source) const noexcept = 0;

        /// Descriptor of what the cost actually is; returns nothing if inapplicable
        [[nodiscard]] virtual std::string getMessage() const noexcept {
            return "";
        }
    };

    /// Skill hook to determine if the skill hit a given target.
    struct CheckHook {
        CheckHook(HookID<CheckHook> id) : id{ id } {}
        virtual ~CheckHook() = default;
        const HookID<CheckHook> id;

        /// Check if the skill hit the given target.
        [[nodiscard]] virtual bool didHit(MessageLogger& logger,
               const Entity& source, const Entity& target) const noexcept = 0;

        /// Get the base accuracy of the check, if applicable.
        /// An accuracy of 50 means a base of 50% chance of hitting.
        /// Accuracies are multiplied together.
        [[nodiscard]] virtual std::optional<int> getAccuracy() const noexcept {
            return std::nullopt;
        }
    };

    /// Skill hook to calculate damage modifiers for the skill.
    struct ModifierHook {
        ModifierHook(HookID<ModifierHook> id) : id{ id } {}
        virtual ~ModifierHook() = default;
        const HookID<ModifierHook> id;

        /// Calculate the damage modifier for the skill.
        /// Should return an integer representing the percentage change;
        /// for example, 50 means "50% more damage".
        [[nodiscard]] virtual int mod(MessageLogger& logger,
                const Entity& source, const Entity& target) const noexcept = 0;

        /// Returns a descriptor for the modifier of the skill. This is
        /// either the element of the skill, or a string describing its effect.
        [[nodiscard]] virtual
            std::variant<Element, std::string> getDescription() const noexcept = 0;
    };

    /// Skill hook to actually do things.
    /// Note that this can involve affecting both the target _and_ yourself.
    /// This hook gets called for every target; if you want to apply something
    /// to the source only at the end of the skill's effects, use PostHook.
    struct EffectHook {
        EffectHook(HookID<EffectHook> id) : id{ id } {}
        virtual ~EffectHook() = default;
        const HookID<EffectHook> id;

        /// Apply effects to the source and target.
        /// `mod` represents the overall damage multiplier
        /// (e.g. 1.2 means "20% more damage")
        virtual void apply(MessageLogger& logger,
                Entity& source, Entity& target, double mod) const noexcept = 0;

        /// Return the power for the skill, if applicable.
        [[nodiscard]] virtual std::optional<int> getPower() const noexcept {
            return std::nullopt;
        }

        /// Returns the method of how the skill does effects, if applicable.
        [[nodiscard]] virtual std::optional<Method> getMethod() const noexcept {
            return std::nullopt;
        }
    };

    /// Skill hook to run after everything else is done.
    /// Can be used to do final effects on the source.
    struct PostHook {
        PostHook(HookID<PostHook> id) : id{ id } {}
        virtual ~PostHook() = default;
        const HookID<PostHook> id;

        /// Apply effects to the source after everything else is done.
        virtual void apply(MessageLogger& logger, Entity& source) const noexcept = 0;

        /// Description of what the hook does
        [[nodiscard]] virtual std::string getMessage() const noexcept = 0;
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

    /// Get the attack method (source stats used) of the skill
    [[nodiscard]] skill::Method getMethod() const noexcept;

    /// If applicable, get the power of the skill
    [[nodiscard]] std::optional<int> getPower() const noexcept;

    /// If applicable, get the percentage chance of scoring a hit with the skill
    [[nodiscard]] std::optional<int> getAccuracy() const noexcept;

    /// Get the primary element associated with the skill.
    [[nodiscard]] Element getElement() const noexcept;

    /// Get a human-readable description of the skill's costs.
    /// An empty string means there are no costs.
    [[nodiscard]] std::string getCostDescription() const noexcept;

    /// Get a list of human-readable descriptions of the modifiers for this skill
    [[nodiscard]] std::vector<std::string> getModifierDescriptions() const noexcept;

    /// Get a list of human-readable descriptions of the post-use effects for this skill
    [[nodiscard]] std::vector<std::string> getUseEffectDescriptions() const noexcept;

    /// Add a hook to the given hook list
    /// Requires "Hook" to be a valid hook type
    template <typename Hook>
    void addHook(Hook&& hook) {
        removeHook(hook.id); // don't have two of the same hook
        auto& hook_list = getHookList<Hook>();
        auto ptr = std::make_unique<std::remove_reference_t<Hook>>(
                std::forward<Hook>(hook));
        hook_list.push_back(std::move(ptr));
    }

    /// Remove a hook from the hook lists, if it exists
    template <typename Hook>
    void removeHook(skill::HookID<Hook> id) {
        auto& hook_list = getHookList<Hook>();
        // does hook order matter? Just in case doing stable partition; rethink later
        auto to_erase = std::stable_partition(
            std::begin(hook_list), std::end(hook_list),
            [id](auto&& hook) { return hook->id != id; }
        );
        hook_list.erase(to_erase, std::end(hook_list));
    }

private:
    std::string name;    ///< the name of the skill
    skill::Spread spread; ///< the spread of the skill

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
