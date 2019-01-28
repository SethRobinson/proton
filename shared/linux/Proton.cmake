string(COMPARE EQUAL "${PROJECT_NAME}" "Project" PROJECT_NOT_SET)
if(${PROJECT_NOT_SET})
message(FATAL_ERROR "project() must be called before including Proton.cmake")
endif(${PROJECT_NOT_SET})

string(REPLACE "/shared/linux/Proton.cmake" "" PROTON_ROOT "${CMAKE_CURRENT_LIST_FILE}")

#for raspberry pi's gles1
if(RASPBERRYPI_GLES11)
message(STATUS "Adding Raspi dirs")
	link_directories("/opt/vc/lib")
	
	#this is for SDL_mixer
	link_directories("/usr/lib/arm-linux-gnueabihf")
	#don't know why, but raspi needs this to include egl.h without errors
	include_directories("/opt/vc/include")
	include_directories("/opt/vc/include/interface/vcos/pthreads")
	include_directories("/usr/include/interface/vmcs_host/linux")
	include_directories("/opt/vc/include/interface/vmcs_host/linux")
else(RASPBERRYPI_GLES11)
#not gles1, so let's do full on GL

add_definitions(-DC_GL_MODE)

	if(RASPBERRYPI_OPENGL)
	link_directories("/usr/local/lib")
	link_directories("/opt/vc/lib")
	endif(RASPBERRYPI_OPENGL)


endif(RASPBERRYPI_GLES11)


set(PROTON_SHARED "${PROTON_ROOT}/shared")
set(PROTON_AD "${PROTON_SHARED}/Ad")
set(PROTON_AUDIO "${PROTON_SHARED}/Audio")
set(PROTON_ENTITY "${PROTON_SHARED}/Entity")
set(PROTON_FILESYSTEM "${PROTON_SHARED}/FileSystem")
set(PROTON_GUI "${PROTON_SHARED}/GUI")
set(PROTON_GAMEPAD "${PROTON_SHARED}/Gamepad")
set(PROTON_MANAGER "${PROTON_SHARED}/Manager")
set(PROTON_MATH "${PROTON_SHARED}/Math")
set(PROTON_NETWORK "${PROTON_SHARED}/Network")
set(PROTON_RENDERER "${PROTON_SHARED}/Renderer")
set(PROTON_TESTFW "${PROTON_SHARED}/testfw")
set(PROTON_UTIL "${PROTON_SHARED}/util")
set(PROTON_ZLIB "${PROTON_UTIL}/zlib")
set(PROTON_BOOSTSIGNALS "${PROTON_UTIL}/boost/libs/signals/src")
set(PROTON_CLANMATH "${PROTON_SHARED}/ClanLib-2.0/Sources/Core/Math")

add_definitions(-DRTLINUX -DBOOST_ALL_NO_LIB -DPLATFORM_LINUX)

IF (CMAKE_BUILD_TYPE MATCHES "Debug")
add_definitions(-ggdb -D_DEBUG)
ENDIF(CMAKE_BUILD_TYPE MATCHES "Debug")

IF (CMAKE_BUILD_TYPE MATCHES "Release")
add_definitions(-O3 -DNDEBUG)
ENDIF(CMAKE_BUILD_TYPE MATCHES "Release")


include_directories("${PROTON_SHARED}")
include_directories("${PROTON_UTIL}/boost")
include_directories("${PROTON_SHARED}/ClanLib-2.0/Sources")
include_directories("/usr/local/include/SDL2")


set(PROTON_SOURCES_BASIC "${PROTON_SHARED}/BaseApp.cpp" "${PROTON_SHARED}/PlatformSetup.cpp" "${PROTON_SHARED}/linux/LinuxUtils.cpp" "${PROTON_SHARED}/SDL/SDL2Main.cpp" "${PROTON_UTIL}/VideoModeSelector.cpp" "${PROTON_UTIL}/PassThroughPointerEventHandler.cpp" "${PROTON_UTIL}/TouchDeviceEmulatorPointerEventHandler.cpp"
	"${PROTON_UTIL}/Variant.cpp" "${PROTON_SHARED}/Manager/VariantDB.cpp"
	"${PROTON_AUDIO}/AudioManager.cpp"
	"${PROTON_FILESYSTEM}/FileManager.cpp" "${PROTON_FILESYSTEM}/StreamingInstance.cpp" "${PROTON_FILESYSTEM}/StreamingInstanceFile.cpp"
	"${PROTON_GUI}/RTFont.cpp"
	"${PROTON_MANAGER}/Console.cpp" "${PROTON_MANAGER}/GameTimer.cpp" "${PROTON_MANAGER}/MessageManager.cpp" "${PROTON_MANAGER}/ResourceManager.cpp"
	"${PROTON_MATH}/rtRect.cpp" 
	"${PROTON_RENDERER}/Surface.cpp" "${PROTON_RENDERER}/SoftSurface.cpp" "${PROTON_RENDERER}/SurfaceAnim.cpp" "${PROTON_RENDERER}/RenderBatcher.cpp"
	"${PROTON_UTIL}/MathUtils.cpp" "${PROTON_UTIL}/CRandom.cpp" "${PROTON_UTIL}/MiscUtils.cpp" "${PROTON_UTIL}/ResourceUtils.cpp" "${PROTON_UTIL}/RenderUtils.cpp" "${PROTON_UTIL}/GLESUtils.cpp"
	"${PROTON_ENTITY}/Entity.cpp" "${PROTON_ENTITY}/Component.cpp"
	"${PROTON_BOOSTSIGNALS}/connection.cpp" "${PROTON_BOOSTSIGNALS}/named_slot_map.cpp" "${PROTON_BOOSTSIGNALS}/signal_base.cpp" "${PROTON_BOOSTSIGNALS}/slot.cpp" "${PROTON_BOOSTSIGNALS}/trackable.cpp"
	"${PROTON_CLANMATH}/angle.cpp" "${PROTON_CLANMATH}/mat3.cpp" "${PROTON_CLANMATH}/mat4.cpp" "${PROTON_CLANMATH}/rect.cpp" "${PROTON_CLANMATH}/vec2.cpp" "${PROTON_CLANMATH}/vec3.cpp" "${PROTON_CLANMATH}/vec4.cpp"
)


