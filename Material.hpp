#pragma once

#include "global.hpp"
#include "Vector.hpp"

class Material {
public:
	Vector3f diffuse;
	Vector3f specular;
	float ka = 0;;
	float kd = 0;
	float ks = 0;
	float n = 0;			// highlights shininess power coefficient

	float alpha;			// opacity 
	float eta;				// index of refraction


	// copy assignment operator
	Material& operator= (const Material& other) {
		if (this != &other) {
			this->diffuse.x = other.diffuse.x;
			this->diffuse.y = other.diffuse.y;
			this->diffuse.z = other.diffuse.z;
			this->specular.x = other.specular.x;
			this->specular.y = other.specular.y;
			this->specular.z = other.specular.z;

			this->ka = other.ka;
			this->kd = other.kd;
			this->ks = other.ks;
			this->n = other.n;

			this->alpha = other.alpha;
			this->eta = other.eta;
		}
		return *this;
	}
};