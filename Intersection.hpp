#pragma once

#include <float.h>

#include "Object.hpp"
#include "global.hpp"
#include "Vector.hpp"
#include "Material.hpp"


class Object;	// circular dependency issue
				// see https://stackoverflow.com/questions/23283080/compiler-error-c4430-missing-type-specifier-int-assumed

class Intersection {
public:

	bool intersected = false;
	float t = FLT_MAX;	// pos = rayPos + t * rayDir
	Vector3f pos;
	Vector3f nDir; // normal direction
	Material mtlcolor;
	Object *obj = nullptr;		// this intersection is on which object	


	// only for triangle
	// barycentric coordinates of the intersected point
	float u = 0;
	float v = 0;
};