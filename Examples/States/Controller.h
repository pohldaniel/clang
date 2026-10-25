#pragma once

#include <States/StateMachine.h>
#include <Shape/Shape.h>

#include <WebGPU/WgpData.h>
#include <WebGPU/WgpBuffer.h>
#include <WebGPU/WgpModel.h>

#include "Camera.h"

class Controller : public State {

public:

	Controller(StateMachine& machine);
	~Controller();

	void fixedUpdate() override;
	void update() override;
	void render() override;
    void OnDraw(const WGPUCommandEncoder& commandEncoder, const WGPURenderPassDescriptor& renderPassDescriptor);

	void OnMouseMotion(const Event::MouseMoveEvent& event) override;
	void OnScroll(const Event::MouseWheelEvent& event) override;
	void OnMouseButtonDown(const Event::MouseButtonEvent& event) override;
	void OnMouseButtonUp(const Event::MouseButtonEvent& event) override;
	void OnKeyDown(const Event::KeyboardEvent& event) override;
	void OnKeyUp(const Event::KeyboardEvent& event) override;
	void resize(int deltaW, int deltaH) override;

private:

	std::vector<WGPUBindGroupLayout> OnBindGroupLayouts();
	std::vector<WGPUBindGroup> OnBindGroups();

	void renderUi(const WGPURenderPassEncoder& renderPassEncoder);
	void renderRectangle(float x, float y);
	void renderPad(float x, float y, long value);
	void renderButton(float x, float y, long value, int index);
	void renderStick(float x, float y, float stickX, float stickY);
	
	bool m_initUi = true;
	bool m_drawUi = false;
	long m_state = 0;
	float m_RightStickX = 0.0f;
	float m_RightStickY = 0.0f;
	float m_LeftStickX = 0.0f;
	float m_LeftStickY = 0.0f;

	Camera m_camera;
	Uniforms m_uniforms;
	Shape m_disk, m_quad;

	WgpBuffer m_uniformBuffer, m_storageBuffer;
	WgpModel m_wgpDisk, m_wgpQuad;
	std::vector<std::array<float, 20>> m_instances;
};