#pragma once

#include <WebGPU/WgpBuffer.h>
#include <WebGPU/WgpTexture.h>
#include <WebGPU/WgpModel.h>
#include <WebGPU/WgpData.h>

#include <animation/AnimationController.h>
#include <animation/AnimatedModel.h>
#include <animation/Animation.h>

#include <scene/SceneNode.h>
#include <scene/CollisionNode.h>

#include <States/StateMachine.h>
#include <Nuklear/NkJoystick.h>
#include <Shape/Shape.h>

#include "AssimpModel.h"
#include "Camera.h"
#include "TrackBall.h"
#include "Transform.h"
#include "bullet_store.h"

#define ARRAY_SIZE_Y 5
#define ARRAY_SIZE_X 5
#define ARRAY_SIZE_Z 5

class Cubes : public State {
	struct GPUInstanceData {
		glm::mat4 modelMatrix;
		glm::vec4 color;
	};

	glm::vec4 colors[4] = { 
						{244.0f / 256.0f, 194.0f / 256.0f, 13.0f / 256., 1.0f},
						{219.0f / 256.0f, 50.0f / 256.0f, 54.0f / 256., 1.0f},
						{72.0f / 256.0f, 133.0f / 256.0f, 237.0f / 256., 1.0f},
						{ 60.0f / 256.0f,  186.0f / 256.0f,84.0f / 256.0f, 1.0f }
	};

public:

	Cubes(StateMachine& machine);
	~Cubes();

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

	std::vector<WGPUBindGroupLayout> OnBindGroupLayouts();
	std::vector<WGPUBindGroup> OnBindGroups();
	void renderUi(const WGPURenderPassEncoder& renderPassEncoder);
	void shootCube(unsigned int posX, unsigned int posY);

	bool m_initUi = true;
	bool m_drawUi = false;
	bool m_debugPhysic = false;

	Camera m_camera;
	Uniforms m_uniforms;	
	TrackBall m_trackball;
	SceneNode* m_scene;
	Shape m_cube;
	std::vector<GPUInstanceData> m_cpuInstanceBuffer;

	WgpBuffer m_uniformBuffer, m_storageBuffer;
	WgpModel m_wgpCube;

	WGPUBindGroup m_bindGroup;

	std::vector<SceneNode*> m_children;
};