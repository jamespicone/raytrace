#include "vector.h"

#include <cmath>

Vector::Vector()
:x(0), y(0), z(0)
{}

Vector::Vector(float _x, float _y, float _z)
:x(_x), y(_y), z(_z)
{}

Vector::Vector(const Vector& rhs)
:x(rhs.x), y(rhs.y), z(rhs.z)
{}

float Vector::length() const {return std::sqrt(x*x + y*y + z*z);}
float Vector::lengthsq() const {return x*x + y*y + z*z;}
float Vector::dotprod(const Vector& rhs) const
{
	return x*rhs.x + y*rhs.y + z*rhs.z;
}

Vector Vector::crossprod(const Vector& rhs) const
{
	return Vector(y*rhs.z - z*rhs.y, z*rhs.x - x*rhs.z, x*rhs.y - y*rhs.x);
}

void Vector::normalise()
{
	float l = length();
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

Vector Vector::operator*(float f) const
{
	return Vector(x*f, y*f, z*f);
}

Vector Vector::operator/(float f) const
{
	return Vector(x/f, y/f, z/f);
}

Vector& Vector::operator*=(float f)
{
	*this = *this * f;
	return *this;
}

Vector& Vector::operator/=(float f)
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

	y = static_cast<float>(y * cos(rad) - z * sin(rad));
	z = static_cast<float>(y_t * sin(rad) + z * cos(rad));
}

void Vector::rotateAroundY(double rad)
{
	double x_t = x;

	x = static_cast<float>(x * cos(rad) - z * sin(rad));
	z = static_cast<float>(x_t * sin(rad) + z * cos(rad));
}