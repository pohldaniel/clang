#pragma once

#include <WebGPU/WgpBuffer.h>
#include <WebGPU/WgpMesh.h>
#include <WebGPU/WgpModel.h>
#include <WebGPU/WgpData.h>

#include <States/StateMachine.h>

#include "Camera.h"
#include "TrackBall.h"
#include "ObjModel.h"

class PrimitivePicking : public State {

	struct Vertex {
		std::array<float, 3> position;
		std::array<float, 3> normal;
		unsigned int primitiveId;
	};

public:

	PrimitivePicking(StateMachine& machine);
	~PrimitivePicking();

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

	std::vector<WGPUBindGroupLayout> OnBindGroupLayoutsPick();
	std::vector<WGPUBindGroupLayout> OnBindGroupLayoutsCompute();
	std::vector<WGPUBindGroupLayout> OnBindGroupLayoutsDebug();

	std::vector<WGPUBindGroup> OnBindGroupsPick();
	WGPUBindGroup createComputeBindGroup();
	WGPUBindGroup createDebugBindGroup();

	void renderUi(const WGPURenderPassEncoder& renderPassEncoder);

	bool m_initUi = true;
	bool m_drawUi = true;
	bool m_debug = false;

	Camera m_camera;
	ObjModel m_teapot;
	Uniforms m_uniforms;
	TrackBall m_trackball;

	WgpModel m_wgpTeapot;
	WgpBuffer m_uniformBuffer, m_computeBuffer, m_stagingBuffer, m_vertexBuffer, m_indexBuffer;

	WGPUBindGroup m_computeBindGroup, m_debugBindGroup;
	WgpTexture m_indexTexture;

	std::vector<WGPURenderPassColorAttachment> renderPassColorAttachments;
	std::vector<Vertex> m_vertices;
	std::vector<unsigned int> m_indices;

	static glm::vec3& RotateY(glm::vec3& p, float rad, const glm::vec3& centerOfRotation = glm::vec3(0.0f, 0.0f, 0.0f));
};