#a simplified version fit for console utilities, that don't really need GL stuff or BaseApp
set(PROTON_SOURCES_CONSOLE "${PROTON_SHARED}/PlatformSetup.cpp" "${PROTON_SHARED}/linux/LinuxUtils.cpp" 
	"${PROTON_UTIL}/Variant.cpp" "${PROTON_SHARED}/Manager/VariantDB.cpp"
	"${PROTON_FILESYSTEM}/FileManager.cpp" "${PROTON_FILESYSTEM}/StreamingInstance.cpp" "${PROTON_FILESYSTEM}/StreamingInstanceFile.cpp"
	"${PROTON_MATH}/rtRect.cpp" 
	"${PROTON_RENDERER}/SoftSurface.cpp" 
	"${PROTON_UTIL}/MathUtils.cpp" "${PROTON_UTIL}/TextScanner.cpp" "${PROTON_UTIL}/CRandom.cpp" "${PROTON_UTIL}/MiscUtils.cpp" "${PROTON_UTIL}/ResourceUtils.cpp"
	"${PROTON_BOOSTSIGNALS}/connection.cpp" "${PROTON_BOOSTSIGNALS}/named_slot_map.cpp" "${PROTON_BOOSTSIGNALS}/signal_base.cpp" "${PROTON_BOOSTSIGNALS}/slot.cpp" "${PROTON_BOOSTSIGNALS}/trackable.cpp"
	"${PROTON_CLANMATH}/angle.cpp" "${PROTON_CLANMATH}/mat3.cpp" "${PROTON_CLANMATH}/mat4.cpp" "${PROTON_CLANMATH}/rect.cpp" "${PROTON_CLANMATH}/vec2.cpp" "${PROTON_CLANMATH}/vec3.cpp" "${PROTON_CLANMATH}/vec4.cpp"
)

# Includes a specific list of Components to the project. The names of
# the Components are the base names of the files without the .cpp extension.
#
# Example:
# proton_include_components(CustomInputComponent ArcadeInputComponent)
macro(proton_include_components)
	foreach(comp ${ARGV})
		list(APPEND PROTON_SOURCES "${PROTON_ENTITY}/${comp}.cpp")
	endforeach(comp)
endmacro(proton_include_components)


# Makes the project to include all the Components.
# Additionally includes the EntityUtils helpers.
# Brings in all the dependencies as well.
macro(proton_include_all_components)
	proton_include_components(ArcadeInputComponent Button2DComponent Component CustomInputComponent DPadComponent EmitVirtualKeyComponent FilterComponent FilterInputComponent FocusInputComponent FocusRenderComponent FocusUpdateComponent HTTPComponent InputTextRenderComponent InterpolateComponent LogDisplayComponent OverlayRenderComponent ProgressBarComponent RandomAudioPlayerComponent RectRenderComponent RenderClipComponent RenderScissorComponent ScrollBarRenderComponent ScrollComponent SelectButtonWithCustomInputComponent SliderComponent SplashComponent TapSequenceDetectComponent TextBoxRenderComponent TextRenderComponent TouchDragComponent TouchHandlerComponent TouchStripComponent TrailRenderComponent TyperComponent UnderlineRenderComponent)
	list(APPEND PROTON_SOURCES "${PROTON_NETWORK}/NetHTTP.cpp" "${PROTON_NETWORK}/NetSocket.cpp" "${PROTON_NETWORK}/NetUtils.cpp" "${PROTON_UTIL}/TextScanner.cpp")
	list(APPEND PROTON_SOURCES "${PROTON_ENTITY}/EntityUtils.cpp")
endmacro(proton_include_all_components)

# Makes the project to include the sprite animation support classes
# and sets the appropriate preprocessor defines.
macro(proton_use_spriteanimation)
	add_definitions(-DRT_SPRITEANIMATION)
	proton_include_components(SpriteAnimationRenderComponent)
	list(APPEND PROTON_SOURCES "${PROTON_RENDERER}/SpriteSheetSurface.cpp" "${PROTON_RENDERER}/SpriteAnimation.cpp" "${PROTON_ENTITY}/SpriteAnimationUtils.cpp")
endmacro(proton_use_spriteanimation)

# Adds any Proton source files to the project. The paths must be
# relative to the "shared" directory in the Proton source tree.
macro(proton_add_proton_sources)
	foreach(src ${ARGV})
		list(APPEND PROTON_SOURCES "${PROTON_SHARED}/${src}")
	endforeach(src)
endmacro(proton_add_proton_sources)

macro(_proton_include_jpeg_sources)
	set(PROTON_JPG "${PROTON_SHARED}/Irrlicht/source/Irrlicht/jpeglib")
	list(APPEND PROTON_SOURCES "${PROTON_JPG}/jcapimin.c" "${PROTON_JPG}/jcapistd.c" "${PROTON_JPG}/jccoefct.c" "${PROTON_JPG}/jccolor.c" "${PROTON_JPG}/jcdctmgr.c" "${PROTON_JPG}/jchuff.c" "${PROTON_JPG}/jcinit.c" "${PROTON_JPG}/jcmainct.c" "${PROTON_JPG}/jcmarker.c" "${PROTON_JPG}/jcmaster.c" "${PROTON_JPG}/jcomapi.c" "${PROTON_JPG}/jcparam.c" "${PROTON_JPG}/jcphuff.c" "${PROTON_JPG}/jcprepct.c" "${PROTON_JPG}/jcsample.c" "${PROTON_JPG}/jctrans.c" "${PROTON_JPG}/jdapimin.c" "${PROTON_JPG}/jdapistd.c" "${PROTON_JPG}/jdatadst.c" "${PROTON_JPG}/jdatasrc.c" "${PROTON_JPG}/jdcoefct.c" "${PROTON_JPG}/jdcolor.c" "${PROTON_JPG}/jddctmgr.c" "${PROTON_JPG}/jdhuff.c" "${PROTON_JPG}/jdinput.c" "${PROTON_JPG}/jdmainct.c" "${PROTON_JPG}/jdmarker.c" "${PROTON_JPG}/jdmaster.c" "${PROTON_JPG}/jdmerge.c" "${PROTON_JPG}/jdphuff.c" "${PROTON_JPG}/jdpostct.c" "${PROTON_JPG}/jdsample.c" "${PROTON_JPG}/jdtrans.c" "${PROTON_JPG}/jerror.c" "${PROTON_JPG}/jfdctflt.c" "${PROTON_JPG}/jfdctfst.c" "${PROTON_JPG}/jfdctint.c" "${PROTON_JPG}/jidctflt.c" "${PROTON_JPG}/jidctfst.c" "${PROTON_JPG}/jidctint.c" "${PROTON_JPG}/jidctred.c" "${PROTON_JPG}/jmemmgr.c" "${PROTON_JPG}/jmemnobs.c" "${PROTON_JPG}/jquant1.c" "${PROTON_JPG}/jquant2.c" "${PROTON_JPG}/jutils.c")
