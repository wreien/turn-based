#ifndef BATTLE_ELEMENT_H_INCLUDED
#define BATTLE_ELEMENT_H_INCLUDED


namespace battle {


/// Lists valid elemental types for a skill
enum class Element {
    Neutral = 0,

    // Primary Elements
    Fire,
    Water,
    Earth,
    Air,
    Light,
    Dark,

    // Secondary Elements
    Ice,
    Lightning,
    Sand,
    Steam,
    Life,
    Metal,

    // Housekeeping -- should not be used in code
    // Would be nice to replace with reflection
    _count
};

static constexpr const unsigned num_elements = static_cast<unsigned>(Element::_count);


} // namespace battle

#endif // BATTLE_ELEMENT_H_INCLUDED
