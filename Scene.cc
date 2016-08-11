#include <iostream>
#include <ostream>
#include <math.h>

#include "irrInc.hh"

#include "Scene.hh"
#include "Reg.hh"
#include "Util.hh"


// derived from irrlicht's FPS camera code
static void
mouseLook(position2d<f32> cursorMove, vector3df &fwd)
{
	const f32 rotateSpeed = 70.0f;
	const f32 maxVerticalAngle = 88.0f;

	vector3df relativeRotation = fwd.getHorizontalAngle();

	if (cursorMove.getLength() > 0.0001f) {
		relativeRotation.Y -= cursorMove.X * rotateSpeed;
		relativeRotation.X -= cursorMove.Y * rotateSpeed;

		// X < MaxVerticalAngle or X > 360-MaxVerticalAngle
		if (relativeRotation.X > maxVerticalAngle * 2 &&
			relativeRotation.X < 360.0f - maxVerticalAngle) {
			relativeRotation.X = 360.0f - maxVerticalAngle;
		}
		else if (relativeRotation.X > maxVerticalAngle &&
			relativeRotation.X < 360.0f - maxVerticalAngle) {
			relativeRotation.X = maxVerticalAngle;
		}

		matrix4 mat;
		mat.setRotationDegrees(relativeRotation);
		fwd = vector3df(0.0f, 0.0f, 1.0f);
		mat.transformVect(fwd);
	}
}

static dimension2d<u32>
getNativeResolution ()
{
	IrrlichtDevice *d = createDevice(EDT_NULL);
	dimension2d<u32> r = d->getVideoModeList()->getDesktopResolution();
	d->drop();
	return r;
}

void
isoScene::draw ()
{
	_deskres = _driver->getCurrentRenderTargetSize();
	_cursor->setPosition(0.5f, 0.5f);
	_smgr->getActiveCamera()->setAspectRatio(_deskres.Width / (f32) _deskres.Height);
	
	//_driver->beginScene(true, true, SColor(255, 135, 206, 250));
	_driver->beginScene(true, true, SColor(255, 0, 0, 80));
	_smgr->drawAll();
	_guienv->drawAll();
	_driver->endScene();
}

void
isoScene::startclock ()
{
	_lastupdate = _device->getTimer()->getTime();
}

bool
isoScene::begincycle (f32 &delta)
{
	const u32 old = _lastupdate;
	_lastupdate = _device->getTimer()->getTime();
	delta = (old - _lastupdate) / 1.0e9f;
	return _device->run();
}

