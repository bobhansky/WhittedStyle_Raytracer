#pragma once

#include "Object.hpp"

class Triangle : public Object{
public:
	// 3 vertices, from lower left counterclockwise 
	Vector3f v0;
	Vector3f v1;
	Vector3f v2;

	// normal direction of 3 vertices
	Vector3f n0, n1, n2;


	// Using Moller Trumbore Algorithm to update the intersection between ray and triangle
	bool intersect(const Vector3f& orig, const Vector3f& dir, Intersection& inter) override {
		Vector3f E1 = v1 - v0;
		Vector3f E2 = v2 - v0;          // v2 - v1   get the strange res
		Vector3f S = orig - v0;
		Vector3f S1 = crossProduct(dir, E2);
		Vector3f S2 = crossProduct(S, E1);

		Vector3f rightVec(S2.dot(E2), S1.dot(S),S2.dot(dir));	// right handside part in my note
		float left = 1.0f / S1.dot(E1);

		Vector3f res = left * rightVec;		// t u v are in res now

		const float EPSILON = 0.00001;    // float number comparison
		// if t > 0 && (1-b1-b2) > 0 &&  b1 > 0 && b2 >0 
		// positive tnear and the intersection point can be represented as 
		// barycentric coordinate INSIDE of this triangle
		// then ray and triangle intersect with each others
		if (res.x + EPSILON > 0 && 1 - res.y - res.z + EPSILON > 0 && res.y + EPSILON > 0 && res.z + EPSILON > 0) {
			// then update intersection
			inter.intersected = true;
			inter.obj = this;
			inter.t = res.x;
			inter.pos = orig + inter.t * dir;
			inter.mtlcolor = this->mtlcolor;
			// also get the interpolated normal direction
			inter.u = res.y;
			inter.v = res.z;
			inter.nDir = (n0 * (1 - res.y - res.z)) + n1 * res.y + n2 * res.z;

			return true;
		}
		return false;
	}

};