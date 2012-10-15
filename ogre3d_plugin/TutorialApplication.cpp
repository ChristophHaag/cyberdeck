/*
-----------------------------------------------------------------------------
Filename:    TutorialApplication.cpp
-----------------------------------------------------------------------------

This source file is part of the
   ___                 __    __ _ _    _
  /___\__ _ _ __ ___  / / /\ \ (_) | _(_)
 //  // _` | '__/ _ \ \ \/  \/ / | |/ / |
/ \_// (_| | | |  __/  \  /\  /| |   <| |
\___/ \__, |_|  \___|   \/  \/ |_|_|\_\_|
      |___/
      Tutorial Framework
      http://www.ogre3d.org/tikiwiki/
-----------------------------------------------------------------------------
*/
#include "TutorialApplication.h"

#include <OgreHardwarePixelBuffer.h>

#include <RenderSystems/GL/OgreGLTexture.h>
#include <RenderSystems/GL/OgreGLTextureManager.h>

#include "DotSceneLoader.h"

using namespace Ogre;

//-------------------------------------------------------------------------------------
Ogre3DRendererPlugin::Ogre3DRendererPlugin(Display *dpy, unsigned long screen, Window window, XVisualInfo *visualinfo, unsigned long context)
: BaseApplication(dpy, screen, window, visualinfo, context), desktopTexture(0), mRenderSystemCommandsRenderQueueListener(0)
{
}
//-------------------------------------------------------------------------------------
Ogre3DRendererPlugin::~Ogre3DRendererPlugin()
{
}

void Ogre3DRendererPlugin::setDesktopTexture(GLuint desktopTexture_) {
  this->desktopTexture = desktopTexture_;
  if(mRenderSystemCommandsRenderQueueListener) {
      mRenderSystemCommandsRenderQueueListener->desktopTexture = desktopTexture_;
  }
}
void Ogre3DRendererPlugin::render() {
//  mWindow->getViewport(0)->setClearEveryFrame(false,0);
  bool r = mRoot->renderOneFrame();
  if(!r) {
    std::cerr << "FAILED TO RENDER" << std::endl;
  }
//  material->touch();
//  material->reload();
}

