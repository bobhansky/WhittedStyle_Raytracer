#include "global.hpp"
#include "Vector.hpp"

#include <vector>


class Texture {
public:
	std::string name;
	int width = 0;
	int height = 0;
	bool isActivated = false;			// only useful in reading input file, use this state to initialize the object
										// object's isTextureActivated is on if it is on
	std::vector<Vector3f> rgb;			// texture rgb, each in range [0, 1]

	Vector3f getRGBat(float u, float v) {
		if (width == 0 && height == 0) {
			return Vector3f();
		}

		int x = u * width;
		int y = v * height;
		return getEleIn(rgb, y * width + x);
	}
};