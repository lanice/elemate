#include "game.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <cassert>

// OSG Classes
#include <osgViewer/Viewer>
#include <osg/Texture2D>
#include <osg/Depth>

// Own Classes
#include "world.h"
#include "screenquad.h"
#include "physicswrapper.h"
#include "objectscontainer.h"
#include "godnavigation.h"
#include "godmanipulator.h"

// Classes from CGS chair
#include "HPICGS/CyclicTime.h"


struct GameResizeCallback : public osg::GraphicsContext::ResizedCallback {
    GameResizeCallback(Game & game) : game(game) { }
    void resizedImplementation(osg::GraphicsContext* /*gc*/, int /*x*/, int /*y*/, int width, int height)
    {
        osg::Viewport * oldViewport = game.m_mainCamera->getViewport();
        assert(oldViewport);
        if (oldViewport->width() == width && oldViewport->height() == height)
            return;

        std::cerr << "Render texture resizing not implemented yet." << std::endl;
        //game.m_mainCamera->getViewport()->setViewport(0, 0, width, height);
        //for (auto & buffer : game.m_renderBuffers) {
        //    // this does not really work ?!
        //    buffer.second->setTextureSize(width, height);
        //}
    }

    Game & game;
};


Game::Game(osgViewer::Viewer& viewer) :
m_interrupted(true),
m_viewer(viewer),
m_world(std::make_shared<World>()),
m_cyclicTime(new CyclicTime(0.0L, 1.0L))
{
    // create new context traits to configure vsync etc
    osg::ref_ptr< osg::GraphicsContext::Traits > traits = new osg::GraphicsContext::Traits(*m_viewer.getCamera()->getGraphicsContext()->getTraits());

    traits->windowName = "Elemate";
    traits->vsync = true;
    traits->useCursor = false;

    // apply new settings viewer and set callbacks
    osg::ref_ptr< osg::GraphicsContext > gc = osg::GraphicsContext::createGraphicsContext(traits.get());
    gc->setResizedCallback(new GameResizeCallback(*this));
    m_viewer.getCamera()->setGraphicsContext(gc.get());

    // use modern OpenGL
    osg::State * graphicsState = m_viewer.getCamera()->getGraphicsContext()->getState();
    graphicsState->setUseModelViewAndProjectionUniforms(true);
    graphicsState->setUseVertexAttributeAliasing(true);
}

Game::~Game()
{}

void Game::start(){
    if (isRunning())
        return;

    m_world->reloadShader();

    // Add GodManipulator (event handler) to the Viewer that handles events
    // that don't belong to the navigation but to game content/logic. 
    // It is added to Viewers EventHandlerQueue to receive incoming events.
    GodManipulator * eventHandler = new GodManipulator();
    // The handler gets access to the World to process game content/logic specific events.
    eventHandler->setNavigation(m_navigation);
    eventHandler->setWorld(m_world);
    m_viewer.addEventHandler(eventHandler);

    setupNavigation();

    eventHandler->setCamera(m_viewer.getCamera());
    eventHandler->setNavigation(m_navigation.get());
    m_world->setNavigation(m_navigation.get());
    
    //The "particles" ... 

    initRendering();

    m_world->physics_wrapper->startSimulation();

    loop();
}

void Game::loop(t_longf delta){
    m_interrupted = false;

    t_longf nextTime = m_cyclicTime->getNonModf(true);
    t_longf maxTimeDiff = 0.5L;
    int skippedFrames = 1;
    int maxSkippedFrames = 5;

    while (isRunning())
    {
        // get current time
        t_longf currTime = m_cyclicTime->getNonModf(true);

        // are we too far far behind? then do drawing step now.
        if ((currTime - nextTime) > maxTimeDiff)
            nextTime = currTime;

        if (currTime >= nextTime)
        {
            nextTime += delta;

            // update physic
            if (m_world->physics_wrapper->step())
                // physx: each simulate() call must be followed by fetchResults()
                m_world->objects_container->updateAllObjects();

            // update and draw objects if we have time remaining or already too many frames skipped.
            if ((currTime < nextTime) || (skippedFrames > maxSkippedFrames))
            {
                m_world->setUniforms(currTime);
                m_viewer.frame();
                skippedFrames = 1;
            } else {
                ++skippedFrames;
            }
        } else {
            t_longf sleepTime = nextTime - currTime;

            if (sleepTime > 0)
                std::this_thread::sleep_for(std::chrono::milliseconds(int(sleepTime * 1000)));
        }
    }

    m_interrupted = true;
    m_world->physics_wrapper->stopSimulation();
}

