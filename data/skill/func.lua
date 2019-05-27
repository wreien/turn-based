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
    local random_var = math.random(100)

    local success = (hit_chance >= random_var)
    if not success then
        log(message.miss(target))
    end
    return success
end