endmacro(_proton_include_jpeg_sources)

macro(_proton_include_png_sources)
	set(PROTON_PNG "${PROTON_SHARED}/Irrlicht/source/Irrlicht/libpng")
	list(APPEND PROTON_SOURCES "${PROTON_PNG}/png.c" "${PROTON_PNG}/pngerror.c" "${PROTON_PNG}/pnggccrd.c" "${PROTON_PNG}/pngget.c" "${PROTON_PNG}/pngmem.c" "${PROTON_PNG}/pngpread.c" "${PROTON_PNG}/pngread.c" "${PROTON_PNG}/pngrio.c" "${PROTON_PNG}/pngrtran.c" "${PROTON_PNG}/pngrutil.c" "${PROTON_PNG}/pngset.c" "${PROTON_PNG}/pngtrans.c" "${PROTON_PNG}/pngvcrd.c" "${PROTON_PNG}/pngwio.c" "${PROTON_PNG}/pngwtran.c")
endmacro(_proton_include_png_sources)

# Enables JPEG support in the project. Defines RT_JPG_SUPPORT and includes
# the files needed to load JPEG images.
macro(proton_use_jpeg_support)
	add_definitions(-DRT_JPG_SUPPORT)
	list(APPEND PROTON_SOURCES "${PROTON_RENDERER}/JPGSurfaceLoader.cpp")
	_proton_include_jpeg_sources()
endmacro(proton_use_jpeg_support)

# Enables PNG support in the project. Defines RT_PNG_SUPPORT and includes
# the files needed to load PNG images.
macro(proton_use_png_support)
	add_definitions(-DRT_PNG_SUPPORT)
	_proton_include_png_sources()
endmacro(proton_use_png_support)

#seth removed, want to use SDL2 mixer but many make installs don't have a macro for it yet

# Enables the project to use the SDL audio system.
macro(proton_use_sdl_audio)
	add_definitions(-DRT_USE_SDL_AUDIO)
	list(APPEND PROTON_SOURCES "${PROTON_AUDIO}/AudioManagerSDL.cpp")
	set(PROTON_USE_SDL_AUDIO TRUE)
endmacro(proton_use_sdl_audio)

# Enables the project to use the linearparticles system.
macro(proton_use_linearparticles)
	set(PROTON_LINEARPARTICLE "${PROTON_RENDERER}/linearparticle/sources")
	list(APPEND PROTON_SOURCES "${PROTON_LINEARPARTICLE}/L_Defination.cpp" "${PROTON_LINEARPARTICLE}/L_DroppingEffect.cpp" "${PROTON_LINEARPARTICLE}/L_EffectEmitter.cpp" "${PROTON_LINEARPARTICLE}/L_ExplosionEffect.cpp" "${PROTON_LINEARPARTICLE}/L_MotionController.cpp" "${PROTON_LINEARPARTICLE}/L_Particle.cpp" "${PROTON_LINEARPARTICLE}/L_ParticleEffect.cpp" "${PROTON_LINEARPARTICLE}/L_ParticleMem.cpp" "${PROTON_LINEARPARTICLE}/L_ParticleSystem.cpp" "${PROTON_LINEARPARTICLE}/L_ShootingEffect.cpp" "${PROTON_LINEARPARTICLE}/L_EffectManager.cpp")
endmacro(proton_use_linearparticles)

# Enables the project to use the zip file system.
macro(proton_use_zipfilesystem)
	list(APPEND PROTON_SOURCES "${PROTON_FILESYSTEM}/FileSystem.cpp" "${PROTON_FILESYSTEM}/FileSystemZip.cpp" "${PROTON_FILESYSTEM}/StreamingInstanceZip.cpp" "${PROTON_UTIL}/unzip/unzip.c" "${PROTON_UTIL}/unzip/ioapi.c")
endmacro(proton_use_zipfilesystem)

# By default the project gets linked against a zlib shared library.
# If this macro is called the zlib sources bundled with Proton are used instead.
macro(proton_use_internal_zlib)
	list(APPEND PROTON_SOURCES "${PROTON_ZLIB}/deflate.c" "${PROTON_ZLIB}/inflate.c" "${PROTON_ZLIB}/compress.c" "${PROTON_ZLIB}/zutil.c" "${PROTON_ZLIB}/adler32.c" "${PROTON_ZLIB}/crc32.c" "${PROTON_ZLIB}/trees.c" "${PROTON_ZLIB}/inftrees.c" "${PROTON_ZLIB}/inffast.c")
	include_directories("${PROTON_ZLIB}")
	set(PROTON_USE_INTERNAL_ZLIB TRUE)
endmacro(proton_use_internal_zlib)

