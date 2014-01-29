-- Lua script for processing incoming events from Manipulator (manipulator.lua)

id = psa_numParticleGroups()

function glfwMouseButtonRight_press()
    eleType = "water";
    id = psa_createParticleGroup(eleType)
    io.write("Created ParticleGroup '", eleType, "' at id ", id, "\n")
end

function glfwMouseButtonLeft_press()
    if id == -1 then io.write("No ParticleGroup created.\n") return end
    io.write("Emit '", psa_elementAtId(id), "' particles.\n")
    psa_emit(id, 1000, hand_posX(), hand_posY(), hand_posZ(), 0, 1, 0)
end

function glfwMouseButtonLeft_release()
    if id == -1 then return end
    psa_stopEmit(id)
end

function glfwKeyPeriod_press()
        io.write("Clear ParticleGroups.\n")
        psa_clearParticleGroups()
        id = psa_numParticleGroups()
end

function glfwKeyTab_press()
    id = psa_nextParticleGroup(id)
    io.write("Selected ParticleGroup ", id, "\n")
end
