#pragma once
#include <string>
#include <float.h>
#include <stdexcept>
#include <math.h>
#include <algorithm>
#include <random>
#include <iostream>
#include <stdio.h>

#include "Vector.hpp"


#define M_PI 3.1415926535897

// lerp(x,v0,v1) = v0 + x(v1-v0);
// x is the portion
// v0 v1 are the values
float lerp(float x, float v0, float v1) {
	return v0 + x * (v1 - v0);
}


// check if str is convertable to a positive integer
// if there's a char other than 0 to 9
// then throw exeption
void checkPosInt(std::string& str) {
	for (auto i : str) {
		if (i < 48 || i > 57) {
			throw std::runtime_error(str + ": expect a positive number");
		}
	}
}

// check if str is convertable to a float
void checkFloat(std::string& str) {
	if (str.size() == 0) {
		throw std::runtime_error(str + ": not a valid float number");
	}

	bool dotAppeared = false;
	// for minus 
	if (str.at(0) == '-') {
		if(str.size()==1) throw std::runtime_error(str + ": not a valid float number");

		for (int i = 1; i < str.size(); i++) {
			// -abcdef   a can't be a char other than a number
			if (i == 1 && str[1] < 48 || str[i] > 57) throw std::runtime_error(str + ": not a valid float number");
			// for other index 
			else {
				// dot can only present once && can't be at the end of the str
				if (str[i] == '.' && !dotAppeared && i!=str.size()-1) {
					dotAppeared = true;		// 
				}
				else {	// expect numbers
					if(str[i] < 48 || str[i] > 57) throw std::runtime_error(str + ": not a valid float number");
				}
			}
		}
	}
	// for positive
	else {
		for (int i = 0; i < str.size(); i++) {
			// abcdef   a can't be a char other than a number
			if (i == 0 && str[0] < 48 || str[i] > 57) throw std::runtime_error(str + ": not a valid float number");
			// for other index 
			else {
				// dot can only present once && can't be at the end of the str
				if (str[i] == '.' && !dotAppeared && i != str.size() - 1) {
					dotAppeared = true;		// 
				}
				else {	// expect numbers
					if (str[i] < 48 || str[i] > 57) throw std::runtime_error(str + ": not a valid float number");
				}
			}
		}
	}

}

// convert degree to radians
float degree2Radians(const float& d) {
	return d * M_PI / 180.f;
}

// check if two float numbers are equal
bool FLOAT_EQUAL(const float& x, const float& y) {
	return (fabs(x - y) < 0.00001);
}

/// <summary
/// solve
/// A*t^2 + B*t + C = 0
/// </summary>
/// <param name="t1"> first solution  </param>
/// <param name="t2"> second solution	</param>
/// 
/// if there's only one solution, then either t1 or t2 == FLT_MAX
/// if there's no real solution, t1 == t2 == FLT_MAX
void solveQuadratic(float& t1, float& t2, float& A, float& B, float& C) {
	float discriminant = B * B - 4 * A * C;
	// no real solution
	if (discriminant < 0) {
		t1 = FLT_MAX;
		t2 = FLT_MAX;
	}
	// one real solution
	else if (discriminant == 0) {
		t1 = (-B + sqrtf(discriminant)) / 2 * A;
		t2 = t1;
	}
	// two real solution
	else {
		t1 = (-B + sqrtf(discriminant)) / 2 * A;
		t2 = (-B - sqrtf(discriminant)) / 2 * A;
	}
}



// check if a ele existing within a string vector
bool existIn(std::string& ele, std::vector<std::string>& v) {
	for (auto i : v) {
		if (ele.compare(i) == 0) return true;
	}
	return false;
}


// get a uniformly distributed number in range [i, j]
float getRandomFloat(float i, float j) {
	// see 
	// https://stackoverflow.com/questions/38367976/do-stdrandom-device-and-stdmt19937-follow-an-uniform-distribution

	// an uniformly - distributed random number generator, use it to seed a pseudo-random generator
	static std::random_device dev;
	// a fast pseudo-random number generator, use this to seed a particular distribution
	static std::mt19937 rng(dev());		
	static std::uniform_real_distribution<float> dist(i, j); // distribution in range [0.0, 1.0]

	return dist(rng);	
}

// cout to terminal the progress
void showProgress(float prog) {
	int barWidth = 60;

	int pos = barWidth * prog;
	for (int i = 0; i < barWidth; ++i) {
		if (i < pos) std::cout << "=";
		else if (i == pos) std::cout << ">";
		else std::cout << " ";
	}
	std::cout << int(prog * 100.0) << " %\r";
}


// safely get vec3f element in an vec3f array
Vector3f getEleIn(std::vector<Vector3f>& arr, int index) {
	if (index >= arr.size() || index < 0) {
		throw std::runtime_error(index + " is out of bound: array has size: " + arr.size());
	}
	return arr.at(index);
}

Vector2f getEleIn(std::vector<Vector2f>& arr, int index) {
	if (index >= arr.size() || index < 0) {
		throw std::runtime_error(index + " is out of bound: array has size: " + arr.size());
	}
	return arr.at(index);
}