//-------------------------------------------------------------------------------------
void Ogre3DRendererPlugin::createDesktopObject() {
  // Create the texture
    GLTexturePtr t = GLTextureManager::getSingleton().createManual("DynamicTexture", // name
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        Ogre::TEX_TYPE_2D,      // type
        1280, 800,         // width & height
        1, //depth
        MIP_DEFAULT,//0,                // number of mipmaps
        Ogre::PF_BYTE_RGBA,     // pixel format
        Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE,
        0);//TU_DEFAULT);

  t->_fireLoadingComplete(true);
  std::cerr << "**** DESKTOP TEXTURE: " << this->desktopTexture << std::endl;
  desktopTexture = t->getHandle();//getGLID();//texture->getHandle();
  std::cerr << "**** DESKTOP TEXTURE: " << this->desktopTexture << std::endl;

// Create a material using the texture
 /*Ogre::MaterialPtr */material = Ogre::MaterialManager::getSingleton().create(
     "DynamicTextureMaterial", // name
     Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

 material->getTechnique(0)->getPass(0)->createTextureUnitState("DynamicTexture");
 material->getTechnique(0)->getPass(0)->setSceneBlending(Ogre::SBT_REPLACE);//SBT_TRANSPARENT_ALPHA);
//return;
//  ogreHead->setMaterial(material);

 Ogre::ManualObject* manual = mSceneMgr->createManualObject("manual");
//  manual->begin("DynamicTextureMaterial", Ogre::RenderOperation::OT_TRIANGLE_STRIP, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
 manual->begin("BlankWhiteMaterial", Ogre::RenderOperation::OT_TRIANGLE_STRIP, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

 manual->textureCoord(0, 0);
 manual->position(-40.0, -40.0, 0.0);  // start position
 manual->textureCoord(1, 0);
 manual->position( 40.0, -40.0, 0.0);  // draw first line
 manual->textureCoord(0, 1);
 manual->position(-40.0, 40.0, 0.0);
 manual->textureCoord(1, 1);
 manual->position(40.0,  40.0, 0.0);

 manual->end();
//  mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(manual);


  ManualObject *manObj; // we will use this Manual Object as a reference point for the native rendering
  manObj = mSceneMgr->createManualObject("sampleArea");

  Ogre::AxisAlignedBox b(-40, -40, -40, 40, 40, 40);
  manObj->setBoundingBox(b);

  // Attach to child of root node, better for culling (otherwise bounds are the combination of the 2)
  mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(manObj);

  String RenderSystemName = mSceneMgr->getDestinationRenderSystem()->getName();
  mRenderSystemCommandsRenderQueueListener = new OpenGLNativeRenderSystemCommandsRenderQueueListener(manObj, mCamera2, mSceneMgr);

  mSceneMgr->addRenderQueueListener(mRenderSystemCommandsRenderQueueListener);

  std::cerr << "***** DONE CONFIGURE and INIT SCENE" << std::endl;
}

void Ogre3DRendererPlugin::createScene(void)
{
  mCamera2 = mSceneMgr->createCamera("PlayerCam2");
  mCamera2->setCustomProjectionMatrix(true, mCamera->getProjectionMatrix());
  mCamera2->setCustomViewMatrix(true, mCamera->getViewMatrix());

  mWindow->removeAllViewports();
  Ogre::Viewport* vp = mWindow->addViewport(mCamera2);



  Ogre::Entity* ogreHead = mSceneMgr->createEntity("Head", "ogrehead.mesh");
  Ogre::SceneNode* headNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("HeadNode");
//  headNode->attachObject(ogreHead);


  // Water
  // Define a plane mesh that will be used for the ocean surface
  Ogre::Plane oceanSurface;
  oceanSurface.normal = Ogre::Vector3::UNIT_Y;
  oceanSurface.d = 20;
  Ogre::MeshManager::getSingleton().createPlane("OceanSurface",
          Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
          oceanSurface,
          5000, 5000, 50, 50, true, 1, 1, 1, Ogre::Vector3::UNIT_Z);

  Entity *pWaterEntity = mSceneMgr->createEntity( "OceanSurface", "OceanSurface" );
  mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pWaterEntity);
  pWaterEntity->setMaterialName("Ocean2_HLSL_GLSL");
//  pWaterEntity->setMaterialName("OceanHLSL_GLSL");

  Ogre::Light* light = mSceneMgr->createLight( "MainLight" );
  light->setPosition(20, 80, 50);
  light->setPosition(0, 0, 50);

  Ogre::DotSceneLoader l;
  l.parseDotScene("first_terrain.scene",  Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, mSceneMgr);
//  mSceneMgr->showBoundingBoxes(true);

  mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8);
//  mSceneMgr->setSkyBox(true, "Examples/SpaceSkyBox");


  createDesktopObject();
}

//   void Ogre3DRendererPlugin::destroyScene()
//   {
//     if (mRenderSystemCommandsRenderQueueListener)
//     {
//       mSceneMgr->removeRenderQueueListener(
//           mRenderSystemCommandsRenderQueueListener);
//
//       delete mRenderSystemCommandsRenderQueueListener;
//       mRenderSystemCommandsRenderQueueListener = NULL;
//     }
//OGRE_DELETE mTerrainGroup;
//OGRE_DELETE mTerrainGlobals;
//   }





