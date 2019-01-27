#ifndef BATTLE_SKILLREF_H_INCLUDED
#define BATTLE_SKILLREF_H_INCLUDED


namespace battle {


struct Skill;

/// Opaque skill reference
class SkillRef {
public:
    SkillRef(const Skill& skill) noexcept : skill{ &skill } {}

    friend bool operator==(const SkillRef& lhs, const SkillRef& rhs) noexcept {
        return lhs.skill == rhs.skill;
    }
    friend bool operator!=(const SkillRef& lhs, const SkillRef& rhs) noexcept {
        return !(lhs == rhs);
    }

    const Skill& get() const noexcept { return *skill; }
    const Skill& operator*() const noexcept { return *skill; }
    const Skill* operator->() const noexcept { return skill; }

private:
    const Skill* skill;
};


}


#endif // BATTLE_SKILLREF_H_INCLUDED
