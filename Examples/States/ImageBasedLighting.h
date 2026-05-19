#pragma once

#include <functional>
#include <WebGPU/WgpBuffer.h>
#include <WebGPU/WgpModel.h>
#include <WebGPU/WgpTexture.h>
#include <WebGPU/WgpData.h>

#include <States/StateMachine.h>
#include <Shape/Shape.h>

#include "Camera.h"
#include "TrackBall.h"
#include "AssimpModel.h"

class ImageBasedLighting : public State {

enum Scene {
	SPHERE,
	HELMET
};

public:

	ImageBasedLighting(StateMachine& machine);
	~ImageBasedLighting();

	void fixedUpdate() override;
	void update() override;
	void render() override;

	void OnDraw(const WGPUCommandEncoder& commandEncoder, const WGPURenderPassDescriptor& renderPassDescriptor);

	void OnMouseMotion(const Event::MouseMoveEvent& event) override;
	void OnMouseButtonDown(const Event::MouseButtonEvent& event) override;
	void OnMouseButtonUp(const Event::MouseButtonEvent& event) override;
	void OnScroll(double xoffset, double yoffset) override;
	void OnKeyDown(const Event::KeyboardEvent& event) override;
	void OnKeyUp(const Event::KeyboardEvent& event) override;
    void resize(int deltaW, int deltaH) override;
	
private:

	std::vector<WGPUBindGroupLayout> OnBindGroupLayoutsPBR();
	std::vector<WGPUBindGroup> OnBindGroupsPBR();
	std::vector<WGPUBindGroupLayout> OnBindGroupLayoutsPBRHelmet();
	std::vector<WGPUBindGroup> OnBindGroupsPBRHelmet();

	std::vector<WGPUBindGroupLayout> OnBindGroupLayoutsCube();
	std::vector<WGPUBindGroup> OnBindGroupsCube();
	std::vector<WGPUBindGroupLayout> OnBindGroupLayoutsIrradiance();
	std::vector<WGPUBindGroup> OnBindGroupsIrradiance();
	std::vector<WGPUBindGroupLayout> OnBindGroupLayoutsPrefilter();
	std::vector<WGPUBindGroup> OnBindGroupsPrefilter();
	std::vector<WGPUBindGroupLayout> OnBindGroupLayoutsSkybox();
	std::vector<WGPUBindGroup> OnBindGroupsSkybox();
	std::vector<WGPUBindGroup> OnBindGroupsSkyboxHelmet();

	void OnDrawBrdf(const WGPURenderPassEncoder& renderPassEncoder, uint32_t layer, uint32_t mip);
	void OnDrawCube(const WGPURenderPassEncoder& renderPassEncoder, uint32_t layer, uint32_t mip);
	void OnDrawIrradiance(const WGPURenderPassEncoder& renderPassEncoder, uint32_t layer, uint32_t mip);
	void OnDrawPrefilter(const WGPURenderPassEncoder& renderPassEncoder, uint32_t layer, uint32_t mip);

	void applyTransformation(const TrackBall& arc);
	void renderUi(const WGPURenderPassEncoder& renderPassEncoder);
	void initMatrices();
	void initLights();
	void initIrradianceMatrices();

	bool m_initUi = true;
	bool m_drawUi = true;
	const uint32_t ROUGHNESS_LEVELS = 5u;

	Camera m_camera;
	TrackBall m_trackball;
	AssimpModel m_helmet;
	Uniforms m_uniforms;
	MaterialUniforms m_material;
	PBRLightingUniforms m_lights[4];

	Shape m_cube, m_sphere, m_quad;
	WgpBuffer m_uniformBuffer, m_uniformModelBuffer, m_uniformLightBuffer, m_uniformMVPBuffer, m_roughnessBuffer, m_uniformMaterial, m_instanceBuffer;
	WgpModel m_wgpCube, m_wgpSphere, m_wgpQuad, m_wgpHelmet;
	WgpTexture m_wgpTextureHDR, m_wgpTextureCube, m_wgpTextureIrradiance, m_wgpTexturePrefilter, m_wgpTextureBrdf, m_wgpTextureShadow;
	WgpTexture m_wgpTextutreNormal, m_wgpTextutreEmission, m_wgpTextutreMetalness, m_wgpTextutreLightmap;

	glm::mat4 m_models[12];	
	glm::mat4 m_mvpInvCube[6];
	glm::mat4 m_mvpCube[6];
	glm::mat4 lightProjection, lightView, shadow;

	Scene m_scene = Scene::HELMET;
};