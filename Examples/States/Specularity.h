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

class Specularity : public State {
	struct CameraState {
		glm::vec2 angles = { 0.8f, 0.5f };
		float zoom = -1.2f;
	};

	struct DragState {
		bool active = false;
		glm::vec2 startMouse;
		CameraState startCameraState;
		glm::vec2 velocity = { 0.0, 0.0 };
		glm::vec2 previousDelta;
		float intertia = 0.9f;
		float sensitivity = 0.01f;
		float scrollSensitivity = 0.1f;
	};

public:

	Specularity(StateMachine& machine);
	~Specularity();

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
	void updateViewMatrix();
	void updateLightingUniforms();
	void updateDragInertia();

	bool m_initUi = true;
	bool m_drawUi = true;

	Camera m_camera;
	ObjModel m_boat;
	WgpBuffer m_uniformBuffer, m_uniformLigthBuffer;
	WgpModel m_wgpBoat;
	Uniforms m_uniforms;

	CameraState m_cameraState;
	DragState m_drag;
	LightingUniforms m_lightingUniforms;
	bool m_updateLight = true;
};