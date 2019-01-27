# Notes

## Skills

### High Level Requirements

- Can level up (coarse modifications)
    - requires some tracking of use
    - requires different instance of skill for each entity
- Attached to equipment
    - possibility of transfer of skill from item to character
- Attached to consumables
    - attached to the item itself rather than the entity?
    - possibility of leveling up non-consumable items?
- Represents "basic attacks" as well as "techniques"
    - "defend" should be a skill as well
- Modifiable on a fine level through "development grid"

### Management

- Every 'thing' keeps a track of their own skills
    - entities track and own their skills
    - equipment tracks and owns its skills
    - items track and owns their skills (?)
    - entities in turn track and own their equipment and items
- Entities provide 'SkillRef' objects on request
    - generic wrapper providing access/id to a skill
        - can skills be modified through their SkillRef?
    - refers to all types of skills transparently
        - entity skills vs. equipment skills vs. item skills
        - shouldn't make a difference to high-level code
        - should still have some flag so kind can be checked if required
    - trivially copyable, generally `const`
    - modification of skill (e.g. level up) doesn't invalidate reference
- Lifetime guarantees: SkillRef ≤ Skill ≤ Entity

### Hooks

- a hook for selection checking — "can I use the skill"?
    - mana, tech, hp, items, some combination thereof
    - should be linked to (relevant) costs without extra program effort
    - elidable
    - provided with `[const source]`
- cache source stats
- a hook for cost handling
    - mana costs, tech costs, item costs, etc.
    - basically actually do the things in the selection checking hook
        - conjoined with the selection checking hook
    - provided with `[source]`
- AOE checker; get list of actual targets based on AOE status and argument
    - form of AOE is dependent on a field in the skill itself
    - no hook here, I think; should be consistent between skills
    - (if you really need dependence here try pre-use checking)
    - following hooks are repeated for each opponent
- cache target stats
- a hook for pre-use checking — "did I hit with the skill"?
    - accuracy, paralysis, you're sleeping mate, etc.
    - extendable for AOE: hit some but not others, for example
    - provided with `[const source, const target]`
    - return `true` if usable; all hooks must succeed for a given opponent
- a hook for damage modification
    - this is where resistances are calculated
    - also allows for "double damage vs. sleeping targets"-type deals
    - return a percentage damage modifier
    - all percentages are applied sequentially (i.e. multiplied together)
    - provided with `[const source, const target]`
- a hook to actually do things
    - do damage, apply status effects, buffs/debuffs, etc.
    - provided `[source, target, mod]`
        - where `mod` is a `double` multiplier (from last hooks)
- a hook for post-use handling
    - recoil, self-inflicted effects, etc.
    - executed only once at the end
    - provided `[source]`

Pseudocode:

```lua
function skill:checkSelectable(source)
  return selectHooks(source);
end

function skill:use(source, target)
  costHooks(source)
  if source.isDead() then
    return;
  end
  for entity in aoeHandle(target) do
    hit = checkHooks(source, entity)
    if hit then
      mod = modHooks(source, entity)
      effectHooks(source, entity, asMultiplier(mod))
    end
    if source.isDead() then
      return;
    end
  end
  postHooks(source)
end
```

### Hook Representation

- Should be able to add/remove hooks as needed
    - Flexibility more useful when this is an actual game
- Inter-hook data sharing (how?) – unnecessary?
- Each hook provides just one function

## Stats

### High Level Requirements

- Different "kinds" of entities have different base stats
- Level actually makes no difference for stats in and of itself
    - it's just a marker noting how strong you are overall
    - a level up simply applies modifiers to your base stats for players
    - higher level variants of monsters just get different base stat blocks
- Status effects can apply modifiers to your base stats
    - doesn't change base stats permanantly, just their current effects
    - can be removed or applied at any time, potentially
    - shouldn't break invariants (e.g. $\texttt{HP}_{cur} \leq \texttt{HP}_{max}$)

### Implementation

- Move current stats for health, mana and tech into the entities themselves
    - too complicated dealing with them inside base stats, imo
    - **TODO** need some way to keep invariants in line
- Move level/xp req as well into the entities
- Similar to skills, list of modifiers
    - each modifier only affects one stat
    - can be either additive or multiplicative
    - sum multiplicative for a tier first before application
    - **TODO** applied purely in order of application? or add/mult first?
        - level up mods first, purely in order of application
        - equipment mods second, additive then multiplicative
        - status effect last, additive then multiplicative
    - modifiers applied to hp,mp,tech should modify them appropriately too
        - applying increase grants that much of the stat
        - applying decrease or removing increase caps stat at new maximum
        - removing decrease has no effect
    - have two types:
        - temporary (status effects)
        - permanant (level ups) (allow for level-down mechanics?)
- Modifiers templated?

## Visualiser

### Process flow

```
initialize BattleSystem
animate battle intro scene
while battle ongoing {
  get action
  display animations and messages for action
  while need user input {
    display idle/queued animations
    process user input
  }
}
animate battle outro scene
```

### Needs

- The action performed (Defend, Item (what?), Skill (what? who?), etc.)
    - Display appropriate messages
    - Display appropriate animations
    - User Input is an action here
- Need to be able to get the results of a performed action
    - Changes in stats (from actions) (from end-of-turn effects)
    - New/removed status effects (local and global)
    - New enemies
    - If multiple things happen, need one for each thing
- Access to the enemy teams
    - Current status (HP/MP/Tech gauges)
    - Their sprites (string identifiers?)

### Implementation
