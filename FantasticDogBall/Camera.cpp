#include "Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


Camera::Camera(): data({ glm::mat4(1), glm::mat4(1) })
{
	buffer.create(sizeof(Data));
}


void Camera::setData(Data data_)
{
	data = data_;
	buffer.update(sizeof(data), &data);
}

void Camera::bindCamera(Shaders::Program prog) const
{
	prog.setUniform("CameraData", 1, buffer);
}


