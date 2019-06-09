# Helper Functions

These are functions that are availble for use within user-facing lua code.

## General

### `random ([m [, n]])`

When called without arguments, returns a uniformly-distributed floating-point
value between 0 and 1. When called with two integers `m` and `n`, returns a
uniformly-distributed integer in the range `[m, n]`. A call to `random(n)` is
equivalent to `random(1, n)`.

This function is identical to `math.random`, but is here for consistency.

### `randf ([m [, n]])`

Similar to `random` but for floating-point arguments. When called with two
floating-point values `m` and `n`, returns a uniformly-distributed
floating-point value in the range `[m, n)`. A call to `randf(n)` is equivalent
to `randf(0.0, n)`, and a call to `randf()` is equivalent to a call to
`randf(0.0, 1.0)`.

### `log (message)`

Logs the message `message` to the progress log for the turn. This call is only
valid when inside a context where logging makes sense, such as in a skill's
`perform` function.

Inside `perform`, there are three types of message that can be sent:

  Function                 | Description
  -------------------------|----------------------------------------------
  `message.miss(entity)`     | The skill missed the specified entity
  `message.critical(entity)` | The skill scored a critical hit on the entity
  `message.notify(string)`   | Display the given string in some manner

The `message.notify` message type can also be useful for debugging.

An example usage of this function is

```lua
log(message.notify("IMPORTANT! 5 + 3 = " .. (5 + 3)))
```

## Skills

### `skill.did_hit (s, source, target [, crit_difficulty])`

This function can be used to determine if a given skill managed to hit the
target. The parameters it takes are:

  Parameter       | Type   | Description
  ----------------|--------|---------------------------------------------------
  `s`               | skill  | The skill being used
  `source`          | entity | The entity using the skill
  `target`          | entity | The entity the skill is being used on
  `crit_difficulty` | number | How hard it is to score a critical hit; default 6

It can return one of three values:

  Value | Meaning
  ------|--------------------------------
  0     | The skill missed
  1     | The skill hit normally
  2     | The skill scored a critical hit

**TODO**: return named enumerators?

The way it works is it calculates a hit chance (a percentage) based on the
accuracy of the skill, the source's `skill` stat and the target's `evade`
stat.

If the skill hits, it then does another check, which succeeds with chance
`hit_chance / crit_difficulty`. For example, if the hit chance is 90% and
`crit_difficulty` is 6, the chance of scoring a critical hit is about 15%.

**TODO**: increase default difficulty of critting, probably?

### `skill.raw_damage (s, source, target)`

This skill calculates the raw damage of a skill, without taking into account
modifiers, though it does account for status effects and traits. It takes
three parameters:

  Parameter | Type   | Description
  ----------|--------|----------------------------------------------
  `s`         | skill  | The skill being used
  `source`    | entity | The entity using the skill
  `target`    | entity | The entity the skill is being used on

It returns a positive floating-point number representing the damage dealt.

### `skill.resistance (s, entity)`

This skill calculates the resistance modifier for an entity for the skill.
This is at this stage just accounting for elemental resistances. It takes two
parameters:

  Paremeter | Type   | Description
  ----------|--------|-----------------------------------------------
  `s`         | skill  | The skill being used
  `entity`    | entity | The entity the skill is being used on

It returns a multiplier representing the resistance. For example, a
fire-element skill being used on an entity with 20% fire resistance would
return "0.8".

### `skill.default_perform (s, source, target)`

This function implements a generic `perform` function for skills. It can be
used to stamp out basic attacking skills with no other real forethought. It
has the same signature as the `perform` function attribute required to define
a skill, so it can be used like this:

```lua
return {
    desc = "an example skill",
    -- other attributes

    perform = skill.default_perform,
}
```

The function, when called, will deal damage according to the parameters for
skills described in its [documentation](skills.md). It doesn't, however, have
any options for perk- or level-specific modifications.

Note that this function can also be used as a wrapped fallback for a skill
where appropriate.
