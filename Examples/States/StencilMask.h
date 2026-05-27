#pragma once

#include <WebGPU/WgpBuffer.h>
#include <WebGPU/WgpModel.h>
#include <WebGPU/WgpData.h>

#include <States/StateMachine.h>
#include <Shape/Shape.h>

#include "Camera.h"
#include "TrackBall.h"
#include "Transform.h"

class StencilMask : public State {

	struct Object {
		float uniformValues[16 + 4];
		WgpBuffer uniformBuffer;
		WGPUBindGroup bindGroup;
		uint32_t geometryIndex;
	};

	struct Scene {
		uint32_t numObjects;
		WgpBuffer sharedUniformBuffer;
		float sharedUniformValues[16 + 4];
		std::vector<Object> objects;		
	};

public:

	StencilMask(StateMachine& machine);
	~StencilMask();

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

	std::vector<WGPUBindGroupLayout> OnBindGroupLayoutsStencil();
	void renderUi(const WGPURenderPassEncoder& renderPassEncoder);
	void updateSceneMask(float time, Scene& scene, size_t index, float rotation[3]);
	void draw(const WGPUCommandEncoder& commandEncoder, const WGPURenderPassDescriptor& renderPassDescriptor, const Scene& scene, uint32_t stencilRef, const WGPURenderPipeline& renderPipeline);

	bool m_initUi = true;
	bool m_drawUi = false;

	Camera m_camera;
	TrackBall m_trackball;
	Shape m_quad, m_sphere, m_cube, m_jem, m_cylinder, m_cone, m_torus, m_dice;

	std::vector<Scene> m_maskScenes;
	std::vector<Scene> m_scenes;
	std::vector<WgpModel> m_wgpModels;

	static void InitScene(Scene& scene, uint32_t numInstances, float hue, uint32_t geometryIndex, uint32_t geometryIndexCount);	
	static void UpdateScene0(float time, Scene& scene);
	static void UpdateScene1(float time, Scene& scene);

	static void HslToTgba(float h, float s, float l, float* rgba);
	static float Randf(float min_val, float max_val);
	static uint32_t RandElem(uint32_t count);
};