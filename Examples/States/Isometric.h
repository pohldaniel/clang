#pragma once

#include <WebGPU/WgpBuffer.h>
#include <WebGPU/WgpTexture.h>
#include <WebGPU/WgpModel.h>
#include <WebGPU/WgpData.h>

#include <animation/AnimationController.h>
#include <animation/AnimatedModel.h>
#include <animation/Animation.h>

#include <States/StateMachine.h>
#include <Nuklear/NkJoystick.h>
#include <Shape/Shape.h>

#include "AssimpModel.h"
#include "Camera.h"
#include "TrackBall.h"
#include "Transform.h"
#include "bullet_store.h"

class Isometric : public State {
	struct Wiggly {
		glm::vec3 nosePos;
		float time;
	};
public:

	Isometric(StateMachine& machine);
	~Isometric();

	void fixedUpdate() override;
	void update() override;
	void render() override;
	void OnDraw(const WGPUCommandEncoder& commandEncoder, const WGPURenderPassDescriptor& renderPassDescriptor);
	void OnFillBuffer(nk_context& nkCntxt);

	void OnMouseMotion(const Event::MouseMoveEvent& event) override;
	void OnScroll(double xoffset, double yoffset) override;
	void OnMouseButtonDown(const Event::MouseButtonEvent& event) override;
	void OnMouseButtonUp(const Event::MouseButtonEvent& event) override;
	void OnKeyDown(const Event::KeyboardEvent& event) override;
	void OnKeyUp(const Event::KeyboardEvent& event) override;
	void resize(int deltaW, int deltaH) override;

private:

	std::vector<WGPUBindGroupLayout> OnBindGroupLayouts();
	std::vector<WGPUBindGroupLayout> OnBindGroupLayoutsFloor();
	std::vector<WGPUBindGroupLayout> OnBindGroupLayoutsWiggly();
	std::vector<WGPUBindGroupLayout> OnBindGroupLayoutsBullet();

	std::vector<WGPUBindGroup> OnBindGroups();
	std::vector<WGPUBindGroup> OnBindGroupsFloor();
	std::vector<WGPUBindGroup> OnBindGroupsBullet();

	void renderUi(const WGPURenderPassEncoder& renderPassEncoder);
	bool getWorldPosition(int xPos, int yPos, const glm::vec3& planeNormal, glm::vec3& outIntersection);
	float getLookAtYRotation(const glm::vec3& objectPos, const glm::vec3& targetPos);

	bool m_initUi = true;
	bool m_drawUi = false;
	bool m_isDeath = false;

	Camera m_camera;
	Uniforms m_uniforms;
	TrackBall m_trackball;
	JoystickResult m_joystickResult;
	RotationButtonResult m_rotationButtonResult;
	Wiggly m_wiggly;
	BulletStore m_bulletStore;

	AssimpModel m_enemy;
	AnimatedModel m_player;
	Shape m_floor, m_bullet;
	Animation m_full;
	WgpBuffer m_uniformBuffer, m_instanceBuffer, m_wigglyBuffer, m_skinBuffer, m_rotationBuffer, m_offsetBuffer;
	WgpModel m_wgpPlayer, m_wgpFloor, m_wgpEnemy, m_wgpBullet;
	WgpTexture m_wgpFloorD, m_wgpEnemyD, m_wgpBulletTexture;

	float prev_idleWeight = 0.0f;
	float prev_rightWeight = 0.0f;
	float prev_forwardWeight = 0.0f;
	float prev_backWeight = 0.0f;
	float prev_leftWeight = 0.0f;
	const float animTransitionTime = 0.2f;
	float deathTime = -1.0f;
	float aimTheta = glm::pi<float>() * 1.5f;
	float lastFireTime = 0.0f;

	static WGPUBindGroup CreateBindGroup(const WgpBuffer& uniformBuffer, const WgpBuffer& wigglyBuffer, const WgpTexture& texture);
};