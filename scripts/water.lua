-- Lua script for the properties of water particles (water.lua)

-- mutable properties
restitution = 0.5
dynamicFriction = 0.05
staticFriction = 0.0
damping = 0.0
particleMass = 0.001
viscosity = 0.5
stiffness = 8.134


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
