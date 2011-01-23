#include "stdafx.h"

#include "core/image.h"
#include "rt/basic_definitions.h"
#include "rt/geometry_group.h"

#include "impl/lwobject.h"
#include "impl/phong_shaders.h"
#include "impl/basic_primitives.h"
#include "impl/perspective_camera.h"
#include "impl/integrator.h"
#include "rt/renderer.h"
#include "impl/samplers.h"
#include "impl/fractallandscape.h"
#include "rt/noise_textures.h"

#define PI 3.14159265

class BumpMirrorPhongShader : public DefaultPhongShader
{
protected:
	float2 m_texCoord; 
	Point m_position;
public:
	float reflCoef;

	virtual void setPosition(const Point& _point) { m_position = _point; }

	//Set the tangent to (0, 1, 0)
	virtual void setNormal(const Vector& _normal) { DefaultPhongShader::setNormal(_normal); }
	virtual void setTextureCoord(const float2& _texCoord) { m_texCoord = _texCoord;}

	virtual Vector getNormal() const 
	{ 
		Vector ret = m_normal;
		float x = m_texCoord.x - floor(m_texCoord.x) - 0.5f;
		float y = m_texCoord.y - floor(m_texCoord.y) - 0.5f;
		if(sqrt(pow(x,2) + pow(y,2)) <= 0.25f) {
			float centerz = sqrt(pow(2.5f,2) -pow(0.25f,2));
			return (~(Vector(x,y,centerz)));
		}
		return ret;
	}

	virtual float4 getIndirectRadiance(const Vector &_out, Integrator *_integrator) const
	{ 
		Ray r;
		r.o = m_position;

		Vector n = getNormal();
		Vector v = n * fabs(n * ~_out);
		r.d = _out + 2 * (v - _out);

		return _integrator->getRadiance(r) * float4::rep(reflCoef);
	}


	_IMPLEMENT_CLONE(BumpMirrorPhongShader);

};


Vector forwardForCamera(float angleX, float angleY)
{
	// in cinema4d it's opposite
	Vector vectors[3];
	vectors[0] = Vector(1,0,0);
	vectors[1] = Vector(0,cos(angleX),sin(angleY));
	vectors[2] = Vector(0,-sin(angleX),cos(angleX));
	Matrix rotationXMatrix(vectors);
	vectors[0] = Vector(cos(angleY),0,-sin(angleY));
	vectors[1] = Vector(0,1,0);
	vectors[2] = Vector(sin(angleY),0,cos(angleY));
	Matrix rotationYMatrix(vectors);
	return (rotationYMatrix * rotationXMatrix) * Vector(0,0,-1);
}

void assigment4_1_and_2()
{
	Image img(400, 300);
	img.addRef();

	//Set up the scene
	GeometryGroup scene;
// 	LWObject cow;
// 	cow.read("models/cow.obj", true);
// 	cow.addReferencesToScene(scene.primitives);
// 	LWObject sc;
// 	sc.read("models/concrete.obj", true);
// 	sc.addReferencesToScene(scene.primitives);
	LWObject objects;
	objects.read("models/cube.obj", true);
	objects.addReferencesToScene(scene.primitives);	
	scene.rebuildIndex();
	DefaultPhongShader as;
	as.addRef();
	as.diffuseCoef = float4(0.2f, 0.2f, 0, 0);
	as.ambientCoef = as.diffuseCoef;
	as.specularCoef = float4::rep(0);
	as.specularExponent = 10000.f;
	as.transparency = float4::rep(0.9);
// 	FractalLandscape f(Point(-3,0,1), Point(10,13,1), 9, 0.1);
// 	f.shader = &as;
// 	f.addReferencesToScene(scene.primitives);
	scene.rebuildIndex();
	
	// my phong
	RRPhongShader glass;
	glass.n1 = 1.0f;
	glass.n2 = 1.5f;
	glass.diffuseCoef = float4(0.1, 0.1, 0.1, 0);
	glass.ambientCoef = glass.diffuseCoef;
	glass.specularCoef = float4::rep(0.9);
	glass.specularExponent = 10000;
	glass.transparency = float4::rep(0.9);
	glass.addRef();
// 	Sphere sphere(Point(-2,4,3), 1, &sh);;
// 	scene.primitives.push_back(&sphere);
// 	scene.rebuildIndex();	
	objects.materials[objects.materialMap["Glass"]].shader = &glass;

	BumpMirrorPhongShader sh4;
	sh4.diffuseCoef = float4(0.2f, 0.2f, 0, 0);
	sh4.ambientCoef = sh4.diffuseCoef;
	sh4.specularCoef = float4::rep(0.8f);
	sh4.specularExponent = 10000.f;
	sh4.reflCoef = 0.4f;
	sh4.setNormal(Vector(0,1,0));
	sh4.addRef();
// 	cow.materials[cow.materialMap["Floor"]].shader = &sh4;
	
	//sample shader for noise
	ProceduralPhongShader skyShader;
	CloudTexture nt;
	nt.addRef();
	skyShader.amibientNoiseTexture = &nt;
	skyShader.diffNoiseTexture = &nt;
	skyShader.specularCoef = float4::rep(1.0f);
	skyShader.specularExponent = 10000.f;
	skyShader.addRef();
// 	float w = skyShader.amibientNoiseTexture->perlin->width;
 objects.materials[objects.materialMap["Sky"]].shader = &skyShader;
	
 	//Enable bi-linear filtering on the walls
//	((BumpTexturePhongShader*)cow.materials[cow.materialMap["Stones"]].shader.data())->diffTexture->filterMode = Texture::TFM_Point;
 	//((BumpTexturePhongShader*)cow.materials[cow.materialMap["Stones"]].shader.data())->amibientTexture->filterMode = Texture::TFM_Point;
	//((BumpTexturePhongShader*)cow.materials[cow.materialMap["Stones"]].shader.data())->bumpTexture->filterMode = Texture::TFM_Point;
// 	((BumpTexturePhongShader*)sc.materials[sc.materialMap["Mat"]].shader.data())->diffTexture->addressModeX = Texture::TAM_Wrap;
// 	((BumpTexturePhongShader*)sc.materials[sc.materialMap["Mat"]].shader.data())->diffTexture->addressModeY = Texture::TAM_Wrap;


	//Set up the cameras
	PerspectiveCamera cam1(Point(0, 250, 1000), forwardForCamera((-6.0)*PI/180.0, 0), Vector(0, 1, 0), 45,
		std::make_pair(img.width(), img.height()));
	
	cam1.addRef();

	//Set up the integrator
	IntegratorImpl integrator;
	integrator.addRef();
	integrator.scene = &scene;
// 	PointLightSource pls;
// 
// 	pls.falloff = float4(0, 0, 1, 0);
// 
// 	pls.intensity  = float4::rep(0.9f);
// 	pls.position = Point(216, 160, -232);
// 	integrator.lightSources.push_back(pls);
	PointLightSource pls2;

	pls2.falloff = float4(0, 0, 1, 0);

	pls2.intensity  = float4::rep(0.9f);
	pls2.position = Point(-293, 409,-233);
	integrator.lightSources.push_back(pls2);

	integrator.ambientLight = float4::rep(0.1f);

	DefaultSampler samp;
	samp.addRef();
// 	samp.sampleCount = 16;

	//Render
	Renderer r;
	r.integrator = &integrator;
	r.target = &img;
	r.sampler = &samp;

	r.camera = &cam1;
	r.render();
	img.writePNG("result_cam1.png");
	
}

