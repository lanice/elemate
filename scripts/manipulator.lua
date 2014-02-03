-- Lua script for processing incoming events from Manipulator (manipulator.lua)

-- Load GLFW constants
dofile "scripts/glfw.lua"

local particleGroupId = psa_numParticleGroups()
local elementTable = {"water", "sand", "lava", "stone"}
local elements = {}

local isEmitting = false
local emitParameters = {}

local MIN_RESTPARTICLEDISTANCE = 0.05
local MAX_RESTPARTICLEDISTANCE = 10.0

local function createParticleGroup( eleType )
    local id = psa_createParticleGroup(eleType)
    io.write("Created ParticleGroup '", eleType, "' at id ", id, "\n")
    return id
end

local function selectElement( eleType )
    if elements[eleType] ~= nil then
        return elements[eleType]
    else
        local id = createParticleGroup(eleType)
        elements[eleType] = id
        return id
    end
end

local function emit( particleGroupId, rate, posX, posY, posZ, dirX, dirY, dirZ )
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
    emitParameters[3] = posX
    emitParameters[4] = posY
    emitParameters[5] = posZ

    if (isEmitting == true) then
        psa_emit(emitParameters[1], emitParameters[2], emitParameters[3], emitParameters[4], emitParameters[5], emitParameters[6], emitParameters[7], emitParameters[8])
    end
end

function handleMouseButtonEvent( button, action )
    if button == GLFW_MOUSE_BUTTON_LEFT and action == GLFW_PRESS then
        if particleGroupId == -1 then
            io.write("No ParticleGroup created.\n")
        else
            io.write("Emit '", psa_elementAtId(particleGroupId), "' particles.\n")
            emit(particleGroupId, 500, hand_posX(), hand_posY(), hand_posZ(), 0, 1, 0)
            isEmitting = true
        end
    end
    if button == GLFW_MOUSE_BUTTON_LEFT and action == GLFW_RELEASE then
        if particleGroupId ~= -1 then
            psa_stopEmit(particleGroupId)
            isEmitting = false
        end
    end
end

function handleScrollEvent( yoffset )
    if particleGroupId ~= -1 then
        local dist = psa_restParticleDistance(particleGroupId)+(0.1*yoffset)
        if dist > MIN_RESTPARTICLEDISTANCE and dist < MAX_RESTPARTICLEDISTANCE then
            psa_setRestParticleDistance(particleGroupId, dist)
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
            io.write("Clear ParticleGroups.\n")
            elements = {}
            psa_clearParticleGroups()
            particleGroupId = psa_numParticleGroups()

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
        end

    elseif action == GLFW_RELEASE then

        if key == GLFW_KEY_LEFT_ALT then
            manipulator_setGrabbedTerrain(false)
        end
    end
end