void
isoScene::updateGraph (f32 delta)
{
	f32 now = _lastupdate / 1.09f + delta;

	// process animations
	{
		array<u32> completedanims;
		u32 i = 0;
		while (i < _state._animations.size()) {
			isoAnimation &anim = _state._animations[i];

			if (anim._t0 < 0) {
				anim._t0 = now;
			}

			if (anim._type == COMPLETED_ANIMATION) {
				_state._animations.erase(i);
			} else {
				i++;
			}
		}
	}

	// update the camera
	{
		ICameraSceneNode *camera = _smgr->getActiveCamera();
		camera->setPosition(_state._cameraPos);
		camera->setTarget(_state._cameraPos + _state._cameraFwd);
		camera->setUpVector(_state._cameraUp);
	}

	// update visible blocks
	{
		for (u32 i = 0; i < _state._blocks.size(); i++) {
			isoBlock &block = _state._blocks[i];

			// create if missing node
			if (block._node == NULL) {
				IAnimatedMeshSceneNode *meshnode = _smgr->addAnimatedMeshSceneNode(_smgr->getMesh(isoReg::STAR_PATH), NULL);

				meshnode->setMaterialFlag(EMF_NORMALIZE_NORMALS, true);
				meshnode->setMaterialFlag(EMF_COLOR_MATERIAL, false);
				meshnode->setMaterialType(EMT_SOLID);
				//meshnode->setDebugDataVisible(EDS_NORMALS);
				//meshnode->setMaterialFlag(EMF_WIREFRAME, true);
				meshnode->getMaterial(0).DiffuseColor = video::SColor(255, 60, 60, i % 2 ? 0 : 120);
				meshnode->getMaterial(0).AmbientColor = video::SColor(255, 255, 255, 255);
				meshnode->getMaterial(0).EmissiveColor = video::SColor(0, 0, 0, 0);
				meshnode->getMaterial(0).SpecularColor = video::SColor(0, 0, 0, 0);
				meshnode->getMaterial(0).Shininess = 0.f;
				meshnode->addShadowVolumeSceneNode();

				if (block._isgoal) {
					ISceneNodeAnimator *animator = _smgr->createRotationAnimator(vector3df(0, 1, 0));
					meshnode->addAnimator(animator);
				}

				block._node = meshnode;
			}

			block._node->setPosition(block._pos);
		}
	}

	// update snake
	{
		AnimationType animtype = COMPLETED_ANIMATION;
		f32 animtime;
		bool deleteSnakeNodes = false;

		// find relevant animations
		for (u32 i = 0; i < _state._animations.size(); i++) {
			isoAnimation &anim = _state._animations[i];

			if (anim._type == DEATH_ANIMATION) {
				animtime = now - anim._t0;
				animtype = DEATH_ANIMATION;
				
				if (animtime > isoReg::DEATH_ANIMATION_DURATION) {
					_state._events.push_back(isoEvent(SNAKE_DED_EVENT));
					anim._type = COMPLETED_ANIMATION;

					deleteSnakeNodes = true;
				}
			} else if (anim._type == BORN_ANIMATION) {
				animtime = now - anim._t0;
				animtype = BORN_ANIMATION;

				if (animtime > isoReg::BIRTH_ANIMATION_DURATION) {
					_state._events.push_back(isoEvent(SNAKE_BORN_EVENT));
					anim._type = COMPLETED_ANIMATION;
				}
			}
		}

		// silly preview joints
		for (u32 i = 0; i < _state._previewJoints.size(); i++) {
			isoSnakeJoint &joint = _state._previewJoints[i];

			if (joint._node == NULL) {
				IAnimatedMeshSceneNode *meshnode = _smgr->addAnimatedMeshSceneNode(_smgr->getMesh(isoReg::SNAKE_PART_PATH), NULL);
				std::cout << "adding" << std::endl;

				meshnode->setMaterialFlag(EMF_NORMALIZE_NORMALS, true);
				meshnode->setMaterialFlag(EMF_COLOR_MATERIAL, false);
				meshnode->setMaterialType(EMT_SOLID);
				//meshnode->setDebugDataVisible(EDS_NORMALS);
				meshnode->setMaterialFlag(EMF_WIREFRAME, true);
				meshnode->getMaterial(0).DiffuseColor.set(255, 60, 60, i % 2 ? 0 : 120);
				meshnode->getMaterial(0).AmbientColor.set(255, 255, 255, 255);
				meshnode->getMaterial(0).EmissiveColor.set(0, 0, 0, 0);
				meshnode->getMaterial(0).SpecularColor.set(0, 0, 0, 0);
				meshnode->getMaterial(0).Shininess = 0.f;
				meshnode->addShadowVolumeSceneNode();

				joint._node = meshnode;
			}

			// update rotation, position, and color of node
			{
				// TODO: wow, this is not how a lookat matrix should be used...
				// Also, note that I am now smart enough to use transpose instead of invert.
				matrix4 jointrotator;
				const vector3df zaxis = joint._nextmove;
				const vector3df xaxis = joint._prevmove;
				const vector3df yaxis = zaxis.crossProduct(xaxis);
				jointrotator.buildCameraLookAtMatrixRH(vector3df(0, 0, 0), yaxis, zaxis);
				jointrotator = jointrotator.getTransposed();
				joint._node->setRotation(jointrotator.getRotationDegrees());

				joint._node->setPosition(joint._pos);

				joint._node->getMaterial(0).EmissiveColor = video::SColor(255, 0, joint._anchored ? 255 : 0, 0);
			}
		}

		// update visible snake by joint
		for (int i = _state._snakeVisibleHead; i < _state._snakeJoints.size(); i++) {
			isoSnakeJoint &joint = _state._snakeJoints[i];

			if (deleteSnakeNodes) {
				if (joint._node != NULL) {
					joint._node->remove();
					joint._node = NULL;
				}

				continue;
			} else if (joint._node == NULL) {
				IAnimatedMeshSceneNode *meshnode = _smgr->addAnimatedMeshSceneNode(_smgr->getMesh(isoReg::SNAKE_PART_PATH), NULL);
				std::cout << "adding" << std::endl;

				meshnode->setMaterialFlag(EMF_NORMALIZE_NORMALS, true);
				meshnode->setMaterialFlag(EMF_COLOR_MATERIAL, false);
				meshnode->setMaterialType(EMT_SOLID);
				//meshnode->setDebugDataVisible(EDS_NORMALS);
				//meshnode->setMaterialFlag(EMF_WIREFRAME, true);
				meshnode->getMaterial(0).DiffuseColor.set(255, 60, 60, i % 2 ? 0 : 120);
				meshnode->getMaterial(0).AmbientColor.set(255, 255, 255, 255);
				meshnode->getMaterial(0).EmissiveColor.set(0, 0, 0, 0);
				meshnode->getMaterial(0).SpecularColor.set(0, 0, 0, 0);
				meshnode->getMaterial(0).Shininess = 0.f;
				meshnode->addShadowVolumeSceneNode();

				joint._node = meshnode;
			}

			// update rotation, position, and color of node
			{
				// TODO: wow, this is not how a lookat matrix should be used...
				// Also, note that I am now smart enough to use transpose instead of invert.
				matrix4 jointrotator;
				const vector3df zaxis = joint._nextmove;
				const vector3df xaxis = joint._prevmove;
				const vector3df yaxis = zaxis.crossProduct(xaxis);
				jointrotator.buildCameraLookAtMatrixRH(vector3df(0, 0, 0), yaxis, zaxis);
				jointrotator = jointrotator.getTransposed();
				joint._node->setRotation(jointrotator.getRotationDegrees());

				joint._node->setPosition(joint._pos);

				joint._node->getMaterial(0).EmissiveColor = video::SColor(255, 0, joint._anchored ? 255 : 0, 0);
			}

			switch (animtype) {
			case BORN_ANIMATION:
				joint._node->setScale(vector3df(animtime / isoReg::BIRTH_ANIMATION_DURATION));
				break;
			case DEATH_ANIMATION:
				joint._node->setScale(vector3df(1.0 - animtime / isoReg::DEATH_ANIMATION_DURATION));
				break;
			case COMPLETED_ANIMATION:
				joint._node->setScale(vector3df(1.0));
			default:
				break;
			}
		}
	}
}

