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

class VolumeRendering : public State {

public:

	VolumeRendering(StateMachine& machine);
	~VolumeRendering();

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

	std::vector<WGPUBindGroupLayout> OnBindGroupLayoutsVolume();
	WGPUBindGroup createVolumeBindGroup();

	void renderUi(const WGPURenderPassEncoder& renderPassEncoder);

	bool m_initUi = true;
	bool m_drawUi = true;
	float m_rotation = 0.0f;
	bool m_rotate = true;
	float m_near = 4.3f;
	float m_far = 4.4f;

	Camera m_camera;
	TrackBall m_trackball;

	WgpBuffer m_uniformBuffer;
	WgpTexture m_volumeTexture;
	WGPUBindGroup m_volumeBindGroup;
};