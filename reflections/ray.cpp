#include "ray.h"

Ray::Ray()
{}

Ray::Ray(const Vector& _start, const Vector& _direction)
:start(_start), direction(_direction)
{}

Ray::Ray(const Ray& rhs)
:start(rhs.start), direction(rhs.direction)
{}

Ray Ray::rayBetween(const Vector& from, const Vector& to)
{
	return Ray(from, to - from);
}