-- Lua script for processing incoming events from Manipulator (manipulator.lua)

index = psa_numParticleGroups()

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

function glfwKeyPeriod_press()
    for i=0,psa_numParticleGroups() do
        io.write("Will remove ParticleGroup at index ", i, "\n")
        psa_removeParticleGroup(i)
        io.write("Removed ParticleGroup at index ", i, "\n")
    end
end
