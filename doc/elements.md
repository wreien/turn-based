# Elements

Elements participate in calculating resistances between skills and entities.
Elements also have status effects associated with them, and generally entities
of a particular element have common strengths/weaknesses (though not always!).

There are three classes of elements:

- neutral; the default element, which encompasses everything that otherewise
  doesn't belong in an element
- primary; the building block elements
- secondary; specialised elements formed from combinations of primary elements

For an element named `X`, we can specify that element in configuration files
as `element.X`.

## Neutral

The neutral element (specified as `element.neutral` in config files) is the
catch-all for anything that doesn't have an element. For example, basic
attacks will tend to have no element. It's also the element for most status
effects, unless they are explicitly associated with a different element; for
example, Sleep is a neutral element.

It is possible to grant resistance to the neutral element for entities, but I
wouldn't recommend it without careful thought due to balance issues. There are
no particular attributes in common for entities with neutral element.

## Primary

These elements are the building blocks for elemental creatures. They
correspond to the four classical elements as well as the two "moral" elements,
Light and Dark.

The primary elements are:

- Fire (`element.fire`)
- Water (`element.water`)
- Earth (`element.earth`)
- Air (`element.air`)
- Light (`element.light`)
- Dark (`element.dark`)

## Secondary

These elements are formed from combinations of primary elements. At the moment
there are no secondary elements constituting from the moral elements, however.

Resistances for secondary elements are calculated in an interesting fashion.
Entities can have specific affinities to a secondary element, and this affects
their resistance to the element, but their resistances to the constituent
primary elements for the secondary element are also considered. For example,
an entity's resistance to Ice is calculated based on not only their Ice
resistance, but also their Water and Air resistances.

_(TODO: what is the exact formula?)_

The secondary elements are:

- Ice (`element.ice`); formed from Water and Air
- Lightning (`element.lightning`); formed from Air and Fire
- Steam (`element.steam`); formed from Fire and Water
- Life (`element.life`); formed from Water and Earth
- Metal (`element.metal`); formed from Earth and Fire
- Dust (`element.sand`); formed from Air and Earth

_(TODO: finalise what we call the Dust element, and update `element.sand` to
match)_
