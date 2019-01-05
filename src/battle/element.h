#ifndef BATTLE_ELEMENT_H_INCLUDED
#define BATTLE_ELEMENT_H_INCLUDED


// Deal with elements
// At the moment, this file is not being included anywhere, as it's
// still uncertain at this stage as to how the mechanics for elements
// will actually work. What's here is a placeholder for how primary
// and secondary elements will likely fit together.


namespace battle {


// lists valid elements for a skill
enum class Element : unsigned char {
    // Primary Elements
    Fire  = 1 << 0,
    Water = 1 << 1,
    Earth = 1 << 2,
    Air   = 1 << 3,
    Light = 1 << 4,
    Dark  = 1 << 5,

    // Secondary Elements
    Ice       = Air   | Water,
    Lightning = Air   | Fire,
    Sand      = Air   | Earth,
    Steam     = Water | Fire,
    Life      = Water | Earth,
    Metal     = Fire  | Earth,
};


// determines if an element is a primary element
[[nodiscard]] inline constexpr bool isPrimaryElement(Element e) noexcept {
    switch (e) {
    case Element::Fire:
    case Element::Water:
    case Element::Earth:
    case Element::Air:
    case Element::Light:
    case Element::Dark:
        return true;

    default:
        return false;
    }
}

// determines if an element is a secondary element
[[nodiscard]] inline constexpr bool isSecondaryElement(Element e) noexcept {
    switch (e) {
    case Element::Ice:
    case Element::Lightning:
    case Element::Sand:
    case Element::Steam:
    case Element::Life:
    case Element::Dark:
        return true;

    default:
        return false;
    }
}

// determines if an element is a valid element
[[nodiscard]] inline constexpr bool isValidElement(Element e) noexcept {
    return isPrimaryElement(e) || isSecondaryElement(e);
}


} // namespace battle

#endif // BATTLE_ELEMENT_H_INCLUDED
