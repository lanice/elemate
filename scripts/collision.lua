

local group1id
local group2id
local element1
local element2
local collisionLlf
local collisionUrb

function boundingBoxCollision(_group1id, _group2id, intersectBoxLlf, intersectBoxUrb)
    group1id = _group1id
    group2id = _group2id

    element1 = psa_elementAtId(group1id)
    element2 = psa_elementAtId(group2id)

    if element1 == element2 then
        return
    end
    
    if (element1 == "water" and element2 == "lava") or (element1 == "lava" and element2 == "water") then
        pc_checkCollidedParticles(group1id, group2id, intersectBoxLlf, intersectBoxUrb)
    end
end

function particleCollision(_collisionLlf, _collisionUrb)
    collisionLlf = _collisionLlf
    collisionUrb = _collisionUrb
    
    if (element1 == "water" and element2 == "lava") then 
        collisionWaterLava(group1id, group2id);
        return
    end
    if (element1 == "lava" and element2 == "water") then 
        collisionWaterLava(group2id, group1id);
        return
    end
end    

function collisionWaterLava(waterGroup, lavaGroup)
    -- let the worker forget what happened before
    pc_forgetOldParticles()
    
    enlargeBox(collisionLlf, collisionUrb, 0.2)    
    -- let it remember our deleted lava particles
    pc_releaseRememberParticles(lavaGroup, collisionLlf, collisionUrb)
    -- forget about water - it should become steam later
    pc_releaseForgetParticles(waterGroup, collisionLlf, collisionUrb)
    -- and later.. check that ratio between the two particles types, the release functions return a number of particles
    -- now create bedrock where lava was removed
    pc_createFromRemembered("bedrock")
    terrain_setInteractElement("bedrock");
    terrain_dropElement(collisionLlf[1], collisionLlf[3], 0.2);
end

function enlargeBox(llf, urb, delta)
    llf[1] = llf[1] - delta
    llf[2] = llf[2] - delta
    llf[3] = llf[3] - delta
    urb[1] = urb[1] + delta
    urb[2] = urb[2] + delta
    urb[3] = urb[3] + delta
end
