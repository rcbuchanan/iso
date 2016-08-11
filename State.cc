#include "Reg.hh"
#include "State.hh"

isoState *
buildTestIsoState ()
{
	isoState *state = new isoState(
		vector3df(0, 0.f, -1.0f));

	return state;
}