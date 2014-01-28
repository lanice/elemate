-- Lua script for processing incoming events from Manipulator (manipulator.lua)

numParticleGroups = psa_numParticleGroups()
index = numParticleGroups

function glfwMouseButtonRight_press()
    eleType = "water";
    index = psa_createParticleGroup(eleType)
    io.write("Created ParticleGroup '", eleType, "' at index ", index, "\n")
end

function glfwMouseButtonLeft_press()
    if index == -1 then io.write("No ParticleGroup created.\n") return end
    io.write("Emit '", psa_elementAtIndex(index), "' particles.\n")
    psa_emit(index, 1000, hand_posX(), hand_posY(), hand_posZ(), 0, 1, 0)
end

function glfwMouseButtonLeft_release()
    if index == -1 then return end
    psa_stopEmit(index)
end
