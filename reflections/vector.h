#ifndef __MIRROR_VECTOR_H
#define __MIRROR_VECTOR_H

class Vector
{
public:
	double x;
	double y;
	double z;
	
	Vector();
	Vector(double _x, double _y, double _z);
	Vector(const Vector& rhs);

	double length() const;
	double lengthsq() const;
	double dotprod(const Vector& rhs) const;
	Vector crossprod(const Vector& rhs) const;
	
	void normalise();
	
	void rotateAroundX(double angle);
	void rotateAroundY(double angle);
	
	Vector operator+(const Vector& rhs) const;
	Vector operator-(const Vector& rhs) const;
	Vector& operator+=(const Vector& rhs);
	Vector& operator-=(const Vector& rhs);
	
	Vector operator*(double f) const;
	Vector operator/(double f) const;
	Vector& operator*=(double f);
	Vector& operator/=(double f);
	
	bool operator==(const Vector& rhs) const;
	bool operator!=(const Vector& rhs) const;
};

#endif