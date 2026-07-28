#pragma once

#include <WebGPU/WgpBuffer.h>
#include <WebGPU/WgpTexture.h>
#include <WebGPU/WgpModel.h>
#include <WebGPU/WgpData.h>

#include <States/StateMachine.h>
#include <Nuklear/NkContext.h>
#include <Shape/Shape.h>

#include "Camera.h"
#include "TrackBall.h"
#include "Transform.h"

class NuklearGui : public State {
	struct JoystickResult {
		float x = 0.0f;
		float y = 0.0f;
		bool is_active = false;
	};

public:

	NuklearGui(StateMachine& machine);
	~NuklearGui();

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

	void renderUi(const WGPURenderPassEncoder& renderPassEncoder);

	bool m_initUi = true;
	bool m_drawUi = false;
	float m_scrollDelta = 0.0f;

	Camera m_camera;
	Uniforms m_uniforms;
	TrackBall m_trackball;

	JoystickResult nk_virtual_joystick(struct nk_context* ctx, float size_px);
	bool nk_circular_action_button(struct nk_context* ctx, const char* label, float size_px);

	struct nk_image playIcon;
	struct nk_vec2 current_pos;

	WgpBuffer m_uniformBuffer;
};