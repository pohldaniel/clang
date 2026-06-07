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

class OcclusionQuery : public State {

	struct Scene {
		WgpBuffer uniformBuffer;
		WgpModel model;
		bool isVisible;
		std::array<float, 4> color;
	};

public:

	OcclusionQuery(StateMachine& machine);
	~OcclusionQuery();

	void fixedUpdate() override;
	void update() override;
	void render() override;
	void OnDraw(const WGPUCommandEncoder& commandEncoder, const WGPURenderPassDescriptor& renderPassDescriptor);
	void OnPostDraw();

	void OnMouseMotion(const Event::MouseMoveEvent& event) override;
	void OnScroll(double xoffset, double yoffset) override;
	void OnMouseButtonDown(const Event::MouseButtonEvent& event) override;
	void OnMouseButtonUp(const Event::MouseButtonEvent& event) override;
	void OnKeyDown(const Event::KeyboardEvent& event) override;
	void OnKeyUp(const Event::KeyboardEvent& event) override;
	void resize(int deltaW, int deltaH) override;

private:

	std::vector<WGPUBindGroupLayout> OnBindGroupLayouts();
	WGPUQuerySet createQuerySet();

	void renderUi(const WGPURenderPassEncoder& renderPassEncoder);

	bool m_initUi = true;
	bool m_drawUi = true;
	bool m_animate = true;
	float m_time = 0.0f;

	Camera m_camera;
	TrackBall m_trackball;
	Uniforms m_uniforms;
	Shape m_cube;

	WgpBuffer m_uniformBuffer;
	WgpBuffer m_resolveBuffer, m_resultBuffer;
	WGPUQuerySet m_querySet;
	std::vector<Scene> m_scenes;

	static float PingPongSine(float t);
	static void InitScene(Scene& scene, Shape& shape, const WgpBuffer& uniformBuffer, std::array<float, 4> color);
	static void UpdateScene(Scene& scene, const glm::vec3& position, float time);
	static void OnMapBuffer(WGPUMapAsyncStatus status, WGPUStringView message, void* userdata1, void* userdata2);
};