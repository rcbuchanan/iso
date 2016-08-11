#include "Reg.hh"
#include "State.hh"

#include "Logic.hh"


// TODO: try to understand why math with normals is confusing. Here, I mean that in a high-level way.
isoSnakeJoint
isoLogic::makeNextJoint (const isoSnakeJoint &joint0, int twist)
{
	vector3df nextnextmove = -joint0._prevmove;
	vector3df nextprevmove = -joint0._nextmove;
	vector3df nextpos = joint0._pos + isoReg::GRID_DIST * joint0._nextmove;
	bool anchored = false;

	// apply twist (rotate nextnextmove around the axis of _nextmove)
	matrix4 twistmatrix;
	twistmatrix.setRotationAxisRadians(twist * HALF_PI, joint0._nextmove);
	twistmatrix.transformVect(nextnextmove);

	// determined 'anchored' status
	for (u32 j = 0; j < _state._blocks.size(); j++) {
		if (nextpos.getDistanceFrom(_state._blocks[j]._pos) < isoReg::CLOSE_DIST) {
			if (_state._blocks[j]._isgoal) {
				_state._events.push_back(isoEvent(WIN_EVENT));
			}

			anchored = true;
			break;
		}
	}
	
	return isoSnakeJoint(nextnextmove, nextprevmove, nextpos, anchored);
}

void
isoLogic::updatePreviewJoints ()
{
	// make preview joints
	for (u32 i = 0; i < 4; i++) {
		if (i >= _state._previewJoints.size()) {
			_state._previewJoints.push_back(makeNextJoint(_state._snakeJoints.getLast(), i));
		} else {
			IAnimatedMeshSceneNode *node = _state._previewJoints[i]._node;
			_state._previewJoints[i] = makeNextJoint(_state._snakeJoints.getLast(), i);
			_state._previewJoints[i]._node = node;

			if (node) {
				node->setVisible(!testSelfCollision(_state._previewJoints[i], _state._snakeVisibleHead, _state._snakeJoints.size()));
			}
		}
	}
}

bool
isoLogic::testSelfCollision (const isoSnakeJoint &joint, u32 starti, u32 endi) const
{
	vector3df norm = (joint._nextmove + joint._prevmove).normalize();

	for (u32 i = starti; i < endi; i++) {
		isoSnakeJoint &j2 = _state._snakeJoints[i];
		f32 d = abs(norm.dotProduct(j2._nextmove) - norm.dotProduct(j2._prevmove));

		if (j2._pos == joint._pos && d > isoReg::CLOSE_DIST / isoReg::GRID_DIST) {
			return true;
		}
	}

	return false;
}

void
isoLogic::createSnake ()
{
	isoSnakeJoint *prevjoint = NULL;

	// make snake
	_state._snakeVisibleHead = 0;
	for (u32 i = 0; i < isoReg::SNAKE_LENGTH; i++) {
		if (prevjoint == NULL) {
			_state._snakeJoints.push_back(
				isoSnakeJoint(vector3df(1, 0, 0), vector3df(0, 0, 1), vector3df(0, 0, 0), false));
		}
		else {
			_state._snakeJoints.push_back(makeNextJoint(*prevjoint, 0));
		}

		prevjoint = &_state._snakeJoints.getLast();
	}
}

isoLogic::isoLogic (isoState &state) :
	_state (state)
{
	// load XML data
	if (0) {
		IrrlichtDevice *nulldev = createDevice(EDT_NULL);
		IXMLReader *xfile = nulldev->getFileSystem()->createXMLReader(isoReg::LEVEL_PATHS[0]);



		while (xfile->read()) {
			switch (xfile->getNodeType()) {
			case EXN_ELEMENT:
				std::cout << "element" << std::endl;
				break;
			case EXN_ELEMENT_END:
				std::cout << "end element" << std::endl;
				break;
			}
		}

		xfile->drop();
		nulldev->drop();
	}

	// add the blocks
	{
		const char grid[8][8] = {
			{ 1, 1, 1, 0, 0, 3, 0, 4 },
			{ 1, 1, 1, 0, 0, 0, 0, 0 },
			{ 1, 1, 1, 0, 2, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 1, 0, 0, 0, 0, 0, 0, 0 },
			{ 0, 3, 0, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 4, 0, 0, 0, 0, 0, 0, 0 }
		};
		const int goalpos[2] = { 0, 7 };

		for (u32 i = 0; i < sizeof (grid) / sizeof (grid[0]); i++) {
			for (u32 j = 0; j < sizeof (grid[0]) / sizeof (grid[0][0]); j++) {
				if (grid[i][j] != 0) {
					const bool isgoal = i == goalpos[0] && j == goalpos[1];
					_state._blocks.push_back(isoBlock(vector3df(2.0 * i, grid[i][j] * 2.0 - 2.0, -2.0 * j), isgoal));
					std::cout << i << " " << j << std::endl;
				}
			}
		}
	}

	_state._snakeIsDead = false;
	_state._snakeControlLocked = false;
	createSnake();
}

