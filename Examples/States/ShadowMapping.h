#pragma once

#include <WebGPU/WgpBuffer.h>
#include <WebGPU/WgpMesh.h>
#include <WebGPU/WgpModel.h>
#include <WebGPU/WgpData.h>

#include <States/StateMachine.h>
#include <Shape/Shape.h>

#include "Camera.h"
#include "AssimpModel.h"

class ShadowMapping : public State {

public:

	ShadowMapping(StateMachine& machine);
	~ShadowMapping();

	void fixedUpdate() override;
	void update() override;
	void render() override;
	void OnDraw(const WGPURenderPassEncoder& renderPass);

	void OnMouseMotion(const Event::MouseMoveEvent& event) override;
	void OnMouseButtonDown(const Event::MouseButtonEvent& event) override;
	void OnMouseButtonUp(const Event::MouseButtonEvent& event) override;
	void OnScroll(double xoffset, double yoffset) override;
	void OnKeyDown(const Event::KeyboardEvent& event) override;
	void OnKeyUp(const Event::KeyboardEvent& event) override;
	void resize(int deltaW, int deltaH) override;

private:

	std::vector<WGPUBindGroupLayout> OnBindGroupLayouts();
	std::vector<WGPUBindGroup> OnBindGroups();
	std::vector<WGPUBindGroupLayout> OnBindGroupLayoutsShadow();
	std::vector<WGPUBindGroup> OnBindGroupsShadow();
	void OnDrawShadow(const WGPURenderPassEncoder& renderPassEncoder);

	void renderUi(const WGPURenderPassEncoder& renderPassEncoder);

	bool m_initUi = true;
	bool m_drawUi = false;

	Camera m_camera;
	Uniforms m_uniforms;
	AssimpModel m_dragon;
	Shape m_quad;
	glm::mat4 m_lightProjection, m_lightView, m_shadow;

	WgpModel m_wgpDragon, m_wgpQuad;
	WgpBuffer m_uniformBuffer;
	WgpTexture m_wgpTextureShadow;

	static glm::vec3& RotateY(glm::vec3& p, float rad, const glm::vec3& centerOfRotation = glm::vec3(0.0f, 0.0f, 0.0f));
};