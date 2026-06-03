#ifndef __MIRRORHOUSE_RAY_H
#define __MIRRORHOUSE_RAY_H

#include "vector.h"

class Ray
{
public:
	Vector start;
	Vector direction;
	
	Ray();
	Ray(const Vector& _start, const Vector& _direction);
	Ray(const Ray&);
	static Ray rayBetween(const Vector& from, const Vector& to);
};

#endif