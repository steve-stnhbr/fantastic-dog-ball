#include "RenderObject.h"

#include <memory>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <LinearMath/btMotionState.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_transform.hpp>
#include <iostream>
#include <filesystem>
#include <glm/gtc/type_ptr.hpp>
#include <ranges>
#include <glm/gtx/string_cast.hpp>

#include "Utils.h"

RenderObject::RenderObject() = default;

RenderObject::RenderObject(Render::Mesh* mesh_, Material::Material* material_, const std::string& name_, bool toDelete) :
	mesh(mesh_),
	material(material_),
	name(name_),
	transform(glm::mat4(1)),
	pScale({1, 1, 1}),
	toDelete(toDelete)
{
	buildVAO();
	decorations = Utils::Map<size_t, Decoration::Decoration*>();
}

RenderObject::~RenderObject()
{
	name.~basic_string();
	delete material;
}

void RenderObject::init()
{
	for (auto* const val : decorations.vals) {
		if(!val->initialized)
		val->init(this);
	}
}

void RenderObject::update(unsigned long frame, float dTime)
{
	for (auto* const val : decorations.vals) {
		val->update(this, frame, dTime);
	}
}
void RenderObject::draw(Shaders::Program prog)
{
	glBindVertexArray(vaoID);
	mesh->bind(prog);
	material->bind(prog);
	Utils::checkError();

	glDrawElements(GL_TRIANGLES, mesh->index_array.size(), GL_UNSIGNED_INT, nullptr);
	Utils::checkError();
}


void RenderObject::add(Decoration::Decoration& decoration_)
{
	decoration_.bind(this);
	decorations.insert(typeid(decoration_).hash_code(), &decoration_);
}

void RenderObject::add(Decoration::Decoration* decoration_) {
	decoration_->bind(this);
	decorations.insert(typeid(*decoration_).hash_code(), decoration_);
}

void RenderObject::buildVAO() const
{
	Loggger::trace("Creating RenderObject %s", name.c_str());
	glCreateVertexArrays(1, (GLuint*)&vaoID);
	mesh->createBuffers(vaoID);
	material->assignVertexAttributes(vaoID);
}

void RenderObject::cleanup()
{

}

RenderObject* RenderObject::scale(float x, float y, float z)
{
	scale({ x,y,z });
	return this;
}

RenderObject* RenderObject::scale(glm::vec3 s)
{
	transform = glm::scale(transform, s);
	pScale += {s.x, s.y, s.z};

	applyToPhysics();
	return this;
}

RenderObject* RenderObject::translate(float x, float y, float z)
{
	translate({ x, y, z });
	return this;
}

RenderObject* RenderObject::translate(glm::vec3 v)
{
	transform = glm::translate(transform, v);
	applyToPhysics();
	return this;
}

RenderObject* RenderObject::rotate(float x, float y, float z)
{
	rotate(x, { 1, 0, 0 });
	rotate(y, { 0, 1, 0 });
	rotate(z, { 0, 0, 1 });
	return this;
}

RenderObject* RenderObject::rotate(float angle, glm::vec3 axes)
{
	transform = glm::rotate(transform, angle, axes);
	applyToPhysics();
	return this;
}

RenderObject* RenderObject::scale(float s)
{
	scale(s, s, s);
	applyToPhysics();
	return this;
}

void RenderObject::applyToPhysics() {
	return;
	if (auto p = getDecoration<Decoration::Physics>()) {
		p->pBody->getWorldTransform().setFromOpenGLMatrix(glm::value_ptr(transform));
	}
}


void Decoration::Decoration::init(RenderObject*)
{
	initialized = true;
}

void Decoration::Decoration::bind(RenderObject* object_)
{
	object = object_;
}

Decoration::Decoration::Decoration(RenderObject* obj_): object(obj_)
{
}

