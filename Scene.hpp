#pragma once

#include <vector>
#include <memory>	// smart pointer

#include "Vector.hpp"
#include "Object.hpp"
#include "Light.hpp"


class Scene {
public:
	// use pointer array instead of "object" array for 
	// unique_ptr<T>'s declared move constructor
	// move instead of copy for efficiency
	std::vector<std::unique_ptr<Object>> objList;
	std::vector<std::unique_ptr<Light>> lightList;

	// add object into object list
	void add(std::unique_ptr<Object> obj) {
		objList.emplace_back(std::move(obj));
	}

	// add light into light list
	void add(std::unique_ptr<Light> light) {
		lightList.emplace_back(std::move(light));
	}
};