#include "triangle.h"

#include <cmath>
#include <cfloat>

Triangle::Triangle()
:colour(0), reflective(false), reflectivity(0), invisible(false), glows(false)
{
	calcInternalVectors();
}

Triangle::Triangle(const Vector& _v0, const Vector& _v1, const Vector& _v2, Colour _colour, bool _reflective, double _reflectivity, bool _invisible, bool _glows)
:v0(_v0), v1(_v1), v2(_v2), colour(_colour), reflective(_reflective), reflectivity(_reflectivity), invisible(_invisible), glows(_glows)
{
	calcInternalVectors();
}

Triangle::Triangle(const Triangle& rhs)
:v0(rhs.v0), v1(rhs.v1), v2(rhs.v2), colour(rhs.colour), reflective(rhs.reflective), reflectivity(rhs.reflectivity), invisible(rhs.invisible), glows(rhs.glows)
{
	calcInternalVectors();
}

void Triangle::calcInternalVectors()
{
	v0v1 = v1 - v0;
	v0v2 = v2 - v0;
	
	normal = v0v1.crossprod(v0v2);
	normal.normalise();
}

bool Triangle::getIntersection(const Ray& ray, Vector& ret, double& t_ret, Colour& colour_ret)
{
	Vector h = ray.direction.crossprod(v0v2);
	double a = v0v1.dotprod(h);
	if (fabs(a) < DBL_EPSILON) {return false;}
	
	double f =  1/a;
	Vector s = ray.start - v0;
	double u = f * s.dotprod(h);
	if (u < 0 || u > 1) {return false;}
	
	Vector q = s.crossprod(v0v1);
	double v = f * ray.direction.dotprod(q);
	if (v < 0 || u + v > 1) {return false;}
	
	double t = f * v0v2.dotprod(q);
	if (t > DBL_EPSILON)
	{
		ret = ray.start + ray.direction * t;
		t_ret = t;
		colour_ret = colour;
		return true;
	}
	else {return false;}
}

const Vector& Triangle::getNormal() const
{
	return normal;
}