void
isoScene::updateInput (f32 delta)
{
	// control the FPS camera
	{
		const position2d<f32> mid = position2d<f32>(0.5f, 0.5f);
		mouseLook(mid - _cursor->getRelativePosition(), _state._cameraFwd);

		const vector3df &fwd = _state._cameraFwd;
		const vector3df &up = _state._cameraUp;
		const vector3df right = fwd.crossProduct(up).normalize();

		vector3df velocity(0, 0, 0);
		if (_keyMaster._keyIsDown[KEY_COMMA]) {
			velocity += fwd;
		}
		if (_keyMaster._keyIsDown[KEY_KEY_O]) {
			velocity -= fwd;
		}
		if (_keyMaster._keyIsDown[KEY_PERIOD]) {
			velocity += up;
		}
		if (_keyMaster._keyIsDown[KEY_OEM_7]) {
			velocity -= up;
		}
		if (_keyMaster._keyIsDown[KEY_KEY_A]) {
			velocity += right;
		}
		if (_keyMaster._keyIsDown[KEY_KEY_E]) {
			velocity -= right;
		}
		velocity.normalize();
		velocity *= 40;

		_state._cameraPos += velocity * delta / 1000.0f;
		//_state._cameraFwd = _state._cameraPos + fwd;
	}

	// update cmds to process in state
	{
		if (_keyMaster._keyJustPressed[KEY_KEY_1]) {
			_state._events.push_back(isoEvent(RIGHT_CMD_EVENT));
		}

		if (_keyMaster._keyJustPressed[KEY_KEY_2]) {
			_state._events.push_back(isoEvent(LEFT_CMD_EVENT));
		}

		if (_keyMaster._keyJustPressed[KEY_KEY_3]) {
			_state._events.push_back(isoEvent(UP_CMD_EVENT));
		}

		if (_keyMaster._keyJustPressed[KEY_KEY_4]) {
			_state._events.push_back(isoEvent(DOWN_CMD_EVENT));
		}

		if (_keyMaster._keyJustPressed[KEY_BACK]) {
			_state._events.push_back(isoEvent(BACK_CMD_EVENT));
		}

		for (u32 i = 0; i < sizeof(_keyMaster._keyJustPressed) / sizeof(_keyMaster._keyJustPressed[0]); i++) {
			_keyMaster._keyJustPressed[i] = false;
		}
	}
}

