#pragma once

#include <WebGPU/WgpBuffer.h>
#include <WebGPU/WgpModel.h>
#include <WebGPU/WgpData.h>

#include <States/StateMachine.h>
#include <Shape/Shape.h>

#include "Camera.h"
#include "TrackBall.h"
#include "Transform.h"
#include "AssimpModel.h"

#define MAX_NUM_LIGHTS 1024u

class DefferedRendering : public State {

public:

	DefferedRendering(StateMachine& machine);
	~DefferedRendering();

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

	std::vector<WGPUBindGroupLayout> OnBindGroupLayoutsGBuffer();
	std::vector<WGPUBindGroupLayout> OnBindGroupLayoutsCompute();
	std::vector<WGPUBindGroupLayout> OnBindGroupLayoutsDeffered();
	std::vector<WGPUBindGroupLayout> OnBindGroupLayoutsDefferedDebug();

	std::vector<WGPUBindGroup> OnBindGroupsGBuffer();
	WGPUBindGroup createDefferedBindGroup();
	WGPUBindGroup createLightBindGroup();
	WGPUBindGroup createComputeBindGroup();

	void renderUi(const WGPURenderPassEncoder& renderPassEncoder);
	float randomFloat(float min, float max);

	bool m_initUi = true;
	bool m_drawUi = true;
	bool m_debug = false;
	int m_numLights = 128;

	Camera m_camera;
	TrackBall m_trackball;
	AssimpModel m_dragon;
	Shape m_quad;

	WgpBuffer m_uniformBuffer, m_cameraBuffer, m_lightBuffer, m_configBuffer, m_extentBuffer;
	WgpModel m_wgpDragon, m_wgpQuad;
	WgpTexture m_normalTexture, m_albedoTexture, m_depthTexture;
	WGPUBindGroup m_defferedBindGroup, m_lightBindGroup, m_computeBindGroup;

	std::vector<WGPURenderPassColorAttachment> renderPassColorAttachments;
	WGPURenderPassDepthStencilAttachment renderPassDepthStencilAttachment;

	static glm::vec3& RotateY(glm::vec3& p, float rad, const glm::vec3& centerOfRotation = glm::vec3(0.0f, 0.0f, 0.0f));

	static const glm::mat4 IDENTITY;
};