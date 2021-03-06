-- Lua script for processing incoming events from Manipulator (manipulator.lua)

-- Load GLFW constants
dofile "scripts/glfw.lua"

local elementTable = {"water", "sand", "lava", "bedrock", "steam"}
local elements = {}

local isEmitting = false
local emitParameters = {}


local function createParticleGroup( emittingGroup, eleType , maxParticles)
    if maxParticles == nil then
        maxParticles = 10000
    end
    local id = psa_createParticleGroup( emittingGroup, eleType, maxParticles)
    io.write("Created ParticleGroup '", eleType, "' at id ", id, "\n")
    return id
end

local function selectElement( eleType )
    if elements[eleType] ~= nil then
        return elements[eleType]
    else
        local id = createParticleGroup(true, eleType)
        elements[eleType] = id
        return id
    end
end

local particleGroupId = selectElement(elementTable[1])
local emitId = particleGroupId

local function activeElement( eleType )
    if elements[eleType] ~= nil and particleGroupId ~= nil then
        return elements[eleType] == particleGroupId
    else
        return false
    end
end

local function spawnSource(posX, posY, posZ)
    Id = createParticleGroup(true, psa_elementAtId(particleGroupId), 1000)
    psa_emit(Id, 200, posX, posY, posZ, 0, 1, 0)
end

local function emit( particleGroupId, rate, posX, posY, posZ, dirX, dirY, dirZ )
    if activeElement("water") and posY > achievement_getProperty("maxWaterFallingHeight") then
        achievement_setProperty("maxWaterFallingHeight", posY)
    end
    emitParameters[1] = particleGroupId
    emitParameters[2] = rate
    emitParameters[3] = posX
    emitParameters[4] = posY
    emitParameters[5] = posZ
    emitParameters[6] = dirX
    emitParameters[7] = dirY
    emitParameters[8] = dirZ

    psa_emit(particleGroupId, rate, posX, posY, posZ, dirX, dirY, dirZ)
end

function updateHandPosition( posX, posY, posZ )
    local posx = posX
    local posy = posY
    local posz = posZ
    
    local ydiff = posy - terrain_terrainHeightAt(posx, posz)
    if ydiff > achievement_getProperty("maxHandY") then
        achievement_setProperty("maxHandY", ydiff);
    end
    emitParameters[3] = posx
    emitParameters[4] = posy
    emitParameters[5] = posz

    if (isEmitting == true) then
        emit(emitParameters[1], emitParameters[2], emitParameters[3], emitParameters[4], emitParameters[5], emitParameters[6], emitParameters[7], emitParameters[8])
    end
end

function handleMouseButtonEvent( button, action )
    if button == GLFW_MOUSE_BUTTON_LEFT and action == GLFW_PRESS then
        if particleGroupId == -1 then
            io.write("No ParticleGroup created.\n")
        else
            io.write("Emit '", psa_elementAtId(particleGroupId), "' particles.\n")
            emit(particleGroupId, 400, hand_posX(), hand_posY(), hand_posZ(), 0, -1, 0)
            isEmitting = true
        end
        emitId = particleGroupId
    end
    if button == GLFW_MOUSE_BUTTON_LEFT and action == GLFW_RELEASE then
        if emitId ~= -1 then
            psa_stopEmit(emitId)
            isEmitting = false
        end
    end
    if button == GLFW_MOUSE_BUTTON_MIDDLE and action == GLFW_RELEASE then
        spawnSource(hand_posX(), hand_posY(), hand_posZ())
    end
end

function handleScrollEvent( yoffset )
    if particleGroupId ~= -1 then
        local dist = psa_restParticleDistance(particleGroupId)+(0.01*yoffset)
        if dist > 0.01 and dist < 5.0 then
            psa_setRestParticleDistance(particleGroupId, dist)
            hud_debugText(dist)
        end
    end
end

function handleKeyEvent( inputKey, action )
    local key = inputKey
    local posX = hand_posX()
    local posZ = hand_posZ()

    if action == GLFW_PRESS then

        if key == GLFW_KEY_F then
            terrain_gatherElement(posX, posZ, 0.1)
            terrain_heightGrab(posX, posZ)

        elseif key == GLFW_KEY_LEFT_ALT then
            manipulator_setGrabbedTerrain(true)
            terrain_heightGrab(posX, posZ)

        elseif key == GLFW_KEY_R then
            terrain_dropElement(posX, posZ, 0.1)
            terrain_heightGrab(posX, posZ)

        elseif key == GLFW_KEY_P then
            world_togglePause()

        elseif key == GLFW_KEY_PERIOD then
            if (isEmitting) then
                return
            end
            io.write("Clear ParticleGroups.\n")
            elements = {}
            local element = psa_elementAtId(particleGroupId)
            psa_clearParticleGroups()
            particleGroupId = selectElement(element)

        elseif key == GLFW_KEY_TAB then
            local hudPosition = hud_activeElement()
            if hudPosition == 0 then hudPosition = 3
            else hudPosition = hudPosition-1 end
            hud_setActiveElement(hudPosition)
            particleGroupId = selectElement(elementTable[4 - hudPosition])

        elseif key == GLFW_KEY_1 then
            hud_setActiveElement(3)
            particleGroupId = selectElement(elementTable[1])

        elseif key == GLFW_KEY_2 then
            hud_setActiveElement(2)
            particleGroupId = selectElement(elementTable[2])

        elseif key == GLFW_KEY_3 then
            hud_setActiveElement(1)
            particleGroupId = selectElement(elementTable[3])

        elseif key == GLFW_KEY_4 then
            hud_setActiveElement(0)
            particleGroupId = selectElement(elementTable[4])

        elseif key == GLFW_KEY_F6 then
            local id = particleGroupId
            if id ~= -1 then
                local elementScript = "scripts/elements/"..psa_elementAtId(id)..".lua"
                dofile(elementScript)
                setImmutableProperties(id)
                setMutableProperties(id)
            end
        end

    elseif action == GLFW_RELEASE then

        if key == GLFW_KEY_LEFT_ALT then
            manipulator_setGrabbedTerrain(false)
        end
    end
end
