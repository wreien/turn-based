# Entities

Entities are currently defined using initialization files; read some of the
files in `data/entity/` for examples. They're very much still a work in
progress!

However, their interface into Lua code for use in skills is fairly set, which
is what this document will describe. There will be minor changes as things
progress, but this should give you an idea.

Entities have both attributes and functions. The syntax for retrieving an
attribute `attr` for an entity `e` is `e.attr`. The syntax for calling a
function `func` for an entity `e` (sometimes with parameters `params`) is
`e:func(params)`. For example:

```lua
-- assuming 'e' is an entity
e.kind         -- what kind of entity this is
e.level        -- what level the entity is
e:drainHP(10)  -- do some damage
```

## Identification

There are three identifying traits for an entity. At the top level is their
"kind": this is the 'class' of entity, or the overarching form for this a set
of entities. For example, you might have a kind of "golem", or "cat", or
"human".

Each "kind" of entity has a set of individual entity "types": this is the
'species' of entity, or a specific manifestation of a class. So, you might
have an "aero golem", or a "blitz hyena", or a "bandit chieftain". This is
what ultimately determines the stats and abilities for an entity.

Finally, each entity also has a "name". This uniquely identifies a given
entity in a battle. Usually this will be the type of entity appended with
"#X"; for example, "blitz hyena #2" (if there is at least one other blitz
hyena in the battle). However, some special entities might have more
interesting names; for example, a bandit chieftain might be called "Jeff".

Each of these identifying traits can be retrieved with the following
attributes:

  Attribute | Type   | Description
  ----------|--------|------------------------------------------------
  `kind`      | string | The "kind" of entity (e.g. cat)
  `type`      | string | The "type" of entity (e.g. blitz hyena)
  `name`      | string | The identifying name of the entity (e.g. blitz hyena #2)

## Data

Some other details about entities, ranked in approximate order of usefulness:

  Attribute    | Type   | Description
  -------------|--------|-----------------------------------------------
  `stats`        | stats  | The entity's stat block; see below for details
  `is_dead`      | bool   | Whether the entity is dead or not
  `hp`           | number | The entity's current HP
  `mp`           | number | The entity's current MP
  `tech`         | number | The entity's current Tech
  `level`        | number | What level the entity is (e.g. 5); integer
  `experience`   | number | Current experience/experience granted on death

All numbers are integers.

Most are fairly self-explanatory. The `experience` attribute should almost
never be useful; for players, it gives the experience they need to reach the
next level, and for enemies it gives the experience they contribute on death.

## Stats

This is the stat block for the entity, which can be retrieved with the `stats`
attribute. This is recalculated at the start of each skill use to account for
changes in equipment, status effects, stances, and so forth. Note this means
that a skill cannot, for example, apply a status effect to halve a target's
defense and then deal (effectively) double damage — the change in defense stat
doesn't happen until after the skill has finished being used.

This limitation only applies to the stat block, however — the entity's pools,
for example, do update in "real time".

```lua
print(e.hp)   -- 10
e:drainHP(5)  -- deal 5 damage
print(e.hp)   -- 5

print(e.stats.max_hp)  -- 10
-- do something that reduces the entity's max_hp
print(e.stats.max_hp)  -- still 10!
```

The attributes for stats are:

  Attribute  | Type   | Description
  -----------|--------|------------------------------------------------------
  `max_hp`     | number | The entity's maximum HP
  `max_mp`     | number | The entity's maximum MP
  `max_tech`   | number | The entity's maximum Tech
  `p_atk`      | number | The entity's physical attack power
  `p_def`      | number | The entity's physical defensive strength
  `m_atk`      | number | The entity's magical attack power
  `m_def`      | number | The entity's magical defensive strength
  `skill`      | number | The entity's accuracy modifier (increase hit chance)
  `evade`      | number | The entity's evasion modifier (decrease hit chance)
  `speed`      | number | Determines the entity's placing in the turn order.

All number are integers.

A stat block also has one function:

### `resists`

Determine the entity's resistance to the given element.

  Parameter | Type      | Description
  ----------|-----------|--------------------------------------------
  `elem`      | `element` | The element to retrieve resistance for

Returns the entity's resistance to the parameter `elem` as a percentage. For
example, an entity `e` with 20% Fire resistance would succeed the following
assertion:

```lua
assert(e.stats:resists(element.fire) == 20)
```

This may be converted for use in a modifer by the equation `mod = -resist / 100 + 1`.

## Functionality

Here we list the various things that you can do to an entity. Note that all of
these can be done to both the source and target entities.

### Pool modifiers

All pool modifier functions have the same signature: they take one number,
which is the value to modify the pool by, and don't return anything. The
following functions are available (where `amt` is a number):

  Function         | Description
  -----------------|--------------------------------------------------
  `drainHP(amt)`     | Damage the entity's health by the given amount
  `drainMP(amt)`     | Damage the entity's mana by the given amount
  `drainTech(amt)`   | Damage the entity's tech by the given amount
  `restoreHP(amt)`   | Heal the entity's health by the given amount
  `restoreMP(amt)`   | Heal the entity's mana by the given amount
  `restoreTech(amt)` | Heal the entity's tech by the given amount

Values are clamped to positive values; for example, `entity:drainHP(-10)` has
the same effect as `entity:drainHP(0)`. Moreover, floating point values are
rounded; `entity:drainHP(3.56)` has the same effect as `entity:drainHP(4.21)`.

### `getTeam`

Retrieve the team members of the entity. Takes no parameters, and returns a
list of living team members, which can be iterated over like:

```lua
local team = entity:getTeam()
for _, entity in ipairs(team) do
    -- use entity somehow
end
```

The list includes the source entity.

There is also a related function `getDeadTeam`, which functions identically
except instead of returning the living team members, it only returns the dead
ones.
