#pragma once
#include <list>
#include <memory>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

class Bone  {

	friend class AnimatedMesh;
	friend class AnimatedModel;
	friend class AnimationState;

public:

	Bone();
	~Bone();
	
	void countChildBones();

	bool animationEnabled() const;
	size_t getNumChildBones() const;

	void setPosition(const glm::vec3& position);
	void setOrientation(const glm::quat& orientation);
	void setScale(const glm::vec3& scale);
	void setName(const std::string& name);
	void setParent(Bone* node);
	void setIsRootBone(bool rootBone);
	void setAnimationEnabled(bool enable);
	void setTransformSilent(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale);
	const bool isRootBone() const;

protected:

	void OnTransformChanged();
	const glm::mat4& getWorldTransformation() const;
	const glm::mat4& getTransformationSOP() const;

private:
	
	Bone* addChild(Bone* node);
	void eraseChild(Bone* child);
	void eraseAllChildren(size_t offset = 0u);
	Bone* findChild(const std::string& name, bool recursive) const;

	bool m_animationEnabled;
	size_t m_numChildBones;
	bool m_isRootBone;
	glm::mat4 m_offsetMatrix;

	Bone* m_parent;
	std::list<std::unique_ptr<Bone>> m_children;
	mutable bool m_isDirty;


	glm::vec3 m_position;
	glm::vec3 m_scale;
	glm::quat m_orientation;
	std::string m_name;
	mutable glm::mat4 m_modelMatrix;
	static thread_local glm::mat4 Transformation;
};