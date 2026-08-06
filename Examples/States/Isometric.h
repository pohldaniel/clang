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

#include "Camera.h"
#include "TrackBall.h"
#include "Transform.h"

class Isometric : public State {
	
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
	std::vector<WGPUBindGroupLayout> OnBindGroupLayoutsTexture();
	std::vector<WGPUBindGroup> OnBindGroups();
	std::vector<WGPUBindGroup> OnBindGroupsTexture();

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
	RotationButtonResult m_rotatioButtonResult;

	AnimatedModel m_player;
	AnimationController m_animationController;
	Shape m_floor;

	WgpBuffer m_uniformBuffer, m_skinBuffer;
	WgpModel m_wgpPlayer, m_wgpFloor;
	WgpTexture m_wgpFloorD;
};