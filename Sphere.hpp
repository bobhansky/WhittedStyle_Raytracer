#pragma once

#include "Object.hpp"

class Sphere : public Object{
public:
	Vector3f centerPos;		// center position
	float radius;

	Sphere() {
		centerPos = Vector3f(0.f, 0.f, 0.f);
		radius = 1.f;
		
	}

	Sphere(float x, float y, float z, float r) {
		centerPos = Vector3f(x, y, z);
		radius = r;

	}

	virtual ~Sphere() {};

	// check if the ray will intersect with this sphere or not
	// if true, then set the nearest time of intersection as tNear, and mtlColor
	bool intersect(const Vector3f& orig, const Vector3f& dir, Intersection& inter) override {
		float A = 1.f;	// here is 1 since we are using normalized vector
		float B = 2 * (dir.x * (orig.x - centerPos.x) + dir.y * (orig.y - centerPos.y)
			+ dir.z * (orig.z - centerPos.z));
		float C = pow((orig.x - centerPos.x), 2) + pow((orig.y - centerPos.y), 2)
			+ pow((orig.z - centerPos.z), 2) - radius * radius;

		float t1 = 0;
		float t2 = 0;
		solveQuadratic(t1, t2, A, B, C);

		inter.intersected = false;
		// miss, no real solution
		if (floatEqual(t1, FLT_MAX) && floatEqual(t2, FLT_MAX)) {
			return false;
		}
		// one soluion
		else if (floatEqual(t1, t2)) {
			// intersection is behind the ray direction, then false
			if (t1 < 0) return false;
			else {
				inter.t = t1;
				// update intersection data
				inter.intersected = true;
				inter.obj = this;
				inter.mtlcolor = this->mtlcolor;
				inter.pos = orig + inter.t * dir;
				inter.nDir = normalized(inter.pos - centerPos);
				return true;
			}

		}
		// two solution
		else {
			if (t1 >= 0 && t2 >= 0) {
				inter.t = fmin(t1, t2);

			}
			else if (t1 >= 0 && t2 < 0) {
				inter.t = t1;
			}
			else if (t1 < 0 && t2 >= 0) {
				inter.t = t2;
			}
			else return false;

			// update intersection data
			inter.intersected = true;
			inter.obj = this;
			inter.mtlcolor = this->mtlcolor;
			inter.pos = orig + inter.t * dir;
			inter.nDir = normalized(inter.pos - centerPos);
			return true;
		}

		return false;
	}
};