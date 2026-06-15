#ifndef __MIRROR_VECTOR_H
#define __MIRROR_VECTOR_H

class Vector
{
public:
	float x;
	float y;
	float z;

	Vector();
	Vector(float _x, float _y, float _z);
	Vector(const Vector& rhs);

	float length() const;
	float lengthsq() const;
	float dotprod(const Vector& rhs) const;
	Vector crossprod(const Vector& rhs) const;

	void normalise();

	void rotateAroundX(double angle);
	void rotateAroundY(double angle);

	Vector operator+(const Vector& rhs) const;
	Vector operator-(const Vector& rhs) const;
	Vector& operator+=(const Vector& rhs);
	Vector& operator-=(const Vector& rhs);

	Vector operator*(float f) const;
	Vector operator/(float f) const;
	Vector& operator*=(float f);
	Vector& operator/=(float f);
	
	bool operator==(const Vector& rhs) const;
	bool operator!=(const Vector& rhs) const;
};

#endif