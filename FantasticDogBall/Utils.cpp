#include "Utils.h"

#include <string>
#include <iostream>
#include <fstream>
#include <GL/glew.h>
#include <chrono>

#include "Shaders.h"

std::string Utils::readFile(const char* path)
{
    std::string content;
    std::ifstream fileStream(path, std::ios::in);

    if (!fileStream.is_open()) {
        std::cerr << "Could not read file " << path << ". File does not exist." << std::endl;
        return "";
    }

    std::string line;
    while (!fileStream.eof()) {
        std::getline(fileStream, line);
        content.append(line + "\n");
    }

    fileStream.close();
    return content;
}

GLenum Utils::checkError_(const char* file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        std::string error;
        switch (errorCode)
        {
        case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
        case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
        case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
        case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
        case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
        case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        // std::cout << error << " | " << file << " (" << line << ")" << std::endl;
		#ifdef FTB_DEBUG
    		fprintf(stderr, "%s: %s (%d)\n", error.c_str(), file, line);
		#endif
    }
    return errorCode;
}



Shaders::Program Utils::loadProgram(std::string vertex, std::string fragment)
{
	try
	{
        return Shaders::Program(vertex, fragment);
	}
	catch (Shaders::ShaderCompilationException& e)
	{
        fprintf(stderr, "Failed to compile shader (%s): %s", e.shaderName.c_str(), e.what());
        exit(-10);
	}
    catch (Shaders::ProgramLinkException& e)
    {
        fprintf(stderr, "Failed to link program (%d): %s", e.program, e.what());
        exit(-11);
    }
}

std::string Utils::getISOCurrentTimestamp()
{
    std::time_t time = std::time(0); // Get current time

    // Construct local time
    char loc[sizeof("2021-03-01T10:44:10Z")];
    tm t;
    localtime_s(&t, &time);
    strftime(loc, sizeof(loc), "%FT%TZ", &t);
    return loc;
}