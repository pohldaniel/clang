#pragma once

#include <WebGPU/WgpTexture.h>
#include <WebGPU/WgpBuffer.h>
#include <WebGPU/WgpMesh.h>
#include <WebGPU/WgpData.h>
#include <WebGPU/WgpModel.h>

#include <States/StateMachine.h>
#include <Shape/Shape.h>

#include "Camera.h"

class InstancedCube : public State {

public:

	InstancedCube(StateMachine& machine);
	~InstancedCube();

	void fixedUpdate() override;
	void update() override;
	void render() override;

	void OnDraw(const WGPUCommandEncoder& commandEncoder, const WGPURenderPassDescriptor& renderPassDescriptor);

	void OnMouseMotion(const Event::MouseMoveEvent& event) override;
	void OnMouseButtonDown(const Event::MouseButtonEvent& event) override;
	void OnMouseButtonUp(const Event::MouseButtonEvent& event) override;
	void OnScroll(const Event::MouseWheelEvent& event) override;
	void OnKeyDown(const Event::KeyboardEvent& event) override;
	void OnKeyUp(const Event::KeyboardEvent& event) override;
    void resize(int deltaW, int deltaH) override;
	
private:

	std::vector<WGPUBindGroupLayout> OnBindGroupLayouts();
	std::vector<WGPUBindGroup> OnBindGroups();

	void renderUi(const WGPURenderPassEncoder& renderPassEncoder);
	void initMVPMatrices();
	void updateMVPMatrices();

	bool m_initUi = true;
	bool m_drawUi = false;

	Camera m_camera;
	WgpBuffer m_uniformBuffer;

	Shape m_cube;
	WgpModel m_wgpCube;
	glm::mat4 m_mvps[16u];
};