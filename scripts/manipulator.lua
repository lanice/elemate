-- Lua script for processing incoming events from Manipulator (manipulator.lua)

-- Load GLFW constants
dofile "scripts/glfw.lua"

local particleGroupId = psa_numParticleGroups()
local elementTable = {"water", "sand", "lava", "stone"}
local elements = {}

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

function handleMouseButtonEvent( button, action )
    if button == GLFW_MOUSE_BUTTON_LEFT and action == GLFW_PRESS then
        if particleGroupId == -1 then
            io.write("No ParticleGroup created.\n")
        else
            io.write("Emit '", psa_elementAtId(particleGroupId), "' particles.\n")
            psa_emit(particleGroupId, 100, hand_posX(), hand_posY(), hand_posZ(), 0, 1, 0)
        end
    end
    if button == GLFW_MOUSE_BUTTON_LEFT and action == GLFW_RELEASE then
        if particleGroupId ~= -1 then
            psa_stopEmit(particleGroupId)
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