void
isoLogic::update (f32 delta)
{
	s32 twist = -1;
	bool backtrack = false;
	bool respawnSnake = false;
	bool reenableSnake = false;
	bool wonLevel = false;

	u32 i = 0;
	while (i < _state._events.size()) {
		isoEvent &ev = _state._events[i];

		bool processed = true;
		switch (ev._type) {
		case RIGHT_CMD_EVENT:
			twist = 0;
			break;
		case LEFT_CMD_EVENT:
			twist = 1;
			break;
		case UP_CMD_EVENT:
			twist = 2;
			break;
		case DOWN_CMD_EVENT:
			twist = 3;
			break;
		case BACK_CMD_EVENT:
			backtrack = true;
			break;
		case SNAKE_DED_EVENT:
			respawnSnake = true;
			break;
		case SNAKE_BORN_EVENT:
			reenableSnake = true;
			break;
		case WIN_EVENT:
			wonLevel = true;
			break;
		default:
			processed = false;
			break;
		}

		if (processed) {
			_state._events.erase(i);
		} else {
			i++;
		}
	}

	if (reenableSnake) {
		_state._snakeControlLocked = false;
		_state._snakeIsDead = false;
	}

	if (respawnSnake) {
		while (_state._snakeJoints.size() > 0) {
			_state._snakeJoints.erase(_state._snakeJoints.size() - 1);
		}
		createSnake();

		_state._animations.push_back(isoAnimation(BORN_ANIMATION));
		_state._snakeControlLocked = true;
	} else if (wonLevel) {
		_state._animations.push_back(isoAnimation(DEATH_ANIMATION));
		_state._snakeControlLocked = true;
	} else if (!_state._snakeControlLocked) {
		// movement or backtracking
		if (backtrack) {
			if (_state._snakeJoints.size() > isoReg::SNAKE_LENGTH) {
				_state._snakeVisibleHead--;
				_state._snakeJoints[_state._snakeVisibleHead]._node = _state._snakeJoints.getLast()._node;
				_state._snakeJoints.erase(_state._snakeJoints.size() - 1);
			}
		}
		else if (twist != -1) {
			_state._snakeJoints.push_back(makeNextJoint(_state._snakeJoints.getLast(), twist));
			isoSnakeJoint &nxtjoint = _state._snakeJoints.getLast();
			_state._snakeVisibleHead++;

			bool legalmove = true;
			bool fatalmove = false;

			legalmove = !testSelfCollision(nxtjoint, _state._snakeVisibleHead, _state._snakeJoints.size() - 1);

			// see if move would move off-anchor
			if (!_state._snakeJoints[_state._snakeVisibleHead]._anchored) {
				fatalmove = true;
				for (int i = _state._snakeVisibleHead; i < _state._snakeJoints.size(); i++) {
					if (_state._snakeJoints[i]._anchored) {
						fatalmove = false;
						break;
					}
				}
			}

			if (!legalmove) {
				_state._snakeVisibleHead--;
				_state._snakeJoints.erase(_state._snakeJoints.size() - 1);
			}
			else {
				nxtjoint._node = _state._snakeJoints[_state._snakeVisibleHead - 1]._node;
				_state._snakeJoints[_state._snakeVisibleHead - 1]._node = NULL;

				if (fatalmove) {
					_state._animations.push_back(isoAnimation(DEATH_ANIMATION));
					_state._snakeControlLocked = true;
				}
			}
		}

		updatePreviewJoints();
	}
}
