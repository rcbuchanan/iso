#include <ostream>
#include <math.h>

#include "irrInc.hh"

#include "Scene.hh"
#include "Logic.hh"
#include "State.hh"

int
main ()
{
	isoState *state = buildTestIsoState();
	isoScene *scene = new isoScene(*state);
	isoLogic *logic = new isoLogic(*state);

	f32 delta;

	while (scene->begincycle(delta)) {
		scene->updateInput(delta);
		logic->update(delta);
		scene->updateGraph(delta);
		scene->draw();
	}
	
	return 0;
}
