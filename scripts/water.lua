-- Lua script for the properties of water particles (water.lua)


-- immutable properties
maxMotionDistance = 0.06
gridSize = 0.64
restOffset = 0.004
contactOffset = 0.008
restParticleDistance = 0.03

-- mutable properties
restitution = 0.5
dynamicFriction = 0.05
staticFriction = 0.0
damping = 0.0
particleMass = 0.001
viscosity = 0.5
stiffness = 8.134


-- apply immutable properties
particles_setImmutableProperties(
    maxMotionDistance,
    gridSize,
    restOffset,
    contactOffset,
    restParticleDistance
    )

-- apply mutable properties
particles_setMutableProperties(
    restitution,
    dynamicFriction,
    staticFriction,
    damping,
    particleMass,
    viscosity,
    stiffness
    )
