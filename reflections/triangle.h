#ifndef __MIRRORHOUSE_TRIANGLE_H
#define __MIRRORHOUSE_TRIANGLE_H

#include "vector.h"
#include "ray.h"
#include "colour.h"

#include <cstdint>

class Triangle
{
public:
	Vector v0;
	Vector v1;
	Vector v2;
	
	Colour colour;
	
	bool reflective;
	double reflectivity;
	
	bool invisible;
	
	bool glows;
	
	Triangle();
	Triangle(const Vector& _v0, const Vector& _v1, const Vector& _v2, Colour _colour, bool reflective = false, double _reflectivity = 0, bool invisible = false, bool glows = false);
	Triangle(const Triangle& t);
	
	bool getIntersection(const Ray& ray, Vector& ret, double& t, Colour& colour);
	const Vector& getNormal() const;
	
private:
	void calcInternalVectors();
	
	Vector v0v1;
	Vector v0v2;
	Vector normal;
};

#endif