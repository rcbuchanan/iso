#pragma once

#include <ostream>

#include "irrInc.hh"


template <typename T>
inline std::ostream &
operator<< (std::ostream &os, const vector3d<T> &v)
{
	os << "(" << v.X << ", " << v.Y << ", " << v.Z << ")";
	return os;
}

inline vector3df
v3df2v3di(vector3di v)
{
	return vector3df(v.X, v.Y, v.Z);
}
