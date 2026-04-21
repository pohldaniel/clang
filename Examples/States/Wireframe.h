#include <functional>
#include <States/StateMachine.h>
#include <WebGPU/WgpTexture.h>
#include <WebGPU/WgpBuffer.h>
#include <WebGPU/WgpMesh.h>
#include <WebGPU/WgpData.h>
#include <WebGPU/WgpModel.h>
#include "Camera.h"
#include "TrackBall.h"
#include "ObjModel.h"

enum SelectedModel {
	MAMMOTH,
	DRAGON
};

class Wireframe : public State {

public:    

    Wireframe(StateMachine& machine);
	~Wireframe();

	void fixedUpdate() override;
	void update() override;
	void render() override;
	void OnDraw(const WGPURenderPassEncoder& renderPass);

	void OnMouseMotion(const Event::MouseMoveEvent& event) override;
	void OnMouseButtonDown(const Event::MouseButtonEvent& event) override;
	void OnMouseButtonUp(const Event::MouseButtonEvent& event) override;
	void OnKeyDown(const Event::KeyboardEvent& event) override;
	void OnKeyUp(const Event::KeyboardEvent& event) override;
    void resize(int deltaW, int deltaH) override;
	
private:

	std::vector<WGPUBindGroupLayout> OnBindGroupLayoutsPTN();
	std::vector<WGPUBindGroupLayout> OnBindGroupLayoutsWF();

	std::vector<WGPUBindGroup> OnBindGroups();
	std::vector<WGPUBindGroup> OnBindGroupsWF();

	void applyTransformation(const TrackBall& arc);
	void renderUi(const WGPURenderPassEncoder& renderPassEncoder);
	
	bool m_initUi = true;
	bool m_drawUi = true;
	SelectedModel m_model = SelectedModel::DRAGON;
	Camera m_camera;
	ObjModel m_mammoth, m_dragon;
	TrackBall m_trackball;
	Uniforms m_uniforms;
	
	WgpBuffer m_uniformBuffer;
	WgpModel m_wgpDragon, m_wgpMammoth;
	
	static void AddBindgroups(const WgpModel& model);
};