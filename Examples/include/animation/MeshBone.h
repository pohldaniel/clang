#pragma once

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

struct MeshBone {

	MeshBone();
	~MeshBone();
	std::string name;
	glm::vec3 initialPosition;
	glm::quat initialRotation;
	glm::vec3 initialScale;
	glm::mat4 offsetMatrix;
	float radius;
	size_t parentIndex;
	bool active;

	void scale(const float sx, const float sy, const float sz);
	void rotate(const float pitch, const float yaw, const float roll);
};