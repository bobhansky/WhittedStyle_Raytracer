#pragma once

#include<fstream>
#include<iostream>
#include<stdexcept>
#include<string>
#include<vector>
#include<cmath>
#include <regex>

#include "Vector.hpp"
#include "global.hpp"
#include "Scene.hpp"
#include "Sphere.hpp"
#include "Triangle.hpp"
#include "Material.hpp"
#include "Texture.hpp"

// sphere vertex face vertex_normal vertex_texture
std::vector<std::string> objType = { "sphere", "v", "f", "vn", "vt"};


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
	std::vector<Vector3i> rgb;			// pixel data
	std::vector<Texture> textures;		// texture data
	std::vector<Texture> normalMaps;    // normalMap array
	//------------------ reading data
	int width = -1;
	int height = -1;
	Vector3f eyePos = Vector3f(FLT_MAX,0,0);
	Vector3f viewdir = Vector3f(FLT_MAX,0,0);
	int hfov = -1;
	Vector3f updir = Vector3f(FLT_MAX, 0, 0);
	Vector3f bkgcolor = Vector3f(FLT_MAX, 0, 0);
	float eta;							// index of refraction of this scene
	Scene scene;
	std::vector<Vector3f> vertices;		// triangle vertex array
	std::vector<Vector3f> normals;		// vertex normal array
	std::vector<Vector2f> textCoords;   // texture coordinates array

	bool isTextureOn = false;
	int textIndex = -1;					// texture index, always points to the activated texture in the textures array
	int bumpIndex = -1;					// normal map index, always points to the activated normal Map in the normalMap array
										// in input file, write "bump ..." once then activate it once, -1 means inactivated

	int parallel_projection = 0;  // 0 for perspective, 1 for orthographic
	int shadowType = 0;			  // 0 for hard shadow, 1 for soft shadow
	Material mtlcolor;			  // temp buffer for material color	default 0 0 0
		// ******* depthcueing *******
		bool depthCueing = false;	// depthcueing flag
		Vector3f dc;				// depthcueing color
		float amin, amax, distmin, distmax;
		// ******* depthcueing ends ********
	//----------------- reading data ends
	friend class Renderer;

	/// <summary>
	/// read the input file recursively and initialize the essential data for 
	/// viewing, check if all the viewing settings are initialized
	/// </summary>
	void initialize() {
		try {
			std::string keyWord;

			checkFin();
			fin >> keyWord;
			while (!fin.eof()) {
				processKeyword(keyWord);
				fin >> keyWord;
			}

			// check if all the required information are set
			if (!isInitialized()) {
				throw std::runtime_error("insufficient input data: unable to initialize the program\n");
			}
			// check if view and up direction are the same
			if (FLOAT_EQUAL(viewdir.x, updir.x) && FLOAT_EQUAL(viewdir.y, updir.y) && FLOAT_EQUAL(viewdir.z, updir.z)) {
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
		// 0 for v(vertex)   1 for sphere    2 for f face (triangle)
		// 3 for vertex normal		4 for texture coordinate
		int type = -1;		
		if (key.compare("v") == 0) type = 0;
		else if (key.compare("sphere") == 0) type = 1;
		else if (!key.compare("f")) 
			type = 2;
		else if (!key.compare("vn")) type = 3;
		else if (!key.compare("vt")) type = 4;

		std::string t0, t1, t2, t3;	// temp string buffer

		switch (type)
		{
		case 0: {	// v vertex 

			checkFin(); fin >> t0; checkFin(); fin >> t1; checkFin(); fin >> t2;
			
			//checkFloat(t0); checkFloat(t1); checkFloat(t2); 
			
			Vector3f vertex;
			vertex.x = std::stof(t0);
			vertex.y = std::stof(t1);
			vertex.z = std::stof(t2);

			vertices.emplace_back(vertex);
			break;
		}
		case 1: {	//sphere

			// create sphere object
			std::unique_ptr<Sphere> s = std::make_unique<Sphere>();
			s->mtlcolor = mtlcolor;

			checkFin(); fin >> t0; checkFin(); fin >> t1; checkFin(); fin >> t2; checkFin(); fin >> t3;
			checkFloat(t0);
			checkFloat(t1);
			checkFloat(t2);
			checkFloat(t3);
			s->centerPos.x = std::stof(t0);
			s->centerPos.y = std::stof(t1);
			s->centerPos.z = std::stof(t2);
			s->radius = std::stof(t3);
			s->objectType = OBJTYPE::SPEHRE;

			// see if texure is enable
			if (isTextureOn) {
				s->isTextureActivated = true;
				s->textureIndex = textIndex;	// set the texture index


				// use the normaltexture
				if (bumpIndex != -1) {
					s->normalMapIndex = bumpIndex;
					bumpIndex = -1;
				}
			}

			// push it into scene.objList
			scene.add(std::move(s));
			break;
		}
		case 2: {	// face/triangle
			// several cases:
			// flat shaded triangle:   f v1 v2  v3		
			// smooth shaded triangle: f   v1//vn1    v2//vn2    v3//vn3
			// flat-shaded textured triangle: f   v1/vt1   v2/vt2   v3/vt3
			// smooth-shaded textured triangle: f   v1/vt1/vn1     v2/vt2/vn2   v3/vt3/vn3
			checkFin(); fin >> t0; checkFin(); fin >> t1; checkFin(); fin >> t2;

			// regular expression, see my google doc CPP
			std::regex flat("[0-9]+");
			std::regex smooth("[0-9]+//[0-9]+");
			std::regex flat_text("[0-9]+/[0-9]+");
			std::regex smooth_text("[0-9]+/[0-9]+/[0-9]+");

			Triangle t;
			t.mtlcolor = mtlcolor;
			// match t0 t1 t2 and see which case the input is
			if (std::regex_match(t0, flat) && std::regex_match(t1, flat) && std::regex_match(t2, flat))
				processFlat(t0, t1, t2, t);
			else if (std::regex_match(t0, smooth) && std::regex_match(t1, smooth) && std::regex_match(t2, smooth))
				processSmooth(t0, t1, t2, t);
			else if (std::regex_match(t0, smooth_text) && std::regex_match(t0, smooth_text)
				&& std::regex_match(t0, smooth_text))
				processSmoothText(t0, t1, t2, t);
			else if (std::regex_match(t0, flat_text) && std::regex_match(t0, flat_text)
				&& std::regex_match(t0, flat_text))
				processFlatText(t0, t1, t2, t);
			else throw std::runtime_error("f face information is not valid");



			std::unique_ptr<Triangle> s = std::make_unique<Triangle>(t);
			// if all attributes of mtlcolor is set to 
			// 1 1 1 1 1 1 1 1 1 0, then it is a light source/light avatar
			if (FLOAT_EQUAL(1.f, s->mtlcolor.diffuse.x) && FLOAT_EQUAL(1.f, s->mtlcolor.diffuse.y)
				&& FLOAT_EQUAL(1.f, s->mtlcolor.diffuse.z) && FLOAT_EQUAL(1.f, s->mtlcolor.specular.x)
				&& FLOAT_EQUAL(1.f, s->mtlcolor.specular.y) && FLOAT_EQUAL(1.f, s->mtlcolor.specular.z)
				&& FLOAT_EQUAL(1.f, s->mtlcolor.ka) && FLOAT_EQUAL(1.f, s->mtlcolor.kd)
				&& FLOAT_EQUAL(1.f, s->mtlcolor.ks) && FLOAT_EQUAL(0.f, s->mtlcolor.n))
				s->isLight = true;

			// see if texure is enable
			if (isTextureOn) {
				s->isTextureActivated = true;
				s->textureIndex = textIndex;	// set the texture index

				// use the normaltexture
				if (bumpIndex != -1) {
					s->normalMapIndex = bumpIndex;
					bumpIndex = -1;
				}
			}
			s->objectType = OBJTYPE::TRIANGLE;

			scene.add(std::move(s));
			break;
		}
		case 3: {	// normal vertex		vn
			checkFin(); fin >> t0; checkFin(); fin >> t1; checkFin(); fin >> t2;

			checkFloat(t0); checkFloat(t1); checkFloat(t2);

			Vector3f normal;
			normal.x = std::stof(t0);
			normal.y = std::stof(t1);
			normal.z = std::stof(t2);

			normals.emplace_back(normalized(normal));
			break;
		}

		case 4: { // texture coordinates    vt
			checkFin(); fin >> t0; checkFin(); fin >> t1;
			checkFloat(t0); checkFloat(t1);
			Vector2f uv(std::stof(t0), std::stof(t1));

			textCoords.emplace_back(uv);
			break;
		}

		default:
			throw std::runtime_error("unknown object type, you should never reach this error\n");
			break;
		}
		
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

			checkFin(); fin >> a;
			checkFloat(a);
			eta = std::stof(a);
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
			// mtlcolor Odr Odg Odb Osr Osg Osb ka kd ks n 
			std::string t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;		// temp string buffer
			std::string t10, t11;									// alpha eta

			checkFin(); fin >> t0; checkFin(); fin >> t1; checkFin(); fin >> t2;
			checkFin(); fin >> t3; checkFin(); fin >> t4; checkFin(); fin >> t5;
			checkFin(); fin >> t6; checkFin(); fin >> t7; checkFin(); fin >> t8;
			checkFin(); fin >> t9; checkFin(); fin >> t10; checkFin(); fin >> t11;

			checkFloat(t0); checkFloat(t1);checkFloat(t2); checkFloat(t3);
			checkFloat(t4); checkFloat(t5); checkFloat(t6); checkFloat(t7);
			checkFloat(t8); checkFloat(t9); checkFloat(t10); checkFloat(t11);

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

			mtlcolor.alpha = std::stof(t10);
			mtlcolor.eta = std::stof(t11);

			isTextureOn = false;		// do not use texture data as Object diffuse term
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

		// color texture
		else if (!key.compare("texture")) {
			int size0 = textures.size();
			checkFin(); fin >> a;
			loadTexture(a.c_str(), textures);
			isTextureOn = true;		// replace mtlcolor's diffuse term with texture data

			int size1 = textures.size();
			// if a(the name of the texture) is loaded before
			// then point the texture index to it in the array
			if (size0 == size1) {
				for (int i = 0; i < textures.size(); i++) {
					if (!textures.at(i).name.compare(a)) {
						textIndex = i;
						break;
					}
				}
			}
			else textIndex = size1 - 1;
		}
		
		// normal map
		else if (!key.compare("bump")) {
			int size0 = normalMaps.size();
			checkFin(); fin >> a;
			loadTexture(a.c_str(), normalMaps);
			isTextureOn = true;		// replace mtlcolor's diffuse term with texture data

			int size1 = normalMaps.size();
			// if a(the name of the texture) is loaded before
			// then point the texture index to it in the array
			if (size0 == size1) {
				for (int i = 0; i < normalMaps.size(); i++) {
					if (!normalMaps.at(i).name.compare(a)) {
						bumpIndex = i;
						break;
					}
				}
			}
			else {
				bumpIndex = size1 - 1;

				// recover to requiered format (tangent plane)
				for (int i = 0; i < normalMaps[bumpIndex].rgb.size();i++) {
					Vector3f& c = normalMaps[bumpIndex].rgb[i];
					c = c * 2.f;
					c.x = c.x - 1.f;
					c.y = c.y - 1.f;
					c.z = c.z - 1.f;
				}
			}
		}

		// read object
		else if (existIn(key, objType)) {
			readObject(key);
		}

		else {
			throw std::runtime_error("extraneous string in the input file\n");
		}
	}


	// *************************************** helper functions ***************************************

	// get the rgb array index
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
		if (FLOAT_EQUAL(eyePos.x, FLT_MAX)) return false;
		if (FLOAT_EQUAL(viewdir.x, FLT_MAX)) return false;
		if (hfov == -1) return false;
		if (FLOAT_EQUAL(updir.x, FLT_MAX)) return false;
		if (FLOAT_EQUAL(bkgcolor.x, FLT_MAX)) return false;

		return true;
	}

	// check if fin has reached eof
	void checkFin() {
		if (fin.eof()) {
			throw std::runtime_error("Insufficient or invalid data as input, check your config file\n");
		}
	}

	// if the input format is flat shaded triangle, process it this way
	// v1 v2 v3
	void processFlat(std::string& t1, std::string& t2, std::string t3, Triangle& t) {
		int index = std::stoi(t1) - 1;
		t.v0 = getEleIn(vertices, index);

		index = std::stoi(t2) - 1;
		t.v1 = getEleIn(vertices, index);

		index = std::stoi(t3) - 1;
		t.v2 = getEleIn(vertices, index);

		// the normal of 3 vertices are the same:
		Vector3f e1 = t.v1 - t.v0;
		Vector3f e2 = t.v2 - t.v0;
		Vector3f normal = normalized(crossProduct(e1, e2));
		t.n0 = normal;
		t.n1 = normal;
		t.n2 = normal;
	}

	// if the input format is smooth shaded triangle, process it this way
	// v1//n1 v2//n2 v3//n3
	void processSmooth(std::string& t1, std::string& t2, std::string t3, Triangle& t) {
		int iv;
		int in;
		std::string verts[3] = {t1,t2,t3};
		for (int i = 0; i < 3; i++) {
			std::string sub;
			int pos = 0;
			int start = 0;
			pos = verts[i].find("//", pos);
			sub = verts[i].substr(start, pos - start);
			iv = std::stoi(sub);

			start = pos + 2;
			pos++;

			pos = verts[i].find("//", pos);
			sub = verts[i].substr(start, pos - start);
			in = std::stoi(sub);

			if (i == 0) {
				t.v0 = getEleIn(vertices, iv - 1);
				t.n0 = getEleIn(normals, in - 1);
			}
			else if (i == 1) {
				t.v1 = getEleIn(vertices, iv - 1);
				t.n1 = getEleIn(normals, in - 1);
			}
			else {
				t.v2 = getEleIn(vertices, iv - 1);
				t.n2 = getEleIn(normals, in - 1);
			}
		}
	}

	// f   v1/vt1   v2/vt2   v3/vt3
	void processFlatText(std::string& t1, std::string& t2, std::string t3, Triangle& t) {
		int iv;
		int it;
		std::string verts[3] = { t1,t2,t3 };

		for (int i = 0; i < 3; i++) {
			std::string sub;
			int pos = 0;
			int start = 0;
			pos = verts[i].find("/", pos);
			sub = verts[i].substr(start, pos - start);
			iv = std::stoi(sub);

			start = pos + 1;


			sub = verts[i].substr(start);
			it = std::stoi(sub);


			if (i == 0) {
				t.v0 = getEleIn(vertices, iv - 1);
				t.uv0 = getEleIn(textCoords, it - 1);
			}
			else if (i == 1) {
				t.v1 = getEleIn(vertices, iv - 1);
				t.uv1 = getEleIn(textCoords, it - 1);
			}
			else {
				t.v2 = getEleIn(vertices, iv - 1);
				t.uv2 = getEleIn(textCoords, it - 1);
			}
		}
		// set normal: all 3 vertices are the same
		// the normal of 3 vertices are the same:
		Vector3f e1 = t.v1 - t.v0;
		Vector3f e2 = t.v2 - t.v0;
		Vector3f normal = normalized(crossProduct(e1, e2));
		t.n0 = normal;
		t.n1 = normal;
		t.n2 = normal;
	}
		

	// if the input format is smooth shaded texture triangle, process it this way
	// f   v1/vt1/vn1     v2/vt2/vn2   v3/vt3/vn3
	void processSmoothText(std::string& t1, std::string& t2, std::string t3, Triangle& t) {
		int iv;
		int in;
		int it;
		std::string verts[3] = { t1,t2,t3 };
		for (int i = 0; i < 3; i++) {
			std::string sub;
			int pos = 0;
			int start = 0;
			pos = verts[i].find("/", pos);
			sub = verts[i].substr(start, pos - start);
			iv = std::stoi(sub);

			start = pos + 1;
			pos++;

			pos = verts[i].find("/", pos);
			sub = verts[i].substr(start, pos - start);
			it = std::stoi(sub);

			start = pos + 1;
			pos++;

			pos = verts[i].find("/", pos);
			sub = verts[i].substr(start, pos - start);
			in = std::stoi(sub);
			if (i == 0) {
				t.v0 = getEleIn(vertices, iv - 1);
				t.n0 = getEleIn(normals, in - 1);
				t.uv0 = getEleIn(textCoords, it - 1);
			}
			else if (i == 1) {
				t.v1 = getEleIn(vertices, iv - 1);
				t.n1 = getEleIn(normals, in - 1);
				t.uv1 = getEleIn(textCoords, it - 1);
			}
			else {
				t.v2 = getEleIn(vertices, iv - 1);
				t.n2 = getEleIn(normals, in - 1);
				t.uv2 = getEleIn(textCoords, it - 1);
			}
		}
	}


	// read the file "name" and store it's ascii ppm data into textList
	void loadTexture(const char* name, std::vector<Texture>& textList) {
		 //if texture is loaded before, do not reload this texture
		for (int i = 0; i < textList.size();i++) {
			if (!textList.at(i).name.compare(name)) {
				return;
			}
		}

		std::ifstream input;
		input.open(name, std::ios_base::in);
		if (!input.is_open()) {
			std::cout << "ERROR:: texture file does not exits, program terminates.\n";
			// close stream and exit. exit WILL NOT call destructors
			input.clear();
			input.close();
			exit(-1);
		}

		int width, height;
		std::string b0, b1, b2;		// reading buffers
		input >> b0; 
		input >> b1;
		input >> b2;
		if (b0.compare("P3")) {
			std::cout << "ERROR:: Need P3 keyword, program terminates.\n";
			// close stream and exit. exit WILL NOT call destructors
			input.clear();
			input.close();
			exit(-1);
		}

		Texture temptext;
		temptext.name = std::string(name);

		checkPosInt(b1); checkPosInt(b2);
		width = std::stoi(b1);
		height = std::stoi(b2);
		temptext.width = width;
		temptext.height = height;

		input >> b0;	// for 255
		// read data
		for (int j = 0; j < height; j++) {
			for (int i = 0; i < width; i++) {
				input >> b0; input >> b1; input >> b2;
				checkPosInt(b0); checkPosInt(b1); checkPosInt(b2);
				int r, g, b;
				r = std::stoi(b0);
				g = std::stoi(b1);
				b = std::stoi(b2);
				temptext.rgb.emplace_back(Vector3f (r/255.f, g/255.f, b/255.f));
			}
		}
		textList.emplace_back(temptext);
	}
};