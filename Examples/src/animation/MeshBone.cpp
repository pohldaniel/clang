#include "MeshBone.h"

MeshBone::MeshBone() :
	initialPosition(glm::vec3(0.0f, 0.0f, 0.0f)),
	initialRotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)),
	initialScale(glm::vec3(1.0f, 1.0f, 1.0f)),
	offsetMatrix(glm::mat4(1.0f)),
	radius(0.0f),
	parentIndex(0),
	active(true)
{
}

MeshBone::~MeshBone()
{
}

void MeshBone::scale(const float sx, const float sy, const float sz) {
	offsetMatrix = glm::scale(offsetMatrix, glm::vec3(sx, sy, sz));
}

void MeshBone::rotate(const float pitch, const float yaw, const float roll) {
	offsetMatrix *= glm::eulerAngleXYZ(glm::radians(pitch), glm::radians(yaw), glm::radians(roll));
}