#pragma once

#include <WebGPU/WgpBuffer.h>
#include <WebGPU/WgpTexture.h>
#include <WebGPU/WgpModel.h>
#include <WebGPU/WgpData.h>

#include <States/StateMachine.h>
#include <Shape/Shape.h>

#include "Camera.h"
#include "TrackBall.h"
#include "Transform.h"
#include "VideoReader.h"

class VideoDecode : public State {
	
public:

	VideoDecode(StateMachine& machine);
	~VideoDecode();

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
	WGPUBindGroup createBindGroup();
	void renderUi(const WGPURenderPassEncoder& renderPassEncoder);
	void upload();

	bool m_initUi = true;
	bool m_drawUi = false;

	Camera m_camera;
	TrackBall m_trackball;
	int frame_width;
	int frame_height;
	VideoReaderState vr_state;
	uint8_t* frame_data;

	WgpTexture m_texture;
	WGPUBindGroup m_bindGroup;
};