void Decoration::Physics::init(RenderObject* object)
{
	if (object == nullptr) {
		Loggger::fatal("Object was somehow not yet initialized in init of Decoration");
		exit(-15);
	}

	if (pShape == nullptr) {
		Loggger::trace("Using Convex hull of mesh for %s", object->name.c_str());

		auto mesh = new btTriangleMesh();

		for (auto i = 0; i < object->mesh->index_array.size(); i += 3) {
			auto vert0 = object->mesh->vertex_array[object->mesh->index_array[i]];
			auto vert1 = object->mesh->vertex_array[object->mesh->index_array[i + 1]];
			auto vert2 = object->mesh->vertex_array[object->mesh->index_array[i + 2]];

			auto vector0 = btVector3(vert0.position.x, vert0.position.y, vert0.position.z);
			if (vector0.fuzzyZero() && !vector0.isZero()) vector0.setZero();
			auto vector1 = btVector3(vert1.position.x, vert1.position.y, vert1.position.z);
			if (vector1.fuzzyZero() && !vector1.isZero()) vector1.setZero();
			auto vector2 = btVector3(vert2.position.x, vert2.position.y, vert2.position.z);
			if (vector2.fuzzyZero() && !vector2.isZero()) vector2.setZero();

			mesh->addTriangle(vector0, vector1, vector2);
		}

		pShape = new btBvhTriangleMeshShape(mesh, false);
	}

	btTransform t;
	t.setIdentity();
	t.setFromOpenGLMatrix(glm::value_ptr(object->transform));

	pTransform = new btDefaultMotionState(t);
	pShape->setLocalScaling(object->pScale);
	
	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(
		pMass,					// mass, in kg. 
		pTransform,				// worldTransform of body
		pShape,					// collision shape of body
		btVector3(1, 1, 1)		// local inertia
	);
	rigidBodyCI.m_restitution = .5;

	pBody = new btRigidBody(rigidBodyCI);
	pBody->setUserPointer(object);
	pBody->setRollingFriction(.2);
	pBody->setDeactivationTime(0);
	pBody->setSleepingThresholds(1.f, 1.f);

	pWorld->addRigidBody(pBody);
}

void Decoration::Physics::update(RenderObject* obj, unsigned frame, float dTime)
{
	pBody->activate(true);
	if(onUpdate)
		onUpdate(obj, frame, dTime);

	btTransform transform;
	pBody->getMotionState()->getWorldTransform(transform);
	glm::mat4 mat;
	transform.getOpenGLMatrix(glm::value_ptr(obj->transform));
	Loggger::trace("Bullet: Updating transform of %s to %s", obj->name.c_str(), glm::to_string(mat).c_str());
} 

void Decoration::Physics::onCollide(RenderObject* other)
{
	if (collidingFunc) collidingFunc(other);
}

Decoration::Physics::Physics(btDynamicsWorld* pWorld_, btCollisionShape* pShape_, float pMass_) : 
	pShape(pShape_), 
	pMass(pMass_), 
	pWorld(pWorld_), 
	collidingFunc(nullptr)
{
}

Decoration::Physics::Physics(btDynamicsWorld* pWorld_, btCollisionShape* pShape_, float pMass_, std::function<void(RenderObject*)> collidingFunc_) : 
	pShape(pShape_), 
	pMass(pMass_), 
	pWorld(pWorld_), 
	collidingFunc(collidingFunc_)
{
}

Decoration::Physics::~Physics()
{
	delete pShape;
}

Decoration::Animation::Animation() : 
	meshes({new Render::Mesh()}), 
	paths({ "" }),
	speed(1),
	started(false),
	played(false),
	started_frame(0)
{
	initialized = false;
}

Decoration::Animation::Animation(std::string path, float speed, bool loop) : loop(loop), speed(speed), played(false), started(false), started_frame(0)
{
	for (const auto& file : std::filesystem::directory_iterator(path.c_str()))
		if(file.path().extension() == ".robj")
			paths.push_back(file.path().string());
}

Decoration::Animation::Animation(std::vector<std::string> paths, float speed, bool loop) : paths(paths), loop(loop), speed(speed), played(false), started(false), started_frame(0)
{
}

void Decoration::Animation::init(RenderObject* obj)
{
	Loggger::trace("Initializing Animation for RenderObject %s", obj->name.c_str());
	for (const auto& file : paths) {
		auto mesh = Render::Mesh::fromFile(file)[0];
		mesh->createBuffers(obj->vaoID);
		meshes.push_back(mesh);
	}
}

void Decoration::Animation::update(RenderObject* obj,unsigned frame, float dTime)
{
	if (!started) {
		started = true;
		started_frame = frame;
	}
	if (!loop && played)
		return;
	auto index = static_cast<unsigned>(floor((frame - started_frame) * speed)) % meshes.size();
	if (index == 0 && started_frame < frame) played = true;
	obj->mesh = meshes[index];
}

void Decoration::Animation::reset()
{
	started = false;
	played = false;
}

Decoration::Custom::Custom(std::function<void(RenderObject*, unsigned, float)> update) : updateFunc(update)
{
}

Decoration::Custom::Custom(std::function<void(RenderObject*, unsigned, float)> update, std::function<void(RenderObject*)> init) : updateFunc(update), initFunc(init)
{
}

void Decoration::Custom::init(RenderObject* obj) {
	if(initFunc)
		initFunc(obj);
}

void Decoration::Custom::update(RenderObject* obj, unsigned frame, float dTime)
{
	updateFunc(obj, frame, dTime);
}