bool Game::isRunning()const{
    return !(m_viewer.done() || m_interrupted);
}

void Game::end(){
    if (isRunning())
        m_interrupted = true;
}

void Game::initRendering()
{
    osg::Viewport * viewport = m_viewer.getCamera()->getViewport();
    assert(viewport);

    osg::ref_ptr<osg::Texture2D> colorBuffer = new osg::Texture2D;
    colorBuffer->setTextureSize(viewport->width(), viewport->height());
    colorBuffer->setInternalFormat(GL_RGBA);
    colorBuffer->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
    colorBuffer->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
    osg::ref_ptr<osg::Texture2D> depthBuffer = new osg::Texture2D;
    depthBuffer->setTextureSize(viewport->width(), viewport->height());
    depthBuffer->setSourceFormat(GL_DEPTH_COMPONENT);
    depthBuffer->setSourceType(GL_FLOAT);
    depthBuffer->setInternalFormat(GL_DEPTH_COMPONENT32F);
    depthBuffer->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
    depthBuffer->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
    depthBuffer->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    depthBuffer->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);

    m_renderBuffers.emplace("color", colorBuffer.get());
    m_renderBuffers.emplace("depth", depthBuffer.get());

    m_mainCamera = new osg::Camera();
    m_mainCamera->setReferenceFrame(osg::Camera::ReferenceFrame::RELATIVE_RF);
    m_mainCamera->setRenderOrder(osg::Camera::RenderOrder::PRE_RENDER);
    m_mainCamera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    m_mainCamera->attach(osg::Camera::COLOR_BUFFER0, colorBuffer.get());
    m_mainCamera->attach(osg::Camera::DEPTH_BUFFER, depthBuffer.get());
    m_mainCamera->setDrawBuffer(GL_FRONT);
    m_mainCamera->setReadBuffer(GL_FRONT);
    m_mainCamera->setComputeNearFarMode(osg::CullSettings::ComputeNearFarMode::DO_NOT_COMPUTE_NEAR_FAR);
    m_mainCamera->setClearDepth(1.0f);
    m_mainCamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
    m_mainCamera->getOrCreateStateSet()->setAttribute(new osg::Depth(osg::Depth::LESS, 0.0, 1.0));
    m_mainCamera->addChild(m_world->root());
    m_mainCamera->setViewport(viewport);
    m_mainCamera->setGraphicsContext(m_viewer.getCamera()->getGraphicsContext());

    m_flushCamera = new osg::Camera();
    m_flushCamera->setReferenceFrame(osg::Camera::ReferenceFrame::ABSOLUTE_RF);
    m_flushCamera->setRenderOrder(osg::Camera::RenderOrder::POST_RENDER);
    m_flushCamera->setClearColor(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
    m_flushCamera->addChild(ScreenQuad::createFlushNode(*colorBuffer.get(), *depthBuffer.get(), *m_world->programByName("flush")));
    m_flushCamera->setCullingMode(osg::CullSettings::CullingModeValues::NO_CULLING);
    m_flushCamera->setGraphicsContext(m_viewer.getCamera()->getGraphicsContext());

    osg::ref_ptr<osg::Group> cameraGroup = new osg::Group;
    cameraGroup->addChild(m_flushCamera);
    cameraGroup->addChild(m_mainCamera);

    m_viewer.setSceneData(cameraGroup.get());
}

void Game::setupNavigation(){
    m_navigation = new GodNavigation();
    m_navigation->setWorld(m_world);
    m_navigation->setHomePosition(
        osg::Vec3d(0.0, 10.0, 12.0),
        osg::Vec3d(0.0, 2.0, 0.0),
        osg::Vec3d(0.0, 1.0, 0.0));
    m_navigation->home(0.0);
    m_viewer.setCameraManipulator(m_navigation);
}
