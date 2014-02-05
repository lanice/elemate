-- Lua script for the properties of water particles (water.lua)

-- immutable properties
function setImmutableProperties( index )
    maxMotionDistance = 0.055
    gridSize = 0.4
    restOffset = 0.05
    contactOffset = 0.07
    restParticleDistance = 0.07

    psa_setImmutableProperties(index, maxMotionDistance, gridSize, restOffset, contactOffset, restParticleDistance)
end

-- mutable properties
function setMutableProperties( index )
    restitution = 0.3
    dynamicFriction = 0.05
    staticFriction = 0.1
    damping = 0.1
    particleMass = 0.1
    viscosity = 35.0
    stiffness = 8.134

    psa_setMutableProperties(index, restitution, dynamicFriction, staticFriction, damping, particleMass, viscosity, stiffness)
end