# Enables the project to use Irrlicht.
macro(proton_use_irrlicht)
	add_definitions(-D_IRR_STATIC_LIB_)
	include_directories("${PROTON_SHARED}/Irrlicht/include")
	set(PROTON_IRRLICHT "${PROTON_SHARED}/Irrlicht/source/Irrlicht")
	list(APPEND PROTON_SOURCES "${PROTON_SHARED}/Irrlicht/IrrlichtManager.cpp"
	"${PROTON_IRRLICHT}/CAttributes.cpp" "${PROTON_IRRLICHT}/CBoneSceneNode.cpp" "${PROTON_IRRLICHT}/CColorConverter.cpp" "${PROTON_IRRLICHT}/CDefaultSceneNodeAnimatorFactory.cpp" "${PROTON_IRRLICHT}/CDefaultSceneNodeFactory.cpp" "${PROTON_IRRLICHT}/CDepthBuffer.cpp" "${PROTON_IRRLICHT}/CDummyTransformationSceneNode.cpp" "${PROTON_IRRLICHT}/CEmptySceneNode.cpp" "${PROTON_IRRLICHT}/CFPSCounter.cpp" "${PROTON_IRRLICHT}/CGeometryCreator.cpp" "${PROTON_IRRLICHT}/CLightSceneNode.cpp" "${PROTON_IRRLICHT}/CLogger.cpp" "${PROTON_IRRLICHT}/CMemoryFile.cpp" "${PROTON_IRRLICHT}/CMeshCache.cpp" "${PROTON_IRRLICHT}/CMeshManipulator.cpp" "${PROTON_IRRLICHT}/CMeshSceneNode.cpp" "${PROTON_IRRLICHT}/COCTLoader.cpp" "${PROTON_IRRLICHT}/COctreeSceneNode.cpp" "${PROTON_IRRLICHT}/CSkinnedMesh.cpp" "${PROTON_IRRLICHT}/CTextSceneNode.cpp" "${PROTON_IRRLICHT}/CTriangleBBSelector.cpp" "${PROTON_IRRLICHT}/CTriangleSelector.cpp" "${PROTON_IRRLICHT}/COctreeTriangleSelector.cpp" "${PROTON_IRRLICHT}/CVideoModeList.cpp" "${PROTON_IRRLICHT}/CVolumeLightSceneNode.cpp" "${PROTON_IRRLICHT}/CWaterSurfaceSceneNode.cpp" "${PROTON_IRRLICHT}/Irrlicht.cpp" "${PROTON_IRRLICHT}/irrXML.cpp" "${PROTON_IRRLICHT}/os.cpp" "${PROTON_IRRLICHT}/CMetaTriangleSelector.cpp"
	"${PROTON_IRRLICHT}/CCameraSceneNode.cpp" "${PROTON_IRRLICHT}/CSceneNodeAnimatorCameraFPS.cpp"
	"${PROTON_IRRLICHT}/CIrrDeviceStub.cpp" "${PROTON_IRRLICHT}/CIrrDeviceWin32.cpp"
	"${PROTON_IRRLICHT}/CFileList.cpp" "${PROTON_IRRLICHT}/CFileSystem.cpp" "${PROTON_IRRLICHT}/CLimitReadFile.cpp" "${PROTON_IRRLICHT}/CMountPointReader.cpp" "${PROTON_IRRLICHT}/COSOperator.cpp" "${PROTON_IRRLICHT}/CPakReader.cpp" "${PROTON_IRRLICHT}/CReadFile.cpp" "${PROTON_IRRLICHT}/CWriteFile.cpp" "${PROTON_IRRLICHT}/CXMLReader.cpp" "${PROTON_IRRLICHT}/CXMLWriter.cpp" "${PROTON_IRRLICHT}/CZBuffer.cpp" "${PROTON_IRRLICHT}/CZipReader.cpp" "${PROTON_IRRLICHT}/CProtonReader.cpp"
	"${PROTON_IRRLICHT}/CImage.cpp" "${PROTON_IRRLICHT}/CImageLoaderBMP.cpp" "${PROTON_IRRLICHT}/CImageLoaderJPG.cpp" "${PROTON_IRRLICHT}/CImageLoaderPNG.cpp" "${PROTON_IRRLICHT}/CImageLoaderRGB.cpp" "${PROTON_IRRLICHT}/CImageLoaderRTTEX.cpp" "${PROTON_IRRLICHT}/CImageLoaderTGA.cpp"
	"${PROTON_IRRLICHT}/C3DSMeshFileLoader.cpp" "${PROTON_IRRLICHT}/CAnimatedMeshMD2.cpp" "${PROTON_IRRLICHT}/CAnimatedMeshMD3.cpp" "${PROTON_IRRLICHT}/CB3DMeshFileLoader.cpp" "${PROTON_IRRLICHT}/CBSPMeshFileLoader.cpp" "${PROTON_IRRLICHT}/CColladaFileLoader.cpp" "${PROTON_IRRLICHT}/CCSMLoader.cpp" "${PROTON_IRRLICHT}/CMD2MeshFileLoader.cpp" "${PROTON_IRRLICHT}/CMD3MeshFileLoader.cpp" "${PROTON_IRRLICHT}/CMS3DMeshFileLoader.cpp" "${PROTON_IRRLICHT}/CMY3DMeshFileLoader.cpp" "${PROTON_IRRLICHT}/COBJMeshFileLoader.cpp" "${PROTON_IRRLICHT}/CQ3LevelMesh.cpp" "${PROTON_IRRLICHT}/CQuake3ShaderSceneNode.cpp" "${PROTON_IRRLICHT}/CXMeshFileLoader.cpp"
	"${PROTON_IRRLICHT}/CParticleAnimatedMeshSceneNodeEmitter.cpp" "${PROTON_IRRLICHT}/CParticleAttractionAffector.cpp" "${PROTON_IRRLICHT}/CParticleBoxEmitter.cpp" "${PROTON_IRRLICHT}/CParticleCylinderEmitter.cpp" "${PROTON_IRRLICHT}/CParticleFadeOutAffector.cpp" "${PROTON_IRRLICHT}/CParticleGravityAffector.cpp" "${PROTON_IRRLICHT}/CParticleMeshEmitter.cpp" "${PROTON_IRRLICHT}/CParticlePointEmitter.cpp" "${PROTON_IRRLICHT}/CParticleRingEmitter.cpp" "${PROTON_IRRLICHT}/CParticleRotationAffector.cpp" "${PROTON_IRRLICHT}/CParticleScaleAffector.cpp" "${PROTON_IRRLICHT}/CParticleSphereEmitter.cpp" "${PROTON_IRRLICHT}/CParticleSystemSceneNode.cpp"
	"${PROTON_IRRLICHT}/CAnimatedMeshSceneNode.cpp" "${PROTON_IRRLICHT}/CBillboardSceneNode.cpp" "${PROTON_IRRLICHT}/CCubeSceneNode.cpp" "${PROTON_IRRLICHT}/CSceneCollisionManager.cpp" "${PROTON_IRRLICHT}/CSceneManager.cpp" "${PROTON_IRRLICHT}/CSceneNodeAnimatorCameraMaya.cpp" "${PROTON_IRRLICHT}/CSceneNodeAnimatorCollisionResponse.cpp" "${PROTON_IRRLICHT}/CSceneNodeAnimatorDelete.cpp" "${PROTON_IRRLICHT}/CSceneNodeAnimatorFlyCircle.cpp" "${PROTON_IRRLICHT}/CSceneNodeAnimatorFlyStraight.cpp" "${PROTON_IRRLICHT}/CSceneNodeAnimatorFollowSpline.cpp" "${PROTON_IRRLICHT}/CSceneNodeAnimatorRotation.cpp" "${PROTON_IRRLICHT}/CSceneNodeAnimatorTexture.cpp" "${PROTON_IRRLICHT}/CShadowVolumeSceneNode.cpp"
	"${PROTON_IRRLICHT}/CSkyBoxSceneNode.cpp" "${PROTON_IRRLICHT}/CSkyDomeSceneNode.cpp" "${PROTON_IRRLICHT}/CSphereSceneNode.cpp" "${PROTON_IRRLICHT}/CTerrainSceneNode.cpp" "${PROTON_IRRLICHT}/CTerrainTriangleSelector.cpp"
	"${PROTON_IRRLICHT}/COpenGLDriver.cpp" "${PROTON_IRRLICHT}/COpenGLExtensionHandler.cpp" "${PROTON_IRRLICHT}/COpenGLNormalMapRenderer.cpp" "${PROTON_IRRLICHT}/COpenGLParallaxMapRenderer.cpp" "${PROTON_IRRLICHT}/COpenGLTexture.cpp" "${PROTON_IRRLICHT}/COpenGLShaderMaterialRenderer.cpp" "${PROTON_IRRLICHT}/COpenGLSLMaterialRenderer.cpp" "${PROTON_IRRLICHT}/CNullDriver.cpp")
	_proton_include_jpeg_sources()
	_proton_include_png_sources()
