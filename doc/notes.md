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

### Implementation

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
- Skills formed from two components: base stats and modifiers
    - each modifier affects just one aspect of the skill
    - the list of modifiers is applied in order
        - entity first, then equipment
        - additive first, then multiplicative

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
- Access to the enemy teams
    - Current status (HP/MP/Tech gauges)
    - Their sprites (string identifiers?)
