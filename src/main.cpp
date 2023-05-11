#include "../include/PPMGenerator.hpp"
#include "../include/Sphere.hpp"
#include "../include/Scene.hpp"
#include "../include/Object.hpp"
#include "../include/Renderer.hpp"
#include "../include/OBJ_Loader.h"

#include <string>
#include <chrono>


int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cout << "ERROR: lack of the input configuration file, please provide its path as the first argument.\n";
		return 0;
	}



	PPMGenerator g(argv[1]);

	Material floor_mtl;

	floor_mtl.diffuse = { 0.529, 0.807, 0.921 };
	floor_mtl.specular = { 0.33, 0.66, 0.99 };
	floor_mtl.ka = 0.05;
	floor_mtl.kd = 0.1;
	floor_mtl.ks = 0;
	floor_mtl.n = 64;
	floor_mtl.alpha = 0.2;
	floor_mtl.eta = 1.52;

	// glass
	/*
	floor_mtl.diffuse = { 0.529, 0.807, 0.921 };
	floor_mtl.specular = { 0.33, 0.66, 0.99 };
	floor_mtl.ka = 0.05;
	floor_mtl.kd = 0.1;
	floor_mtl.ks = 0.2;
	floor_mtl.n = 64;
	floor_mtl.alpha = 0.2;
	floor_mtl.eta = 1.33;
	*/

	
	objl::Loader floor;
	if (floor.LoadFile("bunny.obj")) {
		for (auto &i : floor.LoadedMeshes) {
			for (auto &j : i.Vertices) {
				j.Position = j.Position * 17;
			}
		}
		g.loadObj(floor,floor_mtl, -1, -1);
	}
	
	Renderer r(&g);
	auto start = std::chrono::system_clock::now();		// #include <chrono>
	r.render();
	g.generate();


	auto end = std::chrono::system_clock::now();
	std::cout << "\nRendering Time consumed: \n";
	std::cout<< std::chrono::duration_cast<std::chrono::seconds>(end - start).count() << " seconds\n";
	return 0;
}