endmacro(proton_use_irrlicht)

# Enables the project to use Bullet (and Irrlicht).
macro(proton_use_bullet)
	add_definitions(-DRT_IRRBULLET)
	set(PROTON_BULLET "${PROTON_SHARED}/Bullet")
	set(PROTON_IRRBULLET "${PROTON_SHARED}/Irrlicht/irrBullet")
	include_directories("${PROTON_BULLET}")
	list(APPEND PROTON_SOURCES "${PROTON_BULLET}/BulletCollision/BroadphaseCollision/btAxisSweep3.cpp" "${PROTON_BULLET}/BulletCollision/BroadphaseCollision/btBroadphaseProxy.cpp" "${PROTON_BULLET}/BulletCollision/BroadphaseCollision/btCollisionAlgorithm.cpp" "${PROTON_BULLET}/BulletCollision/BroadphaseCollision/btDbvt.cpp" "${PROTON_BULLET}/BulletCollision/BroadphaseCollision/btDbvtBroadphase.cpp" "${PROTON_BULLET}/BulletCollision/BroadphaseCollision/btDispatcher.cpp" "${PROTON_BULLET}/BulletCollision/BroadphaseCollision/btMultiSapBroadphase.cpp" "${PROTON_BULLET}/BulletCollision/BroadphaseCollision/btOverlappingPairCache.cpp" "${PROTON_BULLET}/BulletCollision/BroadphaseCollision/btQuantizedBvh.cpp" "${PROTON_BULLET}/BulletCollision/BroadphaseCollision/btSimpleBroadphase.cpp" "${PROTON_BULLET}/BulletCollision/CollisionDispatch/btActivatingCollisionAlgorithm.cpp" "${PROTON_BULLET}/BulletCollision/CollisionDispatch/btBoxBoxCollisionAlgorithm.cpp" "${PROTON_BULLET}/BulletCollision/CollisionDispatch/btBox2dBox2dCollisionAlgorithm.cpp" "${PROTON_BULLET}/BulletCollision/CollisionDispatch/btBoxBoxDetector.cpp" "${PROTON_BULLET}/BulletCollision/CollisionDispatch/btCollisionDispatcher.cpp" "${PROTON_BULLET}/BulletCollision/CollisionDispatch/btCollisionObject.cpp" "${PROTON_BULLET}/BulletCollision/CollisionDispatch/btCollisionWorld.cpp" "${PROTON_BULLET}/BulletCollision/CollisionDispatch/btCompoundCollisionAlgorithm.cpp" "${PROTON_BULLET}/BulletCollision/CollisionDispatch/btConvexConcaveCollisionAlgorithm.cpp" "${PROTON_BULLET}/BulletCollision/CollisionDispatch/btConvexConvexAlgorithm.cpp" "${PROTON_BULLET}/BulletCollision/CollisionDispatch/btConvexPlaneCollisionAlgorithm.cpp" "${PROTON_BULLET}/BulletCollision/CollisionDispatch/btConvex2dConvex2dAlgorithm.cpp" "${PROTON_BULLET}/BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.cpp" "${PROTON_BULLET}/BulletCollision/CollisionDispatch/btEmptyCollisionAlgorithm.cpp" "${PROTON_BULLET}/BulletCollision/CollisionDispatch/btGhostObject.cpp" "${PROTON_BULLET}/BulletCollision/CollisionDispatch/btInternalEdgeUtility.cpp" "${PROTON_BULLET}/BulletCollision/CollisionDispatch/btManifoldResult.cpp" "${PROTON_BULLET}/BulletCollision/CollisionDispatch/btSimulationIslandManager.cpp" "${PROTON_BULLET}/BulletCollision/CollisionDispatch/btSphereBoxCollisionAlgorithm.cpp" "${PROTON_BULLET}/BulletCollision/CollisionDispatch/btSphereSphereCollisionAlgorithm.cpp" "${PROTON_BULLET}/BulletCollision/CollisionDispatch/btSphereTriangleCollisionAlgorithm.cpp" "${PROTON_BULLET}/BulletCollision/CollisionDispatch/btUnionFind.cpp" "${PROTON_BULLET}/BulletCollision/CollisionDispatch/SphereTriangleDetector.cpp" "${PROTON_BULLET}/BulletCollision/CollisionShapes/btBoxShape.cpp" "${PROTON_BULLET}/BulletCollision/CollisionShapes/btBox2dShape.cpp" "${PROTON_BULLET}/BulletCollision/CollisionShapes/btBvhTriangleMeshShape.cpp" "${PROTON_BULLET}/BulletCollision/CollisionShapes/btCapsuleShape.cpp" "${PROTON_BULLET}/BulletCollision/CollisionShapes/btCollisionShape.cpp" "${PROTON_BULLET}/BulletCollision/CollisionShapes/btCompoundShape.cpp" "${PROTON_BULLET}/BulletCollision/CollisionShapes/btConcaveShape.cpp" "${PROTON_BULLET}/BulletCollision/CollisionShapes/btConeShape.cpp" "${PROTON_BULLET}/BulletCollision/CollisionShapes/btConvexHullShape.cpp" "${PROTON_BULLET}/BulletCollision/CollisionShapes/btConvexInternalShape.cpp" "${PROTON_BULLET}/BulletCollision/CollisionShapes/btConvexPointCloudShape.cpp" "${PROTON_BULLET}/BulletCollision/CollisionShapes/btConvexShape.cpp" "${PROTON_BULLET}/BulletCollision/CollisionShapes/btConvex2dShape.cpp" "${PROTON_BULLET}/BulletCollision/CollisionShapes/btConvexTriangleMeshShape.cpp" "${PROTON_BULLET}/BulletCollision/CollisionShapes/btCylinderShape.cpp" "${PROTON_BULLET}/BulletCollision/CollisionShapes/btEmptyShape.cpp" "${PROTON_BULLET}/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.cpp" "${PROTON_BULLET}/BulletCollision/CollisionShapes/btMinkowskiSumShape.cpp" "${PROTON_BULLET}/BulletCollision/CollisionShapes/btMultimaterialTriangleMeshShape.cpp" "${PROTON_BULLET}/BulletCollision/CollisionShapes/btMultiSphereShape.cpp" "${PROTON_BULLET}/BulletCollision/CollisionShapes/btOptimizedBvh.cpp" "${PROTON_BULLET}/BulletCollision/CollisionShapes/btPolyhedralConvexShape.cpp" "${PROTON_BULLET}/BulletCollision/CollisionShapes/btScaledBvhTriangleMeshShape.cpp" "${PROTON_BULLET}/BulletCollision/CollisionShapes/btShapeHull.cpp" "${PROTON_BULLET}/BulletCollision/CollisionShapes/btSphereShape.cpp" "${PROTON_BULLET}/BulletCollision/CollisionShapes/btStaticPlaneShape.cpp" "${PROTON_BULLET}/BulletCollision/CollisionShapes/btStridingMeshInterface.cpp" "${PROTON_BULLET}/BulletCollision/CollisionShapes/btTetrahedronShape.cpp" "${PROTON_BULLET}/BulletCollision/CollisionShapes/btTriangleBuffer.cpp" "${PROTON_BULLET}/BulletCollision/CollisionShapes/btTriangleCallback.cpp" "${PROTON_BULLET}/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.cpp" "${PROTON_BULLET}/BulletCollision/CollisionShapes/btTriangleIndexVertexMaterialArray.cpp" "${PROTON_BULLET}/BulletCollision/CollisionShapes/btTriangleMesh.cpp" "${PROTON_BULLET}/BulletCollision/CollisionShapes/btTriangleMeshShape.cpp" "${PROTON_BULLET}/BulletCollision/CollisionShapes/btUniformScalingShape.cpp" "${PROTON_BULLET}/BulletCollision/Gimpact/btContactProcessing.cpp" "${PROTON_BULLET}/BulletCollision/Gimpact/btGenericPoolAllocator.cpp" "${PROTON_BULLET}/BulletCollision/Gimpact/btGImpactBvh.cpp" "${PROTON_BULLET}/BulletCollision/Gimpact/btGImpactCollisionAlgorithm.cpp" "${PROTON_BULLET}/BulletCollision/Gimpact/btGImpactQuantizedBvh.cpp" "${PROTON_BULLET}/BulletCollision/Gimpact/btGImpactShape.cpp" "${PROTON_BULLET}/BulletCollision/Gimpact/btTriangleShapeEx.cpp" "${PROTON_BULLET}/BulletCollision/Gimpact/gim_box_set.cpp" "${PROTON_BULLET}/BulletCollision/Gimpact/gim_contact.cpp" "${PROTON_BULLET}/BulletCollision/Gimpact/gim_memory.cpp" "${PROTON_BULLET}/BulletCollision/Gimpact/gim_tri_collision.cpp" "${PROTON_BULLET}/BulletCollision/NarrowPhaseCollision/btContinuousConvexCollision.cpp" "${PROTON_BULLET}/BulletCollision/NarrowPhaseCollision/btConvexCast.cpp" "${PROTON_BULLET}/BulletCollision/NarrowPhaseCollision/btGjkConvexCast.cpp" "${PROTON_BULLET}/BulletCollision/NarrowPhaseCollision/btGjkEpa2.cpp" "${PROTON_BULLET}/BulletCollision/NarrowPhaseCollision/btGjkEpaPenetrationDepthSolver.cpp" "${PROTON_BULLET}/BulletCollision/NarrowPhaseCollision/btGjkPairDetector.cpp" "${PROTON_BULLET}/BulletCollision/NarrowPhaseCollision/btMinkowskiPenetrationDepthSolver.cpp" "${PROTON_BULLET}/BulletCollision/NarrowPhaseCollision/btPersistentManifold.cpp" "${PROTON_BULLET}/BulletCollision/NarrowPhaseCollision/btRaycastCallback.cpp" "${PROTON_BULLET}/BulletCollision/NarrowPhaseCollision/btSubSimplexConvexCast.cpp" "${PROTON_BULLET}/BulletCollision/NarrowPhaseCollision/btVoronoiSimplexSolver.cpp" "${PROTON_BULLET}/BulletDynamics/Character/btKinematicCharacterController.cpp" "${PROTON_BULLET}/BulletDynamics/ConstraintSolver/btConeTwistConstraint.cpp" "${PROTON_BULLET}/BulletDynamics/ConstraintSolver/btContactConstraint.cpp" "${PROTON_BULLET}/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.cpp" "${PROTON_BULLET}/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.cpp" "${PROTON_BULLET}/BulletDynamics/ConstraintSolver/btHinge2Constraint.cpp" "${PROTON_BULLET}/BulletDynamics/ConstraintSolver/btHingeConstraint.cpp" "${PROTON_BULLET}/BulletDynamics/ConstraintSolver/btPoint2PointConstraint.cpp" "${PROTON_BULLET}/BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.cpp" "${PROTON_BULLET}/BulletDynamics/ConstraintSolver/btSliderConstraint.cpp" "${PROTON_BULLET}/BulletDynamics/ConstraintSolver/btSolve2LinearConstraint.cpp" "${PROTON_BULLET}/BulletDynamics/ConstraintSolver/btTypedConstraint.cpp" "${PROTON_BULLET}/BulletDynamics/ConstraintSolver/btUniversalConstraint.cpp" "${PROTON_BULLET}/BulletDynamics/Dynamics/btContinuousDynamicsWorld.cpp" "${PROTON_BULLET}/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.cpp" "${PROTON_BULLET}/BulletDynamics/Dynamics/btRigidBody.cpp" "${PROTON_BULLET}/BulletDynamics/Dynamics/btSimpleDynamicsWorld.cpp" "${PROTON_BULLET}/BulletDynamics/Dynamics/Bullet-C-API.cpp" "${PROTON_BULLET}/BulletDynamics/Vehicle/btRaycastVehicle.cpp" "${PROTON_BULLET}/BulletDynamics/Vehicle/btWheelInfo.cpp" "${PROTON_BULLET}/LinearMath/btAlignedAllocator.cpp" "${PROTON_BULLET}/LinearMath/btConvexHull.cpp" "${PROTON_BULLET}/LinearMath/btGeometryUtil.cpp" "${PROTON_BULLET}/LinearMath/btQuickprof.cpp" "${PROTON_BULLET}/LinearMath/btSerializer.cpp" "${PROTON_BULLET}/BulletSoftBody/btDefaultSoftBodySolver.cpp" "${PROTON_BULLET}/BulletSoftBody/btSoftBody.cpp" "${PROTON_BULLET}/BulletSoftBody/btSoftBodyConcaveCollisionAlgorithm.cpp" "${PROTON_BULLET}/BulletSoftBody/btSoftBodyHelpers.cpp" "${PROTON_BULLET}/BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.cpp" "${PROTON_BULLET}/BulletSoftBody/btSoftRigidCollisionAlgorithm.cpp" "${PROTON_BULLET}/BulletSoftBody/btSoftRigidDynamicsWorld.cpp" "${PROTON_BULLET}/BulletSoftBody/btSoftSoftCollisionAlgorithm.cpp"
	"${PROTON_IRRBULLET}/boxshape.cpp" "${PROTON_IRRBULLET}/bulletworld.cpp" "${PROTON_IRRBULLET}/bvhtrianglemeshshape.cpp" "${PROTON_IRRBULLET}/collisioncallbackinformation.cpp" "${PROTON_IRRBULLET}/collisionobject.cpp" "${PROTON_IRRBULLET}/collisionobjectaffector.cpp" "${PROTON_IRRBULLET}/collisionobjectaffectorattract.cpp" "${PROTON_IRRBULLET}/collisionobjectaffectordelete.cpp" "${PROTON_IRRBULLET}/collisionshape.cpp" "${PROTON_IRRBULLET}/convexhullshape.cpp" "${PROTON_IRRBULLET}/gimpactmeshshape.cpp" "${PROTON_IRRBULLET}/irrbullet.cpp" "${PROTON_IRRBULLET}/irrbulletcommon.cpp" "${PROTON_IRRBULLET}/liquidbody.cpp" "${PROTON_IRRBULLET}/motionstate.cpp" "${PROTON_IRRBULLET}/physicsdebug.cpp" "${PROTON_IRRBULLET}/raycastvehicle.cpp" "${PROTON_IRRBULLET}/rigidbody.cpp" "${PROTON_IRRBULLET}/softbody.cpp" "${PROTON_IRRBULLET}/sphereshape.cpp" "${PROTON_IRRBULLET}/trianglemeshshape.cpp")
	proton_use_irrlicht()
