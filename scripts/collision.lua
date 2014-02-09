
-- check here if we want to handle the collision of two groups
function collisionCheckRelevance(group1id, group2id)
    local element1 = psa_elementAtId(group1id)
    local element2 = psa_elementAtId(group2id)
    if element1 == element2 then
        return false
    end
    
    -- currently we will only do something if lava and water collide
    if (element1 == "water" and element2 == "lava") or (element1 == "lava" and element2 == "water") then
        return true
    end
    
    return false
end

function elementReaction(element1, element2, count1, count2)
    -- for water + lava:
    return "bedrock"
end    
