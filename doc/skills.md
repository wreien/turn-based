# Skills

Skills are generated in Lua, and consist of a list of key-value pairs defining
their functionality. We define in config files the details for a particular
instance of a skill. This is immutable, and never changes; if anything
important changes for the skill we regenerate the information with the new
data. (This means stay away from random values in skill data! --- though in
the `perform` function it's fine.) A skill's details is uniquely determined by
the triple "name, level, perks".

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

  Attribute   | Type    | Description
 -------------|---------|--------------------------------------------
  `desc`        | string  | An English description for the skill's effect
  `max_level`   | number  | The maximum level the skill can reach; default `1`

Note that `max_level` should be an integer — that is, it cannot be a
fractional value.

**TODO**: provide a list of valid perks + their descriptions? (These just need
to be strings)

### Costs

Checks to make sure that the skill is castable, and what the cast will cost
the user. All of these fields may be, and default to, `nil`, representing no
cost of that form.

  Attribute   | Type    | Description
 -------------|---------|--------------------------------------------
  `hp_cost`     | number  | The amount of HP required to cast the skill
  `mp_cost`     | number  | The amount of MP required to cast the skill
  `tech_cost`   | number  | The amount of Tech required to cast the skill
  `items`       | list    | Any other components required to cast the skill

Apart from `items`, all cost values must be integers (whole numbers); you may
make use of the "flooring" division operator `//` to ensure this (or otherwise
use `math.floor(number)`) if there is a possibility of creating fractional
numbers to forcefully round down to the nearest whole number.

On the other hand, `items` is a list of items required (as strings); specify
the same item multiple times if necessary, such as:

```lua
items = { "herb", "herb", "flower" }
```

(Items have not yet been implemented.)

### Attributes

Relevant to the actual functionality of the skill. Unless otherwise specified,
they default to `nil`, a sort of "not-applicable" value. For example, a skill
to poison an enemy would have `power = nil`, and could therefore just leave
it out, but should specify `accuracy = 100` (unless it can't miss).

  Attribute  | Type    | Description
 ------------|---------|---------------------------------------------
  `power`      | number  | The base damage for skill
  `accuracy`   | number  | The base percentage chance for the skill to hit
  `method`     | method  | What stats of the source it uses; default `method.none`
  `spread`     | spread  | Who the skill targets; default `spread.single`
  `element`    | element | What element the skill is; default `element.neutral`

Like with the costs, `power` and `accuracy` should be integers.

Possible values for `method`:

  Value           | Description
  ----------------|------------------------------------------------------
  `method.physical` | Deals primarily physical damage (uses `p_atk` and `p_def`)
  `method.magical`  | Deals primarily magical damage (uses `m_atk` and `m_def`)
  `method.mixed`    | Deals damage that fits as neither physical nor magical
  `method.none`     | Doesn't deal direct damage

Possible values for `spread`:

  Value          | Description
  ---------------|--------------------------------------------------------
  `spread.self`    | Can only target the skill user (source = target)
  `spread.single`  | Targets any one entity
  `spread.aoe`     | Target an entire team
  `spread.semiaoe` | Targets an entire team, but focusses on an individual
  `spread.field`   | Targets the entire battlefield (unimplemented)

Finally, `element` is any one of the available elements. See [the elements
documentation](elements.md) for more details.

**TODO**: probably the attribute names and type names should be different some
way; maybe some kind of namespacing for the types?

## Functionality

To define exactly what the skill does, a data value `perform` must be
provided. This is a function taking three parameters (you may pick the names,
but the following are standard):

- `self` or `s`: me, the skill using the function (and its details)
- `source`: whoever used the skill in the first place
- `target`: the entity the skill targeted (or `nil` if `spread.field`)

`self` (or `s`) contains all the data specified in the skill you returned. For
example, if you returned a skill with 80 power, then `self.power` is 80.
Prefer to use this value rather than recalculating the desired attributes. The
only calculated values you might ever need are `level` and `perks`, which are
as described [above](#representation).

For details on the API for `source` and `target`, please see [the entity
documentation](entity.md).

While developing, a useful placeholder `perform` function is
`skill.default_perform`, which guesses a standard implementation based upon
the skill's attributes. See [the helper function documentation](func.md)
for more details.

## Example

Here is a detailed example of a skill, demonstrating usage of most of the above.

```lua
-- we define a skill named "Eruption"
skill.list["Eruption"] = function(level, perks)
    -- we set up some defaults
    local power = 50 + 15 * level
    local accuracy = 80
    -- we call it newspread rather than spread so we
    -- can still access the global enum "spread" later
    local newspread = spread.semiaoe

    -- we manage our perks
    if perks["overheat"] then
        power = power + 20
        accuracy = accuracy - 10
    end
    if perks["even spread"] then
        -- make sure we end up with a whole number!
        power = power // 2
        newspread = spread.aoe
    end

    -- now we actually provide the data for our skill
    return {
        -- for niceness, we'll split the long description over multiple lines
        desc = "Unleash a volcanic eruption at a target, "
            .. "damaging their whole team.",
        maxlevel = 5,

        -- we leave out hp_cost here as it's not relevant
        mp_cost = 30,
        tech_cost = 10,
        -- we need two fire stones to use the skill
        items = { "fire stone", "fire stone" },

        -- pass in our calculated values
        power = power,
        accuracy = accuracy,
        spread = newspread,
        method = method.magical,
        element = element.fire,

        -- provide a default function to actually do things
        perform = function(self, source, target)
            -- can use perks inside perform function too
            if not perks["consistency"] then
                if random(10) == 1 then
                    log(message.notify("But it failed!"))
                    return -- quit early
                end
            end

            -- delegate to a different perform function
            skill.default_perform(self, source, target)
        end
    }
end
```