endmacro(proton_use_bullet)

# Enables the project to use the in-app purchasing functionality.
macro(proton_use_iap)
	add_definitions(-DRT_IAP_SUPPORT)
	list(APPEND PROTON_SOURCES "${PROTON_MANAGER}/IAPManager.cpp")
endmacro(proton_use_iap)

# Enables the project to use the ad framework.
# The ad provider names (without the AdProvider prefix) can be passed as arguments.
macro(proton_use_ad_framework)
	list(APPEND PROTON_SOURCES "${PROTON_MANAGER}/AdManager.cpp")

	if(${ARGC} GREATER 0)
		list(APPEND PROTON_SOURCES "${PROTON_AD}/AdProvider.cpp")
	endif(${ARGC} GREATER 0)

	foreach(adprovider ${ARGV})
		list(APPEND PROTON_SOURCES "${PROTON_AD}/AdProvider${adprovider}.cpp")
	endforeach(adprovider)
endmacro(proton_use_ad_framework)

# Enables the project to use gamepads. The supported gamepad names can be given as arguments,
# for example: proton_include_gamepad(mypad SomeOtherPad)
macro(proton_include_gamepad)
	foreach(gamepadname ${ARGV})
		list(APPEND PROTON_SOURCES "${PROTON_GAMEPAD}/Gamepad${gamepadname}.cpp" "${PROTON_GAMEPAD}/GamepadProvider${gamepadname}.cpp")
	endforeach(gamepadname)
	
	list(APPEND PROTON_SOURCES "${PROTON_GAMEPAD}/GamepadManager.cpp" "${PROTON_GAMEPAD}/Gamepad.cpp" "${PROTON_GAMEPAD}/GamepadProvider.cpp")
