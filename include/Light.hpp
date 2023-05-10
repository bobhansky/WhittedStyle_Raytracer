#pragma once

#include "global.hpp"
#include "Vector.hpp"
#include "Triangle.hpp"


class Light {
public:
	Vector4f pos;	// x y z w		w==0: directional light	  w==1: point light
	Vector3f color;
	float c1 = -1, c2 = -1, c3 = -1;	// attenuation coefficient set to negative means no attenuation 
										// plz check if negative before use attenuation

	Triangle triangle;		// area light, set two adjacent side to 1 length


	/*
	initialize area light triangle, this should be called every time after an instance is created.
	|---------					two adjacent side are alwayse 1 length
	|       /					pos are at the barycentric center of this triangle
	|  `  /
	|  /
	|/ 

	now define this triangle
	*/
	void intialize() {
		// do not initialize if this light is a diretional light
		if (FLOAT_EQUAL(pos.w, 0.f)) return;

		triangle.v0 = Vector3f(pos.x, pos.y, pos.z);
		triangle.v1 = Vector3f(pos.x+1, pos.y, pos.z-1);
		triangle.v2 = Vector3f(pos.x, pos.y, pos.z - 1);

		Vector3f center = 0.33333 * triangle.v0 + 0.33333 * triangle.v1 + 0.33333 * triangle.v2;
		Vector3f offset = triangle.v0 - center;		// light position - center, the offset for center to get the light pos
		
		triangle.v0 = triangle.v0 + offset;
		triangle.v1 = triangle.v1 + offset;
		triangle.v2 = triangle.v2 + offset;
	}
};