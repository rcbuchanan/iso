#pragma once

#include "irrInc.hh"

class isoState;

class isoLogic {
public:
	isoLogic (isoState &state);
	void update (f32 delta);

private:
	void createSnake();
	isoSnakeJoint makeNextJoint (const isoSnakeJoint &joint0, int twist);
	void updatePreviewJoints ();
	bool testSelfCollision (const isoSnakeJoint &joint, u32 starti, u32 endi) const;

	isoState &_state;
};
