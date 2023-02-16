#pragma once

#include<fstream>
#include<iostream>
#include<stdexcept>
#include<string>
#include<vector>
#include<cmath>

#include "Vector.hpp"
#include "global.hpp"
#include "Scene.hpp"
#include "Sphere.hpp"
#include "Triangle.hpp"
#include "material.hpp"

std::vector<std::string> objType = { "sphere", "triangle" };


/// <summary>
/// this class read a configuration file and it produces a output ppm file
/// </summary
class PPMGenerator {
public:
	// Constructors, check if the stream to file is correctly opened
	// and initialze fields
	PPMGenerator(const char * path)  {
		fin.open(path, std::ios_base::in);		// read only
		if (!fin.is_open()) {
			std::cout << "ERROR::file does not exits, program terminates.\n";
			// close stream and exit. exit WILL NOT call destructors
			fin.clear();
			fin.close();
			exit(-1);
		}
		
		inputName = path;
		// Read the contents and initialize fields
		initialize();
	}

	// Destructor:
	// close the stream to file
	~PPMGenerator() {
		if (fin.is_open()) {
			fin.clear();
			fin.close();
		}

		if (fout.is_open()) {
			fout.clear();
			fout.close();
		}
	}

	// generate the ppm file. 
	// write the rgb data into a file
	void generate() {

		std::string input(inputName);
		std::string outName;
		std::size_t pos = input.find(".txt");
		// if not find .txt or merely .txt    then generate xxx.ppm or .ppm
		if (pos == std::string::npos || pos == 0) {
			outName = std::string(inputName).append(".ppm");
			if (pos == 0) outName = std::string(".ppm");
		}
		else {
			outName = input.substr(0, pos).append(".ppm");
		}
		fout.open(outName);
		writeHeader();
		writePixel();

		std::cout << "Generating is done successfully!\n";
		fout.clear();
		fout.close();
	}


// ****************************************************** private *****************************************************
private:
	std::ifstream fin;
	std::ofstream fout;
	const char* inputName;
	std::vector<Vector3i> rgb;		// pixel data
	//------------------ reading data
	int width = -1;
	int height = -1;
	Vector3f eyePos = Vector3f(FLT_MAX,0,0);
	Vector3f viewdir = Vector3f(FLT_MAX,0,0);
	int hfov = -1;
	Vector3f updir = Vector3f(FLT_MAX, 0, 0);
	Vector3f bkgcolor = Vector3f(FLT_MAX, 0, 0);
	Scene scene;
	int parallel_projection = 0;  // 0 for perspective, 1 for orthographic
	int shadowType = 0;			 // 0 for hard shadow, 1 for soft shadow
	Material mtlcolor;			// temp buffer for material color	default 0 0 0
		// ******* depthcueing *******
		bool depthCueing = false;	// depthcueing flag
		Vector3f dc;				// depthcueing color
		float amin, amax, distmin, distmax;
		// ******* depthcueing ends ********
	//----------------- reading data ends
	friend class Renderer;


	// read the input file to initialize PPMGenerator
	// check if the input file is in the desired format:
	// imsize 512 256
	// eye 0 0 0
	// viewdir 0 0 -1
	// hfov 130
	// updir 0 1 1
	// bkgcolor 0.1 0.1 0.1
	// mtlcolor 0 1 0
	// sphere -0.5 -1 -8 3
	// mtlcolor 1 0 0
	// sphere 3 1 -3 1
	// if we can initialize our generator, it's fine (ignore the characters after height)
	// otherwise prompt ERROR and exit

	void initialize() {
		try {
			std::string keyWord;

			checkFin();
			while (!fin.eof()) {
				fin >> keyWord;
				processKeyword(keyWord);
			}

			// check if all the required information are set
			if (!isInitialized()) {
				throw std::runtime_error("insufficient input data: unable to initialize the program\n");
			}
			// check if view and up direction are the same
			if (floatEqual(viewdir.x, updir.x) && floatEqual(viewdir.y, updir.y) && floatEqual(viewdir.z, updir.z)) {
				throw std::runtime_error("invalid viewPlane infomation: updir and view dir can't be the same");
			}

			// initialize the rgb arrays
			rgb.resize(width * height);
			rgb.assign(width * height, Vector3i(255 * bkgcolor.x, 255 * bkgcolor.y, 255 * bkgcolor.z));	//  set to bkgcolor
		}

		catch (std::runtime_error e) {
			std::cout << "ERROR: " << e.what();
			fin.clear();
			fin.close();
			exit(-1);
		}
	}

