-- Lua script for processing incoming events from Manipulator (manipulator.lua)

index = -1

function glfwMouseButtonRight_press()
    index = psa_createParticleGroup("water")
    io.write(index, "\n")
end

function glfwMouseButtonLeft_press()
    io.write("Emitting.\n")
    psa_emit(index, 1000, 0, 0, 0, 0, 1, 0)
end

function glfwMouseButtonLeft_release()
    psa_stopEmit(index)
    io.write("Emitting stopped.\n")
end
