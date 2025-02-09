#pragma once

#include <glm/glm.hpp>

// This class defines all the different values for a single point on the mesh
class Vertex {
private:

public:

	explicit Vertex(glm::vec3 position);
	Vertex(glm::vec3 position, glm::vec3 normal);
	Vertex(glm::vec3 position, glm::vec3 normal, glm::vec2 texture_coordinate);
	Vertex(glm::vec3 position, glm::vec3 normal, glm::vec4 color);

	// defines the position in object-space 
	glm::vec3 position;
	// defines the normal of the surface at the point
	glm::vec3 normal;
	// defines the coordinate of the texture at the point
	// constraint: 0 >= v.x >= 1, 0 >= v.y >= 1
	glm::vec2 texture_coordinate;
	// defines the color at the point
	glm::vec4 color;
};