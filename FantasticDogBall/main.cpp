﻿
// constexpr auto _ITERATOR_DEBUG_LEVEL = 2;

#include <cstdio>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Loggger.h"
#include "Scene.h"
#include "Shaders.h"

#include "LightSource.h"
#include "Material.h"
#include "Level.h"
#include "RenderObject.h"


void error_callback(int error, const char* msg);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void initGl();
void initBullet();
void gl_error_callback(GLenum source​, GLenum type​, GLuint id​,
    GLenum severity​, GLsizei length​, const GLchar* message​, const void* userParam);
std::vector<std::string> getFirstNGLMessages(GLuint numMsgs);

const float
	FOV = 45.0f;
const char*
NAME = "Fantastic Dog Ball";


int main(int argc, char* argv[])
{
    glfwSetErrorCallback(error_callback);

    /* Initialize the library */
    if (!glfwInit()) {
        Loggger::fatal("Failed to initialize GLFW");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    /* Create a windowed mode window and its OpenGL context */
    GLFWwindow* window = glfwCreateWindow(Globals::WINDOW_WIDTH, Globals::WINDOW_HEIGHT, NAME, NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glewExperimental = GL_TRUE;

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        Loggger::fatal("Error when initializing GLEW: %s\n", glewGetErrorString(err));
        glfwTerminate();
        return -1;
    }
    Loggger::info("Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

    initGl();
    initBullet();

    const float ratio = static_cast<float>(Globals::WINDOW_WIDTH) / static_cast<float>(Globals::WINDOW_HEIGHT);

    Level level;
    const glm::vec3 cameraPos = glm::vec3(0, 1, -6);

    const auto proj = glm::perspective<float>(45, ratio, .1f, 100.0f);
    const auto view = glm::lookAt<float>(cameraPos, {.0f, .0f, .0f}, {.0f, 1.0f, .0f});
    level.scene.renderer.camera.setData(Camera::Data{ glm::mat4(1), view, proj, glm::vec4(cameraPos.x, cameraPos.y, cameraPos.z, 1)});

    Light::Point p = {
        glm::vec3(0, 1, 0),
        2.0f, 1.0f, .5f,
        glm::vec3(1.5,1.5,1.5),
        glm::vec3(5,5,5),
        glm::vec3(2,2,2)
    };

    //level.scene.lights.add(p);

    Light::Directional d = {
        glm::vec3(2, -4, 1),
        glm::vec3(.3,.3,.3),
        glm::vec3(.8,.8,.8),
        glm::vec3(.9,.9,.9),
        true
    };

    level.add(d);
    
    level.scene.lights.finalize();

    Material::TextureMaterial texture = Material::TextureMaterial{};
    texture.color = { "../res/concrete.jpg" };
    texture.normal = { "../res/concrete_norm.jpg" };
    texture.diffuse = { .8 };
    texture.specular = { 2 };
    texture.shininess = 1;
     
    Material::StaticMaterial material1 = Material::StaticMaterial{};
    material1.vals.color = { 0.2, .4 , 0.3, 1.0 };
    material1.vals.data = { 1.9f, 1.0f, 1.5, 0 };

    Material::TextureMaterial dog_mat;
    dog_mat.color = { "../res/dog_texture.png" };
    dog_mat.normal = { "../res/fur_normal.png" };
    dog_mat.diffuse = { .8 };
    dog_mat.specular = { .1 };
    dog_mat.shininess = 1;

    auto sphere = RenderObject{
        Render::Sphere(1, 32, 16), &material1, "Sphere"
    };
    sphere.translate({ 0, 2, 0 });
    auto cube = RenderObject{
        Render::Cube{
             0, -5, 0, 50, .2, 50
        }, &texture, "Cube"
    };
    cube.translate({ 0, -.5, 0 });

    auto dog = RenderObject(Render::Mesh::fromFile("../res/Cachorro.obj")[0], &dog_mat, "Doggo");
    Decoration::Animation anim("../res/dog_walk", .35);
    Decoration::Custom custom([](RenderObject* obj, unsigned frame, float dTime) {
        obj->rotate(frame / static_cast<float>(10000), { 0,1,1 });
    });
    dog.add(anim);
    dog.add(custom);
    level.add(dog);
    level.add(cube);
    level.add(sphere);


    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        /* Render here */
        glClearColor(1.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Utils::checkError();

        level.render();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);
        Utils::checkError();

        /* Poll for and process events */
        glfwPollEvents();
    }

    Shaders::cleanup();
    level.cleanup();

    glfwTerminate();
    return 0;
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void error_callback(int error, const char* msg) {
    Loggger::error("(%i) %s", error, msg);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}


void initGl()
{
	#ifdef FDB_DEBUG
		glEnable(GL_DEBUG_OUTPUT);

        glEnable(GL_DEBUG_OUTPUT);

        if (glDebugMessageCallbackARB != nullptr) {
            Loggger::debug("GL debug is available.\n");
            // Enable the debug callback
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(gl_error_callback, nullptr);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
            glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 0,
                GL_DEBUG_SEVERITY_NOTIFICATION, -1, "Started debugging");
        }
        else {
            Loggger::debug("GL debug is not available.\n");
        }

	#endif

    glEnable(GL_DEPTH_TEST);
    glDebugMessageCallback(gl_error_callback, nullptr);

	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
}

void initBullet() {
    auto const collisionConfiguration = new btDefaultCollisionConfiguration();
    auto const dispatcher = new btCollisionDispatcher(collisionConfiguration);
    auto const overlappingPairCache = new btDbvtBroadphase();
    auto const solver = new btSequentialImpulseConstraintSolver;
    auto const dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
    dynamicsWorld->setGravity(btVector3(0, -10, 0));
}

void gl_error_callback(GLenum source​, GLenum type​, GLuint id​_, GLenum severity​, GLsizei length​, const GLchar* message​, const void* userParam)
{
    Loggger::log(severity​, "OpenGL-Log (%d): %s", id​_, message​);
}

std::vector<std::string> getFirstNGLMessages(GLuint numMsgs)
{
    GLint maxMsgLen = 0;
    glGetIntegerv(GL_MAX_DEBUG_MESSAGE_LENGTH, &maxMsgLen);

    std::vector<GLchar> msgData(numMsgs * maxMsgLen);
    std::vector<GLenum> sources(numMsgs);
    std::vector<GLenum> types(numMsgs);
    std::vector<GLenum> severities(numMsgs);
    std::vector<GLuint> ids(numMsgs);
    std::vector<GLsizei> lengths(numMsgs);

    GLuint numFound = glGetDebugMessageLog(numMsgs, msgData.size(), &sources[0], &types[0], &ids[0], &severities[0], &lengths[0], &msgData[0]);

    sources.resize(numFound);
    types.resize(numFound);
    severities.resize(numFound);
    ids.resize(numFound);
    lengths.resize(numFound);

    std::vector<std::string> messages;
    messages.reserve(numFound);

    std::vector<GLchar>::iterator currPos = msgData.begin();
    for (size_t msg = 0; msg < lengths.size(); ++msg)
    {
        messages.push_back(std::string(currPos, currPos + lengths[msg] - 1));
        currPos = currPos + lengths[msg];
    }
    return messages;
}