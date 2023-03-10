#include "PPMGenerator.hpp"
#include "Sphere.hpp"
#include "Scene.hpp"
#include "Object.hpp"
#include "Renderer.hpp"

#include <string>
#include <chrono>


int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cout << "ERROR: lack of the input configuration file, please provide its path as the first argument.\n";
		return 0;
	}

	auto start = std::chrono::system_clock::now();		// #include <chrono>

	PPMGenerator g(argv[1]);
	
	Renderer r(&g);
	r.render();
	g.generate();

	auto end = std::chrono::system_clock::now();
	std::cout << "\nTime consumed: \n";
	std::cout<< std::chrono::duration_cast<std::chrono::seconds>(end - start).count() << " seconds\n";
	return 0;
}