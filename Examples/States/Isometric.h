#pragma once

#include <WebGPU/WgpBuffer.h>
#include <WebGPU/WgpTexture.h>
#include <WebGPU/WgpModel.h>
#include <WebGPU/WgpData.h>

#include <States/StateMachine.h>
#include <animation/AnimationController.h>
#include <animation/AnimatedModel.h>
#include <animation/Animation.h>
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


	void OnMouseMotion(const Event::MouseMoveEvent& event) override;
	void OnScroll(double xoffset, double yoffset) override;
	void OnMouseButtonDown(const Event::MouseButtonEvent& event) override;
	void OnMouseButtonUp(const Event::MouseButtonEvent& event) override;
	void OnKeyDown(const Event::KeyboardEvent& event) override;
	void OnKeyUp(const Event::KeyboardEvent& event) override;
	void resize(int deltaW, int deltaH) override;

private:

	std::vector<WGPUBindGroupLayout> OnBindGroupLayouts();
	std::vector<WGPUBindGroup> OnBindGroups();

	void renderUi(const WGPURenderPassEncoder& renderPassEncoder);

	bool m_initUi = true;
	bool m_drawUi = false;
	bool m_isDeath = false;

	Camera m_camera;
	Uniforms m_uniforms;
	TrackBall m_trackball;
	AnimatedModel m_player;
	AnimationController m_animationController;

	WgpBuffer m_uniformBuffer, m_skinBuffer, m_modeBuffer;
	WgpModel m_wgpPlayer;
};