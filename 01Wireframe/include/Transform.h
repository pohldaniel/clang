#ifndef _TRANSFORM_H
#define _TRANSFORM_H

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Transform {

public:

	Transform();
	Transform(const glm::mat4& m);
	Transform(Transform const& rhs);
	Transform(Transform&& rhs) noexcept;
	Transform &operator=(const Transform& rhs);
    Transform &operator=(Transform&& rhs) noexcept;
	~Transform();

	const glm::mat4& getTransformationMatrix() const;
	const glm::mat4& getInvTransformationMatrix();

	void setRotPos(const glm::vec3& axis, float degrees, float dx, float dy, float dz);
	void setRotPosScale(const glm::vec3& axis, float degrees, float dx, float dy, float dz, float sx = 1.0f, float sy = 1.0f, float sz = 1.0f);

	void setPosition(float x, float y, float z);
	void setPosition(const glm::vec3& position);

	void translate(float dx, float dy, float dz);
	void translate(const glm::vec3& trans);

	void rotate(const glm::vec3& axis, float degrees);
	void rotate(const glm::quat& quat);
	void rotate(float pitch, float yaw, float roll);

	void rotate(const glm::vec3& axis, float degrees, const glm::vec3& centerOfRotation);
	void rotate(const glm::quat& quat, const glm::vec3& centerOfRotation);
	void rotate(float pitch, float yaw, float roll, const glm::vec3& centerOfRotation);

	void scale(float s);
	void scale(float a, float b, float c);
	void scale(const glm::vec3& scale);

	void scale(float s, const glm::vec3& centerOfScale);
	void scale(float a, float b, float c, const glm::vec3& centerOfScale);
	void scale(const glm::vec3& scale, const glm::vec3& centerOfScale);

	void reset();

	void fromMatrix(const glm::mat4& m);
	void getPosition(glm::vec3& position);
	void getOrientation(glm::mat4& orientation);
	void getOrientation(glm::quat& orientation);
	void getScale(glm::vec3& scale);

private:

	glm::mat4 T;
	glm::mat4 invT;
};
#endif