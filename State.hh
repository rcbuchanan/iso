#pragma once

#include <iostream>

#include "irrInc.hh"

#include "Util.hh"


enum AnimationType {
	COMPLETED_ANIMATION,
	DEATH_ANIMATION,
	BORN_ANIMATION,
};

enum EventType {
	SNAKE_DED_EVENT,
	SNAKE_BORN_EVENT,
	WIN_EVENT,
	RIGHT_CMD_EVENT,
	LEFT_CMD_EVENT,
	UP_CMD_EVENT,
	DOWN_CMD_EVENT,
	BACK_CMD_EVENT,
	CMD_EVENT_COUNT
};

struct isoEvent {
	EventType _type;
	void *_data;
	isoEvent (EventType type) :
		_type (type),
		_data (NULL) {}
};

struct isoAnimation {
	AnimationType _type;
	f32 _t0;
	ISceneNodeAnimator *_animator;

	isoAnimation (AnimationType type) :
		_type (type),
		_t0 (-1),
		_animator (NULL) {}
};

struct isoSnakeJoint {
	vector3df _nextmove;
	vector3df _prevmove;
	vector3df _pos;
	IAnimatedMeshSceneNode *_node;
	bool _anchored;
	
	isoSnakeJoint (vector3df nextmove, vector3df prevmove, vector3df pos, bool anchored) :
		_nextmove (nextmove),
		_prevmove (prevmove),
		_pos (pos),
		_node (NULL),
		_anchored (anchored) {}
};

struct isoBlock {
	vector3df _pos;
	IAnimatedMeshSceneNode *_node;
	bool _isgoal;

	isoBlock (vector3df pos, bool isgoal) :
		_pos (pos),
		_node (NULL),
		_isgoal (isgoal) {}
};


class isoState {
public:
	vector3df _cameraPos;
	vector3df _cameraFwd;
	vector3df _cameraUp;

	array<isoEvent> _events;

	array<isoBlock> _blocks;

	array<isoSnakeJoint> _snakeJoints;
	array<isoSnakeJoint> _previewJoints;
	u32 _snakeVisibleHead;
	bool _snakeControlLocked;
	bool _snakeIsDead;

	array<isoAnimation> _animations;

	isoState (vector3df cameraPos) :
		_cameraPos (cameraPos),
		_cameraFwd (0, 0, 10.0f),
		_cameraUp (0, 1.0f, 0),
		_events (),
		_blocks (),
		_snakeJoints (),
		_snakeVisibleHead (0),
		_snakeControlLocked (true),
		_snakeIsDead (true) {
	}
};


isoState *buildTestIsoState();
