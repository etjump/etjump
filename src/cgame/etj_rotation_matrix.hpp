#ifndef ROTATION_MATRIX_HPP
#define ROTATION_MATRIX_HPP

#include <math.h> 

#define PI 3.14159265

class RotationMatrix
{

public:

	typedef float vec_t;
	typedef vec_t vec3_t[3];

	void init()
	{
		_m11 = 0;_m12 = 0;_m13 = 0;_m14 = 0;
		_m21 = 0;_m22 = 0;_m23 = 0;_m24 = 0;
		_m31 = 0;_m32 = 0;_m33 = 0;_m34 = 0;
		_degree = 0;
	}

	void rotateAlongAnAbitaryAxis(vec3_t center, vec3_t axis)
	{
		// Use centroid of the object of rotation origin.
		double a = center[0];
		double b = center[1];
		double c = center[2];

		// Use vector (0,0,1) as Z-Up-Vector for the rotation
		double u = axis[0];
		double v = axis[1];
		double w = axis[2];

		// Set some intermediate values.
		double u2 = u*u;
		double v2 = v*v;
		double w2 = w*w;
		double cosT = std::cos(_degree);
		double oneMinusCosT = 1 - cosT;
		double sinT = std::sin(_degree);
		
		// Build the matrix entries element by element.
		_m11 = u2 + (v2 + w2) * cosT;
		_m12 = u*v * oneMinusCosT - w*sinT;
		_m13 = u*w * oneMinusCosT + v*sinT;
		_m14 = (a*(v2 + w2) - u*(b*v + c*w))*oneMinusCosT
			+ (b*w - c*v)*sinT;

		_m21 = u*v * oneMinusCosT + w*sinT;
		_m22 = v2 + (u2 + w2) * cosT;
		_m23 = v*w * oneMinusCosT - u*sinT;
		_m24 = (b*(u2 + w2) - v*(a*u + c*w))*oneMinusCosT
			+ (c*u - a*w)*sinT;

		_m31 = u*w * oneMinusCosT - v*sinT;
		_m32 = v*w * oneMinusCosT + u*sinT;
		_m33 = w2 + (u2 + v2) * cosT;
		_m34 = (c*(u2 + v2) - w*(a*u + b*v))*oneMinusCosT
			+ (a*v - b*u)*sinT;	

		return;
	}

	void rotateAPoint(vec3_t srcPoint, vec3_t& dstPoint)
	{
		// Compute new point with the rotation.
		dstPoint[0] = _m11*srcPoint[0] + _m12*srcPoint[1] + _m13*srcPoint[2] + _m14;
		dstPoint[1] = _m21*srcPoint[0] + _m22*srcPoint[1] + _m23*srcPoint[2] + _m24;
		dstPoint[2] = _m31*srcPoint[0] + _m32*srcPoint[1] + _m33*srcPoint[2] + _m34;

		return;
	}
	
	double getDegree(){ return _degree; }
	void setDegree(double d){ _degree = d; }

private:
	double _m11, _m12, _m13, _m14;
	double _m21, _m22, _m23, _m24;
	double _m31, _m32, _m33, _m34;

	double _degree;

};
#endif