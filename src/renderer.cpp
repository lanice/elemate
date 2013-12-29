#include "renderer.h"

#include <glow/global.h>

#include "world.h"
#include "terrain/terrain.h"
#include "particledrawable.h"

Renderer::Renderer(const World & world)
: m_world(world)
{

}

void Renderer::operator()(const glowutils::Camera & camera)
{
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);


    m_world.terrain->draw(camera);

    ParticleDrawable::drawParticles(camera);
}

void Renderer::resize(int, int) {}
