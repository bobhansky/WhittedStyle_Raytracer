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
#include "Material.hpp"
#include "PPMGenerator.hpp"


/// <summary>
/// this is the class perform computer graphics algorithms
/// It takes a ppmgenerator and then do algorithm based on ppmGenerator's data
/// </summary>
class Renderer {
public:
	// constructor, takes a PPMGenerator for future commands
	Renderer(PPMGenerator *ppmg) {
		g = ppmg;
	}


	// takes a PPMGenerator and render its rgb array
	void render() {
		// we shoot a ray from eyePos, trough each pixel, 
		// to the scene. update the rgb info if we hit objects
		// v: vertical unit vector of the view plane
		// u: horizontal unit vector of the view plane

		Vector3f u = crossProduct(g->viewdir, g->updir);
		u = normalized(u);
		Vector3f v = crossProduct(u, g->viewdir);
		v = normalized(v);

		// calculate the near plane parameters
		// we set the distance from eye to near plane to 1, namely the nearplane.z = eyePos.z - 1;
		float d = 1.f;
		// distance from eye to nearplane MATTERS when we are doing orthographic ray
		// cuz we need a bigger viewplane   I set it to 4 in respond to my scene set
		if (g->parallel_projection) d = 4.f;

		// tan(hfov/2) =  nearplane.width/2 : d
		// h/2 = tan(hfov/2) * d 
		// here height_half and width_half are the h and w of the nearplane in the world space
		float width_half = fabs(tan(degree2Radians(g->hfov / 2.f)) * d);
		float aspect_ratio = g->width / (float)g->height;
		float height_half = width_half / aspect_ratio;


		// we sample the center of the pixel 
		// so we need to add offset to the center later
		Vector3f n = normalized(g->viewdir);
		Vector3f eyePos = g->eyePos;
		Vector3f ul = eyePos + d * n - width_half * u + height_half * v;
		Vector3f ur = eyePos + d * n + width_half * u + height_half * v;
		Vector3f ll = eyePos + d * n - width_half * u - height_half * v;
		Vector3f lr = eyePos + d * n + width_half * u - height_half * v;


		Vector3f delta_h = Vector3f(0, 0, 0);	// delta horizontal
		if (g->width != 1) delta_h = (ur - ul) / (g->width - 1);
		Vector3f delta_v = Vector3f(0, 0, 0);	// delta vertical
		if (g->height != 1) delta_v = (ll - ul) / (g->height - 1);
		Vector3f c_off_h = (ur - ul) / (float)(g->width * 2);	// center horizontal offset
		Vector3f c_off_v = (ll - ul) / (float)(g->height * 2); // vertical

		// trace ray: cast a ray from eyePos through each pixel
		// try to get the tNear and its intersection properties
		for (int y = 0; y < g->height; y++) {
			Vector3f v_off = y * delta_v;
			for (int x = 0; x < g->width; x++) {
				Vector3i& color = g->rgb.at(g->getIndex(x, y));		// update this color to change the rgb array
				Vector3f h_off = x * delta_h;
				Vector3f pixelPos = ul + h_off + v_off + c_off_h + c_off_v;		// pixel center position in world space
				Vector3f rayDir;
				Vector3f eyeLocation;
				// calculate the rayDir and eyeLocation base on different projection method
				if (!g->parallel_projection) {	// perspective
					rayDir = normalized(pixelPos - eyePos);
					eyeLocation = eyePos;
				}
				else {	// orthographic 
					// set ray direction orthogonal to the view plane
					rayDir = n;
					eyeLocation = pixelPos - d * n;		// - d * n  only to set a distance between the eye location and near plane(enlarged)
				}

				Vector3f res = traceRay(eyeLocation, rayDir);
				color.x = 255 * std::min(res.x, 1.f);
				color.y = 255 * std::min(res.y, 1.f);
				color.z = 255 * std::min(res.z, 1.f);
			}
			showProgress((float)y / g->height);
		}
		showProgress(1.f);
		std::cout << std::endl;
	}

private:
	PPMGenerator* g;


