#include "Player.h"

#include "LevelManager.h"

Player::Player() : Player({0,0,0})
{
}

Player::Player(glm::vec3 position)
{
	material = new Material::StaticMaterial();
	auto* dog_material = new Material::TextureMaterial();
	dog_material->color = new Texture::Texture{ Globals::RESOURCES + "/dog/color.png" };
	dog_material->normal = new Texture::Texture{ Globals::RESOURCES + "/dog/normal.png" };
	dog_material->shininess = .8;
	dog_material->specular = new Texture::Texture{ .5 };
	dog = new RenderObject{ Render::Mesh::fromFile(Globals::RESOURCES + "/dog/dog.robj")[0], dog_material, "PlayerDog", false };

	stand = new Decoration::Animation(std::vector<std::string>{ Globals::RESOURCES + "/dog/dog.robj" }, 0, false);
	stand->init(dog);
	walk = new Decoration::Animation(Globals::RESOURCES + "/dog/dog_walk", .6);
	walk->init(dog);
	trot = new Decoration::Animation(Globals::RESOURCES + "/dog/dog_trot", .6);
	trot->init(dog);
	canter = new Decoration::Animation(Globals::RESOURCES + "/dog/dog_canter", .6);
	canter->init(dog);

	//ball->init(); 
	//dog->init();
}

void Player::update(unsigned long frame, float dTime)
{
	// get information about players state
	auto ballBody = ball->getDecoration<Decoration::Physics>()->pBody;
	glm::vec3 velocity = {
		Utils::round(ballBody->getLinearVelocity().x(), 3),
		0,
		Utils::round(ballBody->getLinearVelocity().z(), 3)
	};

	const auto speed = glm::length(velocity);
	if (speed > Globals::MAX_PLAYER_SPEED) {
		ballBody->setLinearVelocity(ballBody->getLinearVelocity() * (Globals::MAX_PLAYER_SPEED / speed));
	}
	velocity = glm::normalize(velocity);

	// change animation based on the size of the velocity
	if (speed > 17)
		dog->add(canter);
	else if (speed > 9)
		dog->add(trot);
	else if (speed > .5)
		dog->add(walk);
	else
		dog->add(stand);
	// set transformations of player-ball and dog to the same
	ball->update(frame, dTime);
	dog->update(frame, dTime);
	if (speed >= .5f) {	// check if the angle is NaN
		directionAngle = Utils::getAngle(velocity.x, velocity.z);
	}



	auto ballPos = ballBody->getWorldTransform().getOrigin();
	dog->transform = glm::rotate(glm::translate(glm::mat4(1), { ballPos.x(), ballPos.y() - .6, ballPos.z() }), directionAngle, { 0, 1, 0 });
	 
	const auto oldCamDirection = glm::vec3(
		LevelManager::current->scene.renderer.camera.direction.x,
		0,
		LevelManager::current->scene.renderer.camera.direction.z
	);

	float threeSixty = glm::two_pi<float>();
	float oneEighty = glm::pi<float>();
	const auto oldCamAngle = Utils::getAngle(oldCamDirection.x, oldCamDirection.z);
	float diffAngle = directionAngle - oldCamAngle;
	diffAngle += (diffAngle > oneEighty) ? -threeSixty : (diffAngle < -oneEighty) ? threeSixty : 0;

	auto newCamAngle = .0f;
	if (abs(diffAngle) < glm::radians(2.f)			// if difference is less than 2� end interpolation
		|| 360 - abs(glm::degrees(diffAngle)) < -10)	// if the difference is 3� around 360� we say its computational error, prevents camera twitching around straigt headings
		newCamAngle = directionAngle;
	else
		newCamAngle = oldCamAngle + diffAngle * .03;
	LevelManager::current->scene.renderer.camera.setYaw(newCamAngle);
	LevelManager::current->scene.renderer.camera.setPlayerPosition(glm::vec3(ballPos.x(), ballPos.y(), ballPos.z()));
	LevelManager::current->scene.renderer.camera.update();
}

void Player::draw(Shaders::Program prog)
{ 
	// here goes nothing
}

void Player::init()
{
	RenderObject::init();
}

void Player::init(btDynamicsWorld* pWorld)
{
	transform = glm::mat4(1);
	dog->transform = transform;

	auto* ball_mat = new Material::StaticMaterial({ .8, .8, .9, .76 }, { .32 }, { .8 }, 7.4, .4);
	ball = new RenderObject{ new Render::Sphere(1, 16, 32), ball_mat, "PlayerBall", false };
	auto* physics = new Decoration::Physics(pWorld, new btSphereShape(1), 10);
	ball->add(physics);

	ball->transform = transform;
}

