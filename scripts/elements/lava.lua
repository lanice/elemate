-- Lua script for the properties of water particles (water.lua)

-- immutable properties
function setImmutableProperties( index )
    maxMotionDistance = 0.04
    gridSize = 0.4
    restOffset = 0.05
    contactOffset = 0.07
    restParticleDistance = 0.07

    psa_setImmutableProperties(index, maxMotionDistance, gridSize, restOffset, contactOffset, restParticleDistance)
end

-- mutable properties
function setMutableProperties( index )
    restitution = 0.6
    dynamicFriction = 0.3
    staticFriction = 0.4
    damping = 0.5
    externalAcceleration = {0, 0, 0}
    particleMass = 1.0
    viscosity = 100.0
    stiffness = 8.134

    psa_setMutableProperties(index, restitution, dynamicFriction, staticFriction, damping, externalAcceleration, particleMass, viscosity, stiffness)
end

function setTemperature( index )
    psa_setTemperature(index, 1000.0)
end
