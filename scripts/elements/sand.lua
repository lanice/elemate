-- Lua script for the properties of water particles (water.lua)


io.write("sand.lua\n")
-- immutable properties
function setImmutableProperties( index )
    maxMotionDistance = 0.06
    gridSize = 0.64
    restOffset = 0.07
    contactOffset = 0.08
    restParticleDistance = 0.1

    psa_setImmutableProperties(index, maxMotionDistance, gridSize, restOffset, contactOffset, restParticleDistance)
end

-- mutable properties
function setMutableProperties( index )
    restitution = 0.5
    dynamicFriction = 0.7
    staticFriction = 0.8
    damping = 1.0
    particleMass = 0.01
    viscosity = 0.1
    stiffness = 4.134

    psa_setMutableProperties(index, restitution, dynamicFriction, staticFriction, damping, particleMass, viscosity, stiffness)
    psa_setTemperature(index, 15.0)
end
