#pragma once

namespace glowutils {
    class Camera;
}
class World;

class Renderer
{
public:
    Renderer(const World & world);

    void operator()(const glowutils::Camera & camera);

protected:
    const World & m_world;

public:
    Renderer() = delete;
    Renderer(Renderer&) = delete;
    void operator=(Renderer&) = delete;
};
