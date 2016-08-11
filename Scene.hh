#pragma once

#include "irrInc.hh"

#include "State.hh"


class KeyMaster : public IEventReceiver {
public:
	virtual bool OnEvent(const SEvent &event);
	KeyMaster();
	~KeyMaster();

	bool _keyIsDown[KEY_KEY_CODES_COUNT];
	bool _keyJustPressed[KEY_KEY_CODES_COUNT];
};

// interaction with irrlicht engine
class isoScene {
public:
	void updateInput (f32 delta);
	void updateGraph (f32 delta);
	void draw ();
	void startclock ();
	bool begincycle (f32 &delta);

	isoScene (isoState &state);
	~isoScene ();

private:
	isoState &_state;
	u32 _lastupdate;
	dimension2d<u32> _deskres;
	
	IrrlichtDevice *_device;
	IVideoDriver *_driver;
	ISceneManager *_smgr;
	ISceneCollisionManager *_cmgr;
	IGUIEnvironment *_guienv;
	IFileSystem *_filesys;
	ICursorControl *_cursor;

	KeyMaster _keyMaster;
};