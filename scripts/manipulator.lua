-- Lua script for processing incoming events from Manipulator (manipulator.lua)

index = -1

function glfwMouseButtonRight_press()
    index = psa_createParticleGroup("water")
    io.write(index, "\n")
end

function glfwMouseButtonLeft_press()
    if index == -1 then
        io.write("No ParticleGroup created.\n")
        return
    end
    psa_emit(index, 1000, hand_posX(), hand_posY(), hand_posZ(), 0, 1, 0)
end

function glfwMouseButtonLeft_release()
    if index == -1 then return end
    psa_stopEmit(index)
end