	// trace a ray into the scene, record the time of the FIRST intersection  and its corresponding mtlColor
	// into Intersection object
	// return the result color
	Vector3f traceRay(const Vector3f& origin, const Vector3f& dir) {
		Intersection inter;

		// loop through all the objects in the scene and find the nearest intersection
		// const Class &: const lvalue reference
		for (const auto& obj : g->scene.objList) {
			Intersection interTemp;
			if (obj->intersect(origin, dir, interTemp)) {	// intersect also update intersection
				// if the ray hits this object first, then we update intersection
				if (interTemp.t < inter.t - 0.000001) {		// dealling with float number
					inter = interTemp;		// update the intersection object for future calculation
				}
			}
		}

		// if ray has no intersection, return bkgcolor
		if (!inter.intersected) return g->bkgcolor;
		if (inter.obj->isLight) return inter.obj->mtlcolor.diffuse;	// if hits light avatar, return the light color

		// if texture is activated, change mtlcolor.diffuse to texture data
		if (! FLOAT_EQUAL(-1.f, inter.textPos.x) && !FLOAT_EQUAL(-1.f, inter.textPos.y)) {
			inter.mtlcolor.diffuse = g->textures.at(inter.textureIndex)
				.getRGBat(inter.textPos.x, inter.textPos.y);
		}

		// if do shading with normal map
		if (inter.normalMapIndex != -1) {
			changeNormalDir(inter);
			int a = 1;
		}

		// if have the nearest intersection, calculate color
		return blinnPhongShader(origin, inter);
	}


	// use Blinn Phon reflectance model to calculate the color
	// return the calculated color
	Vector3f blinnPhongShader(const Vector3f& eyePos, Intersection& inter) {
		// p is the intersection point
		Vector3f p_eye_dir = normalized(eyePos - inter.pos);
		// calculate ambient term
		Vector3f ambient = inter.mtlcolor.ka * inter.mtlcolor.diffuse;
		Vector3f diffuse;
		Vector3f specular;
		Vector3f res;	// result color

		// loop through all the light sources
		for (auto& light : g->scene.lightList) {
			// -----Point light case
			if (FLOAT_EQUAL(light->pos.w, 1.f)) {
				// ---------------hard shadow
				if (g->shadowType == 0) {
					Vector3f lightPos = Vector3f(light->pos.x, light->pos.y, light->pos.z);
					float d_p_light = (lightPos - inter.pos).norm();	// distance from p to light
					float attenuation = 1.f;
					if (light->c1 >= 0.f) attenuation = 1.f / (light->c1 + light->c2 * d_p_light + light->c3 * d_p_light * d_p_light);
					Vector3f p_light_dir = normalized(lightPos - inter.pos);

					// shadow coefficient
					float shadow = getShadowCoeffi(inter, lightPos);

					// calculate diffuse term
					diffuse = diffuse +
						shadow * light->color * inter.mtlcolor.kd * inter.mtlcolor.diffuse * attenuation *
						std::max(p_light_dir.dot(normalized(inter.nDir)), 0.f);

					// calculate specular term
					Vector3f h = normalized(p_light_dir + p_eye_dir);
					specular = specular +
						shadow * light->color * inter.mtlcolor.ks * inter.mtlcolor.specular * attenuation *
						powf(std::max(h.dot(inter.nDir), 0.f), inter.mtlcolor.n);
				}
				// -----------soft shadow
				else {
					Vector3f lightPos = Vector3f(light->pos.x, light->pos.y, light->pos.z);
					float d_p_light = (lightPos - inter.pos).norm();	// distance from p to light
					float attenuation = 1.f;
					if (light->c1 >= 0.f) attenuation = 1.f / (light->c1 + light->c2 * d_p_light + light->c3 * d_p_light * d_p_light);
					Vector3f p_light_dir = normalized(lightPos - inter.pos);

					// shadow coefficient
					float shadow = getAreaLightShadowCoeffi(inter, light.get());

					// calculate diffuse term
					diffuse = diffuse +
						shadow * light->color * inter.mtlcolor.kd * inter.mtlcolor.diffuse * attenuation *
						std::max(p_light_dir.dot(normalized(inter.nDir)), 0.f);

					// calculate specular term
					Vector3f h = normalized(p_light_dir + p_eye_dir);
					specular = specular +
						shadow * light->color * inter.mtlcolor.ks * inter.mtlcolor.specular * attenuation *
						powf(std::max(h.dot(inter.nDir), 0.f), inter.mtlcolor.n);
				}
			}

			// ------directional light case
			else {
				Vector3f p_light_dir = normalized(Vector3f(-light->pos.x, -light->pos.y, -light->pos.z));

				// shadow coefficient		
				float shadow = getShadowCoeffi(inter, light->pos);

				// calculate diffuse term
				diffuse = diffuse +
					shadow * light->color * inter.mtlcolor.kd * inter.mtlcolor.diffuse *
					std::max(p_light_dir.dot(normalized(inter.nDir)), 0.f);

				// calculate specular term
				Vector3f h = normalized(p_light_dir + p_eye_dir);
				specular = specular +
					shadow * light->color * inter.mtlcolor.ks * inter.mtlcolor.specular *
					powf(std::max(h.dot(inter.nDir), 0.f), inter.mtlcolor.n);
			}
		}
			
		res = ambient + diffuse + specular;

		// do depthcueing 
		if (g->depthCueing) {
			float p_eye_dist = (inter.pos - eyePos).norm();
			float alpha = 0;
			if (p_eye_dist <= g->distmin) alpha = g->amax;
			else if (p_eye_dist >= g->distmax) alpha = g->amin;
			else alpha = g->amin + (g->amax - g->amin) * (g->distmax - p_eye_dist) / (g->distmax - g->distmin);

			res = alpha * res + (1 - alpha) * g->dc;
		}

		return res;
	}

