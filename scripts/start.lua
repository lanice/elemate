-- Lua script for tweaking terrain / setting emitters at startup (start.lua)


local id = psa_createParticleGroup("water", 10000)

psa_emit(id, 500, 105.446, 7.953, 95.338, 1, 0, 0)
