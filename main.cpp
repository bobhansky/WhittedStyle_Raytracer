#include "PPMGenerator.hpp"
#include "Sphere.hpp"
#include "Scene.hpp"
#include "Object.hpp"
#include "Renderer.hpp"

#include <string>
#include <regex>




int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cout << "ERROR: lack of the input configuration file, please provide its path as the first argument.\n";
		return 0;
	}

	PPMGenerator g(argv[1]);
	
	Renderer r(&g);
	r.render();
	g.generate();

	return 0;
}