//-------------------------------------------------------------------------------------
static void getTerrainImage(bool flipX, bool flipY, Ogre::Image& img)
{
  img.load("terrain.png", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
  if (flipX)
      img.flipAroundY();
  if (flipY)
      img.flipAroundX();
}
//-------------------------------------------------------------------------------------
void
Ogre3DRendererPlugin::defineTerrain(long x, long y)
{
  Ogre::String filename = mTerrainGroup->generateFilename(x, y);
  if (Ogre::ResourceGroupManager::getSingleton().resourceExists(mTerrainGroup->getResourceGroup(), filename))
  {
      mTerrainGroup->defineTerrain(x, y);
  }
  else
  {
      Ogre::Image img;
      getTerrainImage(x % 2 != 0, y % 2 != 0, img);
      mTerrainGroup->defineTerrain(x, y, &img);
      mTerrainsImported = true;
  }
}
//-------------------------------------------------------------------------------------
void Ogre3DRendererPlugin::initBlendMaps(Ogre::Terrain* terrain)
{
  Ogre::TerrainLayerBlendMap* blendMap0 = terrain->getLayerBlendMap(1);
  Ogre::TerrainLayerBlendMap* blendMap1 = terrain->getLayerBlendMap(2);
  Ogre::Real minHeight0 = 70;
  Ogre::Real fadeDist0 = 40;
  Ogre::Real minHeight1 = 70;
  Ogre::Real fadeDist1 = 15;
  float* pBlend0 = blendMap0->getBlendPointer();
  float* pBlend1 = blendMap1->getBlendPointer();
  for (Ogre::uint16 y = 0; y < terrain->getLayerBlendMapSize(); ++y)
  {
      for (Ogre::uint16 x = 0; x < terrain->getLayerBlendMapSize(); ++x)
      {
          Ogre::Real tx, ty;

          blendMap0->convertImageToTerrainSpace(x, y, &tx, &ty);
          Ogre::Real height = terrain->getHeightAtTerrainPosition(tx, ty);
          Ogre::Real val = (height - minHeight0) / fadeDist0;
          val = Ogre::Math::Clamp(val, (Ogre::Real)0, (Ogre::Real)1);
          *pBlend0++ = val;

          val = (height - minHeight1) / fadeDist1;
          val = Ogre::Math::Clamp(val, (Ogre::Real)0, (Ogre::Real)1);
          *pBlend1++ = val;
      }
  }
  blendMap0->dirty();
  blendMap1->dirty();
  blendMap0->update();
  blendMap1->update();
}
//-------------------------------------------------------------------------------------
void Ogre3DRendererPlugin::configureTerrainDefaults(Ogre::Light* light)
{
  // Configure global
  mTerrainGlobals->setMaxPixelError(8);
  // testing composite map
  mTerrainGlobals->setCompositeMapDistance(3000);

  // Important to set these so that the terrain knows what to use for derived (non-realtime) data
  mTerrainGlobals->setLightMapDirection(light->getDerivedDirection());
  mTerrainGlobals->setCompositeMapAmbient(mSceneMgr->getAmbientLight());
  mTerrainGlobals->setCompositeMapDiffuse(light->getDiffuseColour());

  // Configure default import settings for if we use imported image
  Ogre::Terrain::ImportData& defaultimp = mTerrainGroup->getDefaultImportSettings();
  defaultimp.terrainSize = 513;
  defaultimp.worldSize = 12000.0f;
  defaultimp.inputScale = 600;
  defaultimp.minBatchSize = 33;
  defaultimp.maxBatchSize = 65;
  // textures
  defaultimp.layerList.resize(3);
  defaultimp.layerList[0].worldSize = 100;
  defaultimp.layerList[0].textureNames.push_back("dirt_grayrocky_diffusespecular.dds");
  defaultimp.layerList[0].textureNames.push_back("dirt_grayrocky_normalheight.dds");
  defaultimp.layerList[1].worldSize = 30;
  defaultimp.layerList[1].textureNames.push_back("grass_green-01_diffusespecular.dds");
  defaultimp.layerList[1].textureNames.push_back("grass_green-01_normalheight.dds");
  defaultimp.layerList[2].worldSize = 200;
  defaultimp.layerList[2].textureNames.push_back("growth_weirdfungus-03_diffusespecular.dds");
  defaultimp.layerList[2].textureNames.push_back("growth_weirdfungus-03_normalheight.dds");
}



bool Ogre3DRendererPlugin::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    bool ret = BaseApplication::frameRenderingQueued(evt);

    if (!mTerrainGroup->isDerivedDataUpdateInProgress()) {
        if (mTerrainsImported)
        {
            mTerrainGroup->saveAllTerrains(true);
            mTerrainsImported = false;
        }
    }

    return ret;
}
