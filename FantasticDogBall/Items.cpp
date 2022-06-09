#include "Items.h"

#include "LevelManager.h"

Items::DogTreat::DogTreat(btDynamicsWorld* world)
{}

Items::DogTreat::DogTreat(btDynamicsWorld* world, glm::vec3 position) : RenderObject(
	Render::Mesh::fromFile("../res/treat/treat.obj")[0],
	new Material::StaticMaterial({ .4f, .2f, .7f, 1.f }, .5f, .2f, 2, 1), // change to texture
	"DogTreat"
	)
{
	translate(position);
	/*
	auto mat = new Material::TextureMaterial();
	mat->color = { "../res/treat/color.png" };
	mat->normal = { "../res/treat/normal.png" };
	mat->diffuse = { 1.0f };
	mat->specular = { .1f };
	mat->shininess = .1f;
	*/
	auto physics = new Decoration::Physics(world, new btSphereShape(1), 0, [item = this](RenderObject* other) {
		LevelManager::current->remove(item);
		LevelManager::current->bones++;
	});

	auto custom = new Decoration::Custom([](RenderObject* obj, unsigned frame, float deltaTime) {
		obj->rotate(deltaTime, { 0, 1, 0 });
	});
}

void Items::DogTreat::onCollide(RenderObject* other)
{
	exists = false;
}


Items::Goal::Goal() : Goal({0,0,0})
{
}

Items::Goal::Goal(glm::vec3 position) : RenderObject(
	Render::Mesh::fromFile("../res/goal/goal.obj")[0],
	new Material::TextureMaterial(
		new Texture::Texture{ "../res/goal/goal.obj"},
		new Texture::Texture{ "" },
		new Texture::Texture{ .4f },
		new Texture::Texture{ .2f },
		new Texture::Texture{ .1f },
		1.4
	),
	"Goal"
)
{
	translate(position);

	auto custom = new Decoration::Custom([](RenderObject* obj, unsigned frame, float deltaTime) {
		const auto playerPos = LevelManager::current->player->ball->getDecoration<Decoration::Physics>()->pBody->getWorldTransform().getOrigin();
		const auto goalPos = obj->getDecoration<Decoration::Physics>()->pBody->getWorldTransform().getOrigin();
		if (playerPos.x() == goalPos.x() && playerPos.z() == goalPos.z()) {
			LevelManager::state = State::LEVEL_FINISHED;
		}
	});
}