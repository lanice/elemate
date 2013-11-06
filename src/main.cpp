#include <osgViewer/Viewer>
#include <osgViewer/View>
#include <osg/GraphicsContext>
#include <osgGA/TrackballManipulator>
#include <iostream>
#include <thread>

#include "worlddrawable.h"
#include "game.h"

int main( int argc, char** argv )
{
    osgViewer::Viewer viewer;
	Game game;
    // for easier debugging: set up in window mode
    // lol see in header osgViewer/Viewer:
    // /** depreacted, use view.apply(new osgViewer::SingleWindow(x,y,width,screenNum)). */
    // void setUpViewInWindow(int x, int y, int width, int height, unsigned int screenNum = 0);
    // but this class "SingleWindow" does not exist?!
    viewer.setUpViewInWindow(50, 50, 500, 500);

    osg::ref_ptr<osg::Geode> root = new osg::Geode();
    WorldDrawable * world = new WorldDrawable;

    root->addDrawable(world);

    
            // do the geometry stuff in the osg way
    
            /*osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
            vertices->push_back(osg::Vec3(-1.0f, 0.0f, -1.0f));
            vertices->push_back(osg::Vec3(1.0f, 0.0f, -1.0f));
            vertices->push_back(osg::Vec3(1.0f, 0.0f, 1.0f));
            vertices->push_back(osg::Vec3(-1.0f, 0.0f, 1.0f));

            osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
            normals->push_back(osg::Vec3(0.0f, 1.0f, 0.0f));

            osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
            colors->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));
            colors->push_back(osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f));
            colors->push_back(osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f));
            colors->push_back(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));

            osg::ref_ptr<osg::Geometry> quad = new osg::Geometry;
            quad->setVertexArray(vertices.get());
            quad->setNormalArray(normals.get());
            quad->setNormalBinding(osg::Geometry::BIND_OVERALL);
            quad->setColorArray(colors.get());
            quad->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

            quad->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, 4));

            osg::ref_ptr<osg::Geode> root = new osg::Geode;
            root->addDrawable(quad.get());*/

	viewer.setSceneData(root.get());

    osgGA::TrackballManipulator * navigation = new osgGA::TrackballManipulator();
    navigation->setHomePosition(
        osg::Vec3d(0.0, 3.0, 2.0),
        osg::Vec3d(0.0, 0.0, 0.0),
        osg::Vec3d(0.0, 1.0, 0.0));
    navigation->home(0.0);
    viewer.setCameraManipulator(navigation);
	
	game.start();
	viewer.run();
	game.end();

	return 0;
}
