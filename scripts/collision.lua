

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
    collisionCenterXZ = {0.5 * (collisionUrb[1] + collisionLlf[1]), 0.5*(collisionUrb[3] + collisionLlf[3])}
    enlargeBox(collisionLlf, collisionUrb, 0.2)
    -- forget the lava particles, but remember how many we have deleted
    numLavaParticles = pc_releaseForgetParticles(lavaGroup, collisionLlf, collisionUrb)
    -- remember water to transform it into steam
    pc_releaseRememberParticles(waterGroup, collisionLlf, collisionUrb)
    -- and later.. check that ratio between the two particles types, the release functions return a number of particles
    -- this would create steam particles for water (once we have steam..):
    pc_createFromRemembered("steam")
    
    terrain_setInteractElement("bedrock")
    achievement_setProperty("bedrock", 1)
    -- assuming the collision bbox is not "too large"
    -- calculate a height delta that looks fine =)
    heightDelta = psa_restOffset(waterGroup) * numLavaParticles * terrain_sampleInterval() * 0.2
    terrain_dropElement(collisionCenterXZ[1], collisionCenterXZ[2], heightDelta)
    
    -- discard all remembered particles
    pc_forgetOldParticles()
end

function enlargeBox(llf, urb, delta)
    llf[1] = llf[1] - delta
    llf[2] = llf[2] - delta
    llf[3] = llf[3] - delta
    urb[1] = urb[1] + delta
    urb[2] = urb[2] + delta
    urb[3] = urb[3] + delta
end

function temperatureCheck(id, element, bboxCenter, numParticles)
    terrain_setInteractElement("temperature")
    local terrainTemp = terrain_heightAt(bboxCenter[1], bboxCenter[3])
    local elementTemp = psa_temperature(id)
    
    local numberTerrainParticles = 200
    local tempTarget = ((numberTerrainParticles * terrainTemp) + (numParticles * elementTemp)) / (numberTerrainParticles + numParticles)
    local minStep = 0.001
    
    local terrainDelta = tempTarget - terrainTemp
    local elementDelta = tempTarget - elementTemp
    
    if math.abs(terrainDelta) > minStep then
        terrain_changeHeight(bboxCenter[1], bboxCenter[3], terrainDelta/10.0)
    end
    
    if math.abs(elementDelta) > minStep then
        psa_setTemperature(id, psa_temperature(id)+ elementDelta/10.0)
    end
end
