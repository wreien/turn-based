#ifndef BATTLE_ELEMENT_H_INCLUDED
#define BATTLE_ELEMENT_H_INCLUDED

#include <tuple>
#include <optional>

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

[[nodiscard]] constexpr bool isPrimaryElement(Element e) noexcept {
    return e == Element::Fire
        || e == Element::Water
        || e == Element::Earth
        || e == Element::Air
        || e == Element::Light
        || e == Element::Dark;
}

[[nodiscard]] constexpr bool isSecondaryElement(Element e) noexcept {
    return e == Element::Ice
        || e == Element::Lightning
        || e == Element::Sand
        || e == Element::Steam
        || e == Element::Life
        || e == Element::Metal;
}

[[nodiscard]] constexpr auto constituentElements(Element e) noexcept
    -> std::optional<std::tuple<Element, Element>>
{
    if (!isSecondaryElement(e))
        return std::nullopt;

#define ELEMENT_PARTS(elem,p1,p2) \
    if (e == Element::elem) return std::make_tuple(Element::p1,Element::p2)

    ELEMENT_PARTS(Ice,       Water, Air);
    ELEMENT_PARTS(Lightning, Air,   Fire);
    ELEMENT_PARTS(Metal,     Fire,  Earth);
    ELEMENT_PARTS(Life,      Earth, Water);
    ELEMENT_PARTS(Steam,     Water, Fire);
    ELEMENT_PARTS(Sand,      Air,   Earth);

#undef ELEMENT_PARTS

    return std::nullopt;
}


} // namespace battle

#endif // BATTLE_ELEMENT_H_INCLUDED
