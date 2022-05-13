#pragma once
#include <memory>
#include <typeindex>
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletDynamics/Dynamics/btDynamicsWorld.h>
#include <BulletCollision/BroadphaseCollision/btBroadphaseInterface.h>
#include <BulletCollision/CollisionDispatch/btCollisionConfiguration.h>
#include <BulletCollision/CollisionDispatch/btCollisionDispatcher.h>
#include <btBulletDynamicsCommon.h>
#include <LinearMath/btQuickprof.h>
#include <functional>

#include <unordered_map>

#include "Material.h"
#include "Render.h"


namespace Decoration
{
	class Decoration;
	class Physics;
	class Animation;
};

/**
 * 
 */
class RenderObject
{
private:
	void applyToPhysics();
public:
	/**
	 * The Mesh containing the vertecies and indices used to draw
	 */
	Render::Mesh		mesh;
	/*
	 * A pointer to the material the RenderObject is using
	 */
	Material::Material*	material;
	/*
	 *
	 */
	Utils::Map<size_t, Decoration::Decoration*>* decorations;
	/*
	 * This string is just for debugging purposes
	 */
	std::string			name;
	/**
	 * The index of the render object in the scene, set when adding object to the scene
	 */
	unsigned long long	index;
	/**
	 * unsigned 
	 */
	GLuint vaoID, vboID, eboID;
	/*
	 * matrix holding information about transforms
	 */
	glm::mat4 transform;
	btMatrix3x3 pRotate;
	btVector3 pTranslate;
	btVector3 pScale;

	RenderObject(Render::Mesh, Material::Material*, const std::string&);
	
	void init();
	void update(unsigned long frame, float dTime);
	void add(Decoration::Decoration&);
	void buildVAO() const;
	RenderObject* translate(float, float, float);
	RenderObject* translate(glm::vec3);
	RenderObject* rotate(float, float, float);
	RenderObject* rotate(float, glm::vec3);
	RenderObject* scale(float);
	RenderObject* scale(float, float, float);
	RenderObject* scale(glm::vec3);

	template <class T>
	inline T* getDecoration() const {
		auto& type = typeid(T);
		auto name = type.name();
		auto f = decorations->get(type.hash_code());
		if (f == NULL) return nullptr;
		return static_cast<T*>(f);
	}
};

namespace Decoration
{
	class Decoration
	{
	public:
		RenderObject* object;
		std::function<void(RenderObject*, unsigned long, float)> onUpdate;
		virtual void init(RenderObject*) = 0;
		virtual void update(RenderObject* object, unsigned frame, float dTime) = 0;

		void bind(RenderObject*);

		Decoration() = default;
		Decoration(RenderObject*);
	};


	class Physics : public Decoration
	{
	private:
		btCollisionShape* pShape;
		float pMass;
		btMotionState* pTransform;

	public:
		btRigidBody* pBody;
		btDynamicsWorld* pWorld;

		void init(RenderObject*) override;
		void bind(RenderObject*);
		void update(RenderObject* object, unsigned frame, float dTime) override;

		void onCollide(RenderObject* other);

		/*
		 *	if pShape is nullptr the mesh of the object bound will be used
		 */
		Physics(btDynamicsWorld* pWorld, btCollisionShape* pShape, float mass);
	};

	class Animation : public Decoration
	{
	private:
		std::vector<Render::Mesh> meshes;
	public:
		void init(RenderObject*) override;
		void update(RenderObject* object, unsigned frame, float dTime) override;
	};

}