endmacro(proton_include_gamepad)

# Includes the Proton testing framework to the project.
# Sets RT_TESTFW preprocessor define which can be used in the code for conditional compiling.
# If the argument "GUI" is passed to this macro then also the GUI parts
# of the testing framework are included.
macro(proton_include_testing)
	add_definitions(-DRT_TESTFW)
	list(APPEND PROTON_SOURCES "${PROTON_TESTFW}/ProtonTester.cpp")
	
	# For some reason list(FIND doesn't work in a macro so have to search by hand
	set(INCLUDE_TEST_GUI FALSE)
	foreach(arg ${ARGV})
		string(COMPARE EQUAL ${arg} GUI found)
		if(found)
			set(INCLUDE_TEST_GUI TRUE)
		endif(found)
	endforeach(arg)
	
	if(INCLUDE_TEST_GUI)
		list(APPEND PROTON_SOURCES "${PROTON_TESTFW}/ProtonTesterGUI.cpp")
	endif(INCLUDE_TEST_GUI)
endmacro(proton_include_testing)

# Sets the source files that should be compiled to the application.
# You only need to supply the source files that you have written yourself.
# All Proton related source files should be excluded - they are automatically
# included as needed.
#
# Example:
# proton_set_sources(mysource1.cpp mysource2.cpp)
function(proton_set_sources)
	list(APPEND PROTON_SOURCES ${PROTON_SOURCES_BASIC})
	
	# TouchDeviceEmulatorPointerEventHandler needs these components
	proton_include_components(RectRenderComponent FocusRenderComponent)

	list(REMOVE_DUPLICATES PROTON_SOURCES)
	add_executable(${PROJECT_NAME} ${ARGV} ${PROTON_SOURCES})
	
	if(NOT PROTON_USE_INTERNAL_ZLIB)
		find_package(ZLIB REQUIRED)
		if(ZLIB_FOUND)
			include_directories(${ZLIB_INCLUDE_DIRS})
			target_link_libraries(${PROJECT_NAME} ${ZLIB_LIBRARIES})
		endif(ZLIB_FOUND)
	endif(NOT PROTON_USE_INTERNAL_ZLIB)
	
	

