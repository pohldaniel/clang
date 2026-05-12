#pragma once

#include <animation/AnimatedModel.h>
#include <animation/Animation.h>

#include <WebGPU/WgpBuffer.h>
#include <WebGPU/WgpMesh.h>
#include <WebGPU/WgpModel.h>
#include <WebGPU/WgpData.h>

#include <States/StateMachine.h>
#include <Shape/Shape.h>

#include "Camera.h"
#include "Fade.h"

#define MAX_JOIN 96u

class SkinnedMesh : public State {
    enum SelectedAnimation {
        ATTACK,
        SWIM,
		PROCEDURAL
    };
    enum SelectedModel {
        VAMPIRE,
        WHALE
    };
	enum SelectedMode {
		NORMAL,
		JOINTS,
		WEIGHT
	};
public:

	SkinnedMesh(StateMachine& machine);
	~SkinnedMesh();

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
	std::vector<WGPUBindGroupLayout> OnBindGroupLayoutsSkybox();
	std::vector<WGPUBindGroup> OnBindGroupsSkybox();
	
	void renderUi(const WGPURenderPassEncoder& renderPassEncoder);

	bool m_initUi = true;
	bool m_drawUi = true;
	float m_fadeValue = 0.0f;
	float m_speed = 50.0f;
	float m_angle = 0.2f;
	bool m_skinMode = true;

	Camera m_camera;
	Uniforms m_uniforms;
	glm::mat4 m_lightProjection, m_lightView, m_shadow;

    Animation m_attack, m_swim, m_dance;
    AnimatedModel m_whale, m_vampire;
    Shape m_cube;
    Fade m_fade;

    SelectedAnimation m_animation = SelectedAnimation::ATTACK;
    SelectedModel m_model = SelectedModel::WHALE;
	SelectedMode m_mode = SelectedMode::NORMAL;

	WgpBuffer m_uniformBuffer, m_skinBuffer, m_modeBuffer;
    WgpModel m_wgpWhale, m_wgpVampire, m_wgpCube;
	WgpTexture m_wgpTextureCube;

	void proceduralSkinning(Bone**& bones, unsigned short numBones, float angle);
};