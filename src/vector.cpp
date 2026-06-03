#include "vector.h"

#include <cmath>

Vector::Vector()
:x(0), y(0), z(0)
{}

Vector::Vector(double _x, double _y, double _z)
:x(_x), y(_y), z(_z)
{}

Vector::Vector(const Vector& rhs)
:x(rhs.x), y(rhs.y), z(rhs.z)
{}

double Vector::length() const {return std::sqrt(x*x + y*y + z*z);}
double Vector::lengthsq() const {return x*x + y*y + z*z;}
double Vector::dotprod(const Vector& rhs) const
{
	return x*rhs.x + y*rhs.y + z*rhs.z;
}

Vector Vector::crossprod(const Vector& rhs) const
{
	return Vector(y*rhs.z - z*rhs.y, z*rhs.x - x*rhs.z, x*rhs.y - y*rhs.x);
}

void Vector::normalise()
{
	double l = length();
	if (l != 0) {*this /= l;}
}

Vector Vector::operator+(const Vector& rhs) const
{
	return Vector(x + rhs.x, y + rhs.y, z + rhs.z);
}

Vector Vector::operator-(const Vector& rhs) const
{
	return Vector(x - rhs.x, y - rhs.y, z - rhs.z);
}

Vector& Vector::operator+=(const Vector& rhs)
{
	*this = *this + rhs;
	return *this;
}

Vector& Vector::operator-=(const Vector& rhs)
{
	*this = *this - rhs;
	return *this;
}

Vector Vector::operator*(double f) const
{
	return Vector(x*f, y*f, z*f);
}

Vector Vector::operator/(double f) const
{
	return Vector(x/f, y/f, z/f);
}

Vector& Vector::operator*=(double f)
{
	*this = *this * f;
	return *this;
}

Vector& Vector::operator/=(double f)
{
	*this = *this / f;
	return *this;
}

bool Vector::operator==(const Vector& rhs) const
{
	return x == rhs.x && y == rhs.y && z == rhs.z;
}

bool Vector::operator!=(const Vector& rhs) const
{
	return !(*this == rhs);
}

void Vector::rotateAroundX(double rad)
{
	double y_t = y;
	
	y = y * cos(rad) - z * sin(rad);
	z = y_t * sin(rad) + z * cos(rad);
}

void Vector::rotateAroundY(double rad)
{
	double x_t = x;
	
	x = x * cos(rad) - z * sin(rad);
	z = x_t * sin(rad) + z * cos(rad);
}