	// recursively read object (if any) until end of file or any extraneous info appears
	// add the object into this->scene->objList
	void readObject(std::string & key) {
		int type = -1;		// 0 for triangle   1 for sphere
		if (key.compare("triangle") == 0) type = 0;
		if (key.compare("sphere") == 0) type = 1;


		switch (type)
		{
		case 0: {	// triangle 
			std::string t0, t1, t2, t3, t4, t5, t6, t7, t8, 
						t9, t10, t11 ,t12, t13, t14, t15, t16, t17;		// temp string buffer

			checkFin(); fin >> t0; checkFin(); fin >> t1; checkFin(); fin >> t2;
			checkFin(); fin >> t3; checkFin(); fin >> t4; checkFin(); fin >> t5;
			checkFin(); fin >> t6; checkFin(); fin >> t7; checkFin(); fin >> t8;
			checkFin(); fin >> t9;
			checkFin(); fin >> t10; checkFin(); fin >> t11; checkFin(); fin >> t12;
			checkFin(); fin >> t13; checkFin(); fin >> t14; checkFin(); fin >> t15;
			checkFin(); fin >> t16; checkFin(); fin >> t17; 
			checkFloat(t0); checkFloat(t1); checkFloat(t2); checkFloat(t3);
			checkFloat(t4); checkFloat(t5); checkFloat(t6); checkFloat(t7);
			checkFloat(t8); checkFloat(t9);
			checkFloat(t10); checkFloat(t11); checkFloat(t12); checkFloat(t13);
			checkFloat(t14); checkFloat(t15); checkFloat(t16); checkFloat(t17);
			
			// create triangle object
			std::unique_ptr<Triangle> s = std::make_unique<Triangle>();
			s->mtlcolor = mtlcolor;

			s->v0.x = std::stof(t0);
			s->v0.y = std::stof(t1);
			s->v0.z = std::stof(t2);
			s->v1.x = std::stof(t3);
			s->v1.y = std::stof(t4);
			s->v1.z = std::stof(t5);
			s->v2.x = std::stof(t6);
			s->v2.y = std::stof(t7);
			s->v2.z = std::stof(t8);
			s->n0.x = std::stof(t9);
			s->n0.y = std::stof(t10);
			s->n0.z = std::stof(t11);
			s->n1.x = std::stof(t12);
			s->n1.y = std::stof(t13);
			s->n1.z = std::stof(t14);
			s->n2.x = std::stof(t15);
			s->n2.y = std::stof(t16);
			s->n2.z = std::stof(t17);

			// if all attributes of mtlcolor is set to 
			// 1 1 1 1 1 1 1 1 1 0, then it is a light source
			if (floatEqual(1.f, s->mtlcolor.diffuse.x) && floatEqual(1.f, s->mtlcolor.diffuse.y)
				&& floatEqual(1.f, s->mtlcolor.diffuse.z) && floatEqual(1.f, s->mtlcolor.specular.x)
				&& floatEqual(1.f, s->mtlcolor.specular.y) && floatEqual(1.f, s->mtlcolor.specular.z)
				&& floatEqual(1.f, s->mtlcolor.ka) && floatEqual(1.f, s->mtlcolor.kd)
				&& floatEqual(1.f, s->mtlcolor.ks) && floatEqual(0.f, s->mtlcolor.n))
				s->isLight = true;

			scene.add(std::move(s));
			break;
		}
		case 1: {	//sphere
			std::string a, b, c, d;

			// create sphere object
			std::unique_ptr<Sphere> s = std::make_unique<Sphere>();
			s->mtlcolor = mtlcolor;

			checkFin(); fin >> a; checkFin(); fin >> b; checkFin(); fin >> c; checkFin(); fin >> d;
			checkFloat(a);
			checkFloat(b);
			checkFloat(c);
			checkFloat(d);
			s->centerPos.x = std::stof(a);
			s->centerPos.y = std::stof(b);
			s->centerPos.z = std::stof(c);
			s->radius = std::stof(d);

			// push it into scene.objList
			scene.add(std::move(s));
			break;
		}
		default:
			throw std::runtime_error("unknown object type, you should never reach this error\n");
			break;
		}
		
	}

	// row major, then index = y*width + x
	// note in ppm, upper left is (0,0)
	size_t getIndex(int x, int y) {
		return y * width + x;
	}

	// write ppm header part
	void writeHeader() {
		fout << "P3\n";
		fout << width << "\n";
		fout << height << "\n";
		fout << 255 << "\n";
	}

	// write the pixel rgb information
	void writePixel() {
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				size_t index = getIndex(j, i);
				fout << rgb[index].x << " " << rgb[index].y << " " << rgb[index].z << "\n";
			}
		}
	}
	/*
	the followings are the default value, return false if any of them 
	is still the default value

		size_t width = -1;
		size_t height = -1;
		Vector3f eyePos = Vector3f(FLT_MAX,0,0);
		Vector3f viewdir = Vector3f(FLT_MAX,0,0);
		int hfov = -1;
		Vector3f updir = Vector3f(FLT_MAX, 0, 0);
		Vector3f bkgcolor = Vector3f(FLT_MAX, 0, 0);
	*/
	bool isInitialized() {
		if (width == -1) return false;
		if (height == -1) return false;
		if (floatEqual(eyePos.x, FLT_MAX)) return false;
		if (floatEqual(viewdir.x, FLT_MAX)) return false;
		if (hfov == -1) return false;
		if (floatEqual(updir.x, FLT_MAX)) return false;
		if (floatEqual(bkgcolor.x, FLT_MAX)) return false;

		return true;
	}

	// helper func of void PPMGenerator::initialize();
	// process the keyword being read
	void processKeyword(std::string & key) {
		std::string  a, b, c, d;		// reading buffer data

		// imsize
		if (key.compare("imsize") == 0) {
			checkFin(); fin >> a; checkFin(); fin >> b;
			checkPosInt(a);
			width = std::stoi(a);
			checkPosInt(b);
			height = std::stoi(b);
		}
		// eye
		else if (key.compare("eye") == 0) {
			checkFin(); fin >> a; checkFin(); fin >> b; checkFin();fin >> c;
			checkFloat(a);
			checkFloat(b);
			checkFloat(c);
			eyePos.x = std::stof(a);
			eyePos.y = std::stof(b);
			eyePos.z = std::stof(c);
		}
		// viewdir
		else if (key.compare("viewdir") == 0) {
			checkFin(); fin >> a; checkFin(); fin >> b; checkFin(); fin >> c;
			checkFloat(a);
			checkFloat(b);
			checkFloat(c);
			viewdir.x = std::stof(a);
			viewdir.y = std::stof(b);
			viewdir.z = std::stof(c);
		}
		// hfov
		else if (key.compare("hfov") == 0) {
			
			checkFin(); fin >> a;
			checkPosInt(a);
			hfov = std::stoi(a);
		}
		// updir
		else if (key.compare("updir") == 0) {
			checkFin(); fin >> a; checkFin(); fin >> b; checkFin(); fin >> c;
			checkFloat(a);
			checkFloat(b);
			checkFloat(c);
			updir.x = std::stof(a);
			updir.y = std::stof(b);
			updir.z = std::stof(c);
		}
		// bkgcolor 
		else if (key.compare("bkgcolor") == 0) {
			checkFin(); fin >> a; checkFin(); fin >> b; checkFin(); fin >> c;
			checkFloat(a);
			checkFloat(b);
			checkFloat(c);
			bkgcolor.x = std::stof(a);
			bkgcolor.y = std::stof(b);
			bkgcolor.z = std::stof(c);
		}
		// enhancement: read projection config
		else if (key.compare("projection") == 0) {
			checkFin(); fin >> a;
			if (a.compare("parallel") == 0) {
				parallel_projection = 1;
			}
		}
		// deal with light without attenuation
		else if (key.compare("light") == 0) {
			std::string t0, t1, t2, t3, t4, t5, t6;
			checkFin(); fin >> t0; checkFin(); fin >> t1; checkFin(); fin >> t2;
			checkFin(); fin >> t3; checkFin(); fin >> t4; checkFin(); fin >> t5;
			checkFin(); fin >> t6;
			checkFloat(t0); checkFloat(t1); checkFloat(t2); checkFloat(t3);
			checkFloat(t4); checkFloat(t5); checkFloat(t6);

			// create light object
			std::unique_ptr<Light> l = std::make_unique<Light>();
			l->pos.x = std::stof(t0);
			l->pos.y = std::stof(t1);
			l->pos.z = std::stof(t2);
			l->pos.w = std::stof(t3);
			l->color.x = std::stof(t4);
			l->color.y = std::stof(t5);
			l->color.z = std::stof(t6);

			l->intialize();
			scene.add(std::move(l));
		}

		// deal with attenuation light 
		else if (key.compare("attlight") == 0) {
			std::string t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;		// temp string buffer

			checkFin(); fin >> t0; checkFin(); fin >> t1; checkFin(); fin >> t2;
			checkFin(); fin >> t3; checkFin(); fin >> t4; checkFin(); fin >> t5;
			checkFin(); fin >> t6; checkFin(); fin >> t7; checkFin(); fin >> t8;
			checkFin(); fin >> t9;

			checkFloat(t0); checkFloat(t1); checkFloat(t2); checkFloat(t3);
			checkFloat(t4); checkFloat(t5); checkFloat(t6); checkFloat(t7);
			checkFloat(t8); checkFloat(t9);

			// create light object
			std::unique_ptr<Light> l = std::make_unique<Light>();
			l->pos.x = std::stof(t0);
			l->pos.y = std::stof(t1);
			l->pos.z = std::stof(t2);
			l->pos.w = std::stof(t3);
			l->color.x = std::stof(t4);
			l->color.y = std::stof(t5);
			l->color.z = std::stof(t6);
			l->c1 = std::stof(t7);
			l->c2 = std::stof(t8);
			l->c3 = std::stof(t9);

			l->intialize();
			scene.add(std::move(l));
		}

		// process mtlcolor
		else if (key.compare("mtlcolor") == 0) {
			std::string t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;		// temp string buffer

			checkFin(); fin >> t0; checkFin(); fin >> t1; checkFin(); fin >> t2;
			checkFin(); fin >> t3; checkFin(); fin >> t4; checkFin(); fin >> t5;
			checkFin(); fin >> t6; checkFin(); fin >> t7; checkFin(); fin >> t8;
			checkFin(); fin >> t9; 

			checkFloat(t0); checkFloat(t1);checkFloat(t2); checkFloat(t3);
			checkFloat(t4); checkFloat(t5); checkFloat(t6); checkFloat(t7);
			checkFloat(t8); checkFloat(t9); 

			// set mtlcolor
			mtlcolor.diffuse.x = std::stof(t0);
			mtlcolor.diffuse.y = std::stof(t1);
			mtlcolor.diffuse.z = std::stof(t2);

			mtlcolor.specular.x = std::stof(t3);
			mtlcolor.specular.y = std::stof(t4);
			mtlcolor.specular.z = std::stof(t5);

			mtlcolor.ka = std::stof(t6);
			mtlcolor.kd = std::stof(t7);
			mtlcolor.ks = std::stof(t8);
			mtlcolor.n = std::stof(t9);
		}

		// read object
		else if (existIn(key, objType)) {
			readObject(key);
		}
		// read shadow config
		else if (key.compare("shadow") == 0) {
			checkFin(); fin >> a;
			if (a.compare("soft") == 0) {
				shadowType = 1;
			}
		}
		// depthcueing flag
		else if (key.compare("depthcueing") == 0) {
		depthCueing = true;

		std::string t0, t1, t2, t3, t4, t5, t6;
		checkFin(); fin >> t0; checkFin(); fin >> t1; checkFin(); fin >> t2;
		checkFin(); fin >> t3; checkFin(); fin >> t4; checkFin(); fin >> t5;
		checkFin(); fin >> t6;
		checkFloat(t0); checkFloat(t1); checkFloat(t2); checkFloat(t3);
		checkFloat(t4); checkFloat(t5); checkFloat(t6);

		dc.x = std::stof(t0);
		dc.y = std::stof(t1);
		dc.z = std::stof(t2);

		// amax, amin distmin, distmax;
		amax = std::stof(t3);
		amin = std::stof(t4);
		distmax = std::stof(t5);
		distmin = std::stof(t6);
		}
		else {
			throw std::runtime_error("extraneous string in the input file\n");
		}
	}

	// check if fin has reached eof
	void checkFin() {
		if (fin.eof()) {
			throw std::runtime_error("Insufficient or invalid data as input, check your config file\n");
		}
	}

};