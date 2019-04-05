# Skills

Skills are generated in Lua, and consist of a list of key-value pairs defining
their functionality.

_Skills_ here more accurately refer to _skill templates_, which construct a
"type" of a skill that can then be generated and specialised. Individual
instances of skills can then be modified --- through level-ups, perks, or (if
you're feeling frisky) directly replacing skill components --- without
affecting other entities that happen to have the same base skill.

The direct owner of a skill isn't necessarily an entity. As a quick list,
skills can belong to:

- the entity, for generic learnt skills and skills from the development grid
- a "stance", denoting skills that can only be used while the entity is in a
  particular form
- a consumable, representing the effect of the item
- a piece of equipment, as a skill granted whilst wearing it
- a buff, e.g. the second stage of a "charge up → attack" skill

## Representation

To specify a skill, declare a function taking `level` and `perks` as follows:

```lua
require "skill.base"

skill.list["Name of Skill"] = function(level, perks)
  return {
    -- skill definition here; see below for details.
  }
end
```

Obviously, `"Name of Skill"` should be unique; if the same skill is defined
twice, the later construction has priority. The information passed is as
follows:

- `level` represents the skill level to construct the skill at. It must be a
  value between `1` and `skill.max_level` inclusive.

- `perks` is a set of perks to be applied. You can test for the existence of a
  particular perk with

  ```lua
  if perks["power boost"] then
    -- do something
  end
  ```

You should return a list of key-value data representing the data and
functionality of the skill.

## Data

Data is used both internally within the skill to "do things", as well as
within the calling C++ code to generate metainfo and descriptions for a skill.
When creating a skill, some code is run to error-check a level 1 instance of
the new skill to make sure that all the preconditions of the data is
fulfilled.

### Informational

Bookkeeping for the skill. `desc` is required, but `max_level` can be
defaulted to `1`. Other information may appear here in the future, for example
categorising skill types in the UI or linking to sprite/animation data.

  Attribute   | Description
 -------------|------------------------------------------------------
  `desc`      | An English description for the skill's effect
  `maxlevel`  | The maximum level the skill can reach; default `1`

### Costs

Checks to make sure that the skill is castable, and what the cast will cost
the user. All of these fields may be, and default to, `nil`, representing no
cost of that form.

  Attribute   | Description
 -------------|------------------------------------------------------
  `hp_cost`   | The amount of HP required to cast the skill
  `mp_cost`   | The amount of MP required to cast the skill
  `tech_cost` | The amount of Tech required to cast the skill
  `items`     | Any other components required to cast the skill

Apart from `items`, all cost values must be integers (whole numbers); you may
make use of the "flooring" division operator `//` to ensure this (or otherwise
use `math.floor(number)`{.lua}) if there is a possibility of creating
fractional numbers to forcefully round down to the nearest whole number.

On the other hand, `items` is a list of items required; specify the same item
multiple times if necessary, such as:

```lua
items = { "herb", "herb", "flower" }
```

### Attributes

Relevant to the actual functionality of the skill. Unless otherwise specified,
they default to `nil`, a sort of "not-applicable" value. For example, a skill
to poison an enemy would have `power = nil`, and could therefore just leave
it out, but should specify `accuracy = 100` (unless it can't miss).

  Attribute  | Description
 ------------|------------------------------------------------------
  `power`    | The base damage for skill
  `accuracy` | The base percentage chance for the skill to hit
  `method`   | What stats of the source it uses; default `method.none`
  `spread`   | Who the skill targets; default `spread.single`
  `element`  | What element the skill is; default `element.neutral`

Like with the costs, `power` and `accuracy` should be integers.

`method` can be one of `physical`, `magical`, `mixed`, or `none`. You must
specify this as, for example, `method.none`.

`spread` can be one of `self`, `single`, `semiaoe`, `aoe`, and `field`. You
must specify this as, for example, `spread.single`.

`element` is any one of the available elements. (See [the elements
documentation](elements.md).)