isoScene::isoScene (isoState &state) :
	_state (state),
	_lastupdate (0),
	_keyMaster ()
{
	SIrrlichtCreationParameters params;
	params.Fullscreen = false;
	if (params.Fullscreen) {
		params.WindowSize = getNativeResolution();
	} else {
		params.WindowSize = dimension2d<u32>(640, 480);
	}
	params.Stencilbuffer = true;
	params.Vsync = true;
	params.AntiAlias = 4;
	params.Bits = 16;
	params.DriverType = EDT_OPENGL;
	_device = createDeviceEx(params);
	
	_device->setWindowCaption(L"Iso");
	_device->setEventReceiver(&_keyMaster);
	
	_driver = _device->getVideoDriver();
	_smgr = _device->getSceneManager();
	_cmgr = _smgr->getSceneCollisionManager();
	_guienv = _device->getGUIEnvironment();
	_filesys = _device->getFileSystem();
	
	_cursor = _device->getCursorControl();
	_cursor->setVisible(false);
	
	vector3df lightpos = vector3df(2, 20, 0);
	ILightSceneNode *light = _smgr->addLightSceneNode(0, lightpos, SColor(255, 8, 8, 8), 800.0);
	_smgr->addCubeSceneNode(1.0f, NULL, -1, lightpos)->getMaterial(0).EmissiveColor.set(255, 255, 255, 255);

	light->enableCastShadow(true);
	_smgr->setAmbientLight(SColor(255, 70, 30, 30));

	_smgr->addAnimatedMeshSceneNode(_smgr->addHillPlaneMesh("foo", dimension2df(10, 10), dimension2du(10, 10)))->setPosition(vector3df(0, -2, 0));

	_smgr->addSkyBoxSceneNode(
		_driver->getTexture(isoReg::SKYBOX_PATH[0]),
		_driver->getTexture(isoReg::SKYBOX_PATH[1]),
		_driver->getTexture(isoReg::SKYBOX_PATH[2]),
		_driver->getTexture(isoReg::SKYBOX_PATH[3]),
		_driver->getTexture(isoReg::SKYBOX_PATH[4]),
		_driver->getTexture(isoReg::SKYBOX_PATH[5]));

	ICameraSceneNode *camera = _smgr->addCameraSceneNode(
		0,
		_state._cameraPos,
		_state._cameraPos + _state._cameraFwd);
	camera->setNearValue(0.1);
	camera->setFarValue(20000);
}

isoScene::~isoScene ()
{
	_device->drop();
}

KeyMaster::KeyMaster ()
{
	for (s32 i = 0; i < KEY_KEY_CODES_COUNT; i++) {
		_keyIsDown[i] = false;
		_keyJustPressed[i] = false;
	}
}

bool
KeyMaster::OnEvent (const SEvent &event)
{
	if (event.EventType == EET_KEY_INPUT_EVENT) {
		EKEY_CODE code = event.KeyInput.Key;
		bool pressed = event.KeyInput.PressedDown;

		_keyJustPressed[code] = !_keyIsDown[code] && pressed;
		_keyIsDown[code] = pressed;
	}
	return false;
}
	
KeyMaster::~KeyMaster ()
{
}
