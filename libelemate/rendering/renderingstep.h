#pragma once

class CameraEx;

/** interface for rendering steps */
class RenderingStep
{
public:
    virtual ~RenderingStep();

    virtual void draw(const CameraEx & camera) = 0;
    virtual void resize(int width, int height);
};
