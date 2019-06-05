if skill == nil then skill = {} end

-- Skill helper functions

-- determine if a skill hit the target
-- params:
--   accuracy = percentage hit chance
--   source = user of the skill
--   target = who the skill is aimed at
-- returns:
--   true if hit, false otherwise
function skill.did_hit(accuracy, source, target)
    local hit_chance = accuracy + source:stats().skill - target:stats().evade
    local random_var = random(100)

    local success = (hit_chance >= random_var)
    if not success then
        log(message.miss(target))
    end
    return success
end

-- calculate raw damage for a skill
-- params:
--   skill = the skill being used
--   source = user of the skill
--   target = who the skill is aimed at
function skill.raw_damage(skill, source, target)
    local attack
    local defense

    if skill.method == method.physical then
        attack = source:stats().p_atk
        defense = target:stats().p_def
    elseif skill.method == method.magical then
        attack = source:stats().m_atk
        defense = target:stats().m_def
    else
        error("raw_damage: skill.method must be physical or magical")
    end

    local variance = randf(0.8, 1.2)
    local raw = variance * (skill.power / 100) * (4 * attack - 2 * defense)
    return math.max(raw, 0)  -- ensure positive damage at this stage
end
