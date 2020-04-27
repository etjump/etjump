/*
 * MIT License
 * 
 * Copyright (c) 2020 ETJump team <haapanen.jussi@gmail.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef ROTATION_MATRIX_HPP
#define ROTATION_MATRIX_HPP

#include <cmath> 

#define PI 3.14159265

class RotationMatrix
{

public:

	typedef float vec_t;
	typedef vec_t vec3_t[3];

	void init()
	{
		_m11 = 0.0f; _m12 = 0.0f; _m13 = 0.0f; _m14 = 0.0f;
		_m21 = 0.0f; _m22 = 0.0f; _m23 = 0.0f; _m24 = 0.0f;
		_m31 = 0.0f; _m32 = 0.0f; _m33 = 0.0f; _m34 = 0.0f;
		_degree = 0.0f;
	}

	void rotateAlongAnAbitaryAxis(vec3_t center, vec3_t axis)
	{
		// Use centroid of the object of rotation origin.
		float a = center[0];
		float b = center[1];
		float c = center[2];

		// Use vector (0,0,1) as Z-Up-Vector for the rotation
		float u = axis[0];
		float v = axis[1];
		float w = axis[2];

		// Set some intermediate values.
		float u2 = u*u;
		float v2 = v*v;
		float w2 = w*w;
		float cosT = std::cos(_degree);
		float oneMinusCosT = 1.0f - cosT;
		float sinT = std::sin(_degree);

		// Build the matrix entries element by element.
		_m11 = u2 + (v2 + w2) * cosT;
		_m12 = u*v * oneMinusCosT - w*sinT;
		_m13 = u*w * oneMinusCosT + v*sinT;
		_m14 = (a*(v2 + w2) - u*(b*v + c*w))*oneMinusCosT + (b*w - c*v)*sinT;

		_m21 = u*v * oneMinusCosT + w*sinT;
		_m22 = v2 + (u2 + w2) * cosT;
		_m23 = v*w * oneMinusCosT - u*sinT;
		_m24 = (b*(u2 + w2) - v*(a*u + c*w))*oneMinusCosT + (c*u - a*w)*sinT;

		_m31 = u*w * oneMinusCosT - v*sinT;
		_m32 = v*w * oneMinusCosT + u*sinT;
		_m33 = w2 + (u2 + v2) * cosT;
		_m34 = (c*(u2 + v2) - w*(a*u + b*v))*oneMinusCosT + (a*v - b*u)*sinT;

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

	float getDegree() { return _degree; }
	void setDegree(float d) { _degree = d; }

private:
	float _m11, _m12, _m13, _m14;
	float _m21, _m22, _m23, _m24;
	float _m31, _m32, _m33, _m34;

	float _degree;

};
#endif