	// point light 
	// return 0 if there's a object between the intersection and the light source
	// return 1 otherwise
	float getShadowCoeffi(Intersection& p, Vector3f& lightPos) {
		Vector3f orig = p.pos;
		Vector3f raydir = normalized(lightPos - orig);

		// loop through all the objects in the scene 
		// if there's one valid intersection, thrn return 0
		for (auto& i : g->scene.objList) {
			if (i.get() == p.obj) continue;			// do not test intersection with itself
			if (i->isLight) continue;				// do not test with light avatar
			Intersection p_light_inter;

			if (i->intersect(orig, raydir, p_light_inter)) return 0;
		}
		// no intersection
		return 1;
	}

	// overload for directional light
	// return 0 if there's a object between the intersection and the light source
	// return 1 otherwise
	float getShadowCoeffi(Intersection& p, Vector4f& lightDir) {
		Vector3f orig = p.pos;
		Vector3f negDir = Vector3f(-lightDir.x, -lightDir.y, -lightDir.z);
		Vector3f raydir = normalized(negDir);

		// loop through all the objects in the scene 
		// if there's one valid intersection, thrn return 0
		for (auto& i : g->scene.objList) {
			if (i.get() == p.obj) continue;			// do not test intersection with itself
			if (i->isLight) continue;				// do not test with light avatar
			Intersection p_light_inter;

			if (i->intersect(orig, raydir, p_light_inter)) return 0;
		}
		// no intersection
		return 1;
	}


	// calculate the shadow Coeffitient for area light		soft shadow
	// randomly sample in the area multiple times and then average each result;
	float getAreaLightShadowCoeffi(Intersection& p, Light* light) {
		float sum = 0.f;
		int sampleNum = 100;
		for (int i = 0; i < sampleNum; i++) {
			Vector3f lightPos = randomSampleTriangle(light->triangle);
			sum += getShadowCoeffi(p, lightPos);
			
		}
		return sum / sampleNum;
	}

	// TBN transformation matrix to change the dir of normal
	void changeNormalDir(Intersection& inter) {
		Texture& nMap = g->normalMaps.at(inter.normalMapIndex);
		Vector3f color = nMap.getRGBat(inter.textPos.x, inter.textPos.y);

		switch (inter.obj->objectType)
		{
		
			
		case TRIANGLE: {
			Triangle* t = static_cast<Triangle*>(inter.obj);
			// our triangle start from lower left corner and go counterclockwise
			// but in this algorithm the v0 should be the upper left corner// which is our v2
			// so e1 e2 should be:

			Vector3f e1 = t->v1 - t->v0;
			Vector3f e2 = t->v2 - t->v0;
			Vector3f nDir = crossProduct(e1, e2); // note the order!
			nDir = normalized(nDir);

			float deltaU1 = t->uv1.x - t->uv0.x;
			float deltaV1 = t->uv1.y - t->uv1.y;

			float deltaU2 = t->uv2.x - t->uv0.x;
			float deltaV2 = t->uv2.y - t->uv0.y;

			float coef = 1 / (-deltaU1 * deltaV2 + deltaV1 * deltaU2);

			Vector3f T = coef * (-deltaV2 * e1 + deltaV1 * e2);
			Vector3f B = coef * (-deltaU2 * e1 + deltaU1 * e2);
			T = normalized(T);
			B = normalized(B);

			Vector3f res;
			res.x = T.x * color.x + B.x * color.y + nDir.x * color.z;
			res.y = T.y * color.x + B.y * color.y + nDir.y * color.z;
			res.z = T.z * color.x + B.z * color.y + nDir.z * color.z;

			inter.nDir = normalized(res);
			break;
		}
					 

		case SPEHRE: {
			Vector3f nDir = inter.nDir;
			Vector3f T = Vector3f(-nDir.y / sqrtf(nDir.x * nDir.x + nDir.y * nDir.y),
				nDir.x / sqrtf(nDir.x * nDir.x + nDir.y * nDir.y), 0);
			
			Vector3f B = crossProduct(nDir, T);

			Vector3f res;
			res.x = T.x * color.x + B.x * color.y + nDir.x * color.z;
			res.y = T.y * color.x + B.y * color.y + nDir.y * color.z;
			res.z = T.z * color.x + B.z * color.y + nDir.z * color.z;

			inter.nDir = normalized(res);
			break;
		}
		
		default:
			break;
		}

	}
};