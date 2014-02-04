-- Lua script for the properties of water particles (water.lua)

io.write("lava.lua\n")
-- immutable properties
function setImmutableProperties( index )
    maxMotionDistance = 0.06
    gridSize = 0.64
    restOffset = 0.009
    contactOffset = 0.008
    restParticleDistance = 0.09

    psa_setImmutableProperties(index, maxMotionDistance, gridSize, restOffset, contactOffset, restParticleDistance)
end

-- mutable properties
function setMutableProperties( index )
    restitution = 0.3
    dynamicFriction = 0.05
    staticFriction = 0.1
    damping = 1.0
    particleMass = 0.1
    viscosity = 0.5
    stiffness = 8.134

    psa_setMutableProperties(index, restitution, dynamicFriction, staticFriction, damping, particleMass, viscosity, stiffness)
end
