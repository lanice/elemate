-- Lua script for the properties of water particles (water.lua)


io.write("stone.lua\n")
-- immutable properties
function setImmutableProperties( index )
    maxMotionDistance = 0.06
    gridSize = 0.64
    restOffset = 0.004
    contactOffset = 0.008
    restParticleDistance = 0.1

    psa_setImmutableProperties(index, maxMotionDistance, gridSize, restOffset, contactOffset, restParticleDistance)
end

-- mutable properties
function setMutableProperties( index )
    restitution = 0.5
    dynamicFriction = 1.0
    staticFriction = 1.0
    damping = 1.0
    externalAcceleration = {0, 0, 0}
    particleMass = 2.0
    viscosity = 0.1
    stiffness = 8.134

    psa_setMutableProperties(index, restitution, dynamicFriction, staticFriction, damping, externalAcceleration, particleMass, viscosity, stiffness)
end

function setTemperature( index )
    psa_setTemperature(index, 10.0)
end