#	find_package(SDL REQUIRED)
#	if(SDL_FOUND)
#		include_directories(${SDL_INCLUDE_DIR})
#		target_link_libraries(${PROJECT_NAME} ${SDL_LIBRARY})
#	endif(SDL_FOUND)

if(PROTON_USE_SDL_AUDIO)
		target_link_libraries(${PROJECT_NAME} SDL2_mixer)
endif(PROTON_USE_SDL_AUDIO)


if(RASPBERRYPI_GLES11)
#note: GLESv2 has the v1.1 and v2 libraries on rasberry pi, you don't use GLESv1_CM!
target_link_libraries(${PROJECT_NAME} pthread bcm_host SDL2)

find_library(PI_GLSTUFF brcmGLESv2 /opt/vc/lib)

if (PI_GLSTUFF)
	#raspian stretch has renamed these files:
message(STATUS "Linking with libbrecmGLESv2 because we found 'em.  Using Raspbian stretch or newer probably")
	target_link_libraries(${PROJECT_NAME} brcmGLESv2.so brcmEGL.so)

else()

message(STATUS "Linking with GLESv2 and EGL for non-desktop raspbian because lib brcmGLESv2 can't be found")
target_link_libraries(${PROJECT_NAME} GLESv2 EGL)

endif()


else(RASPBERRYPI_GLES11)
	
	
	if(RASPBERRYPI_OPENGL)
	target_link_libraries(${PROJECT_NAME} GL pthread bcm_host SDL2)
	else(RASPBERRYPI_OPENGL)
	target_link_libraries(${PROJECT_NAME} gl)
	endif(RASPBERRYPI_OPENGL)
	
endif(RASPBERRYPI_GLES11)

target_link_libraries(${PROJECT_NAME} rt)
endfunction(proton_set_sources)


#simplified version
function(proton_set_sources_console)
	
	#use these instead
	list(APPEND PROTON_SOURCES ${PROTON_SOURCES_CONSOLE})
	
	list(REMOVE_DUPLICATES PROTON_SOURCES)
	add_executable(${PROJECT_NAME} ${ARGV} ${PROTON_SOURCES})
	
	if(NOT PROTON_USE_INTERNAL_ZLIB)
		find_package(ZLIB REQUIRED)
		if(ZLIB_FOUND)
			include_directories(${ZLIB_INCLUDE_DIRS})
			target_link_libraries(${PROJECT_NAME} ${ZLIB_LIBRARIES})
		endif(ZLIB_FOUND)
	endif(NOT PROTON_USE_INTERNAL_ZLIB)


if(RASPBERRYPI_GLES11)
target_link_libraries(${PROJECT_NAME} pthread bcm_host)
endif(RASPBERRYPI_GLES11)

target_link_libraries(${PROJECT_NAME} rt)
endfunction(proton_set_sources_console)

