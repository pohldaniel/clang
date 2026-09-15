#pragma once

#include <Nuklear/NkContext.h>
#include <WebGPU/WgpData.h>
#include <Video/VideoDecoder.h>
#include <States/StateMachine.h>

#include "Camera.h"
#include "TrackBall.h"
#include "Transform.h"

class VideoDecode : public State {
	
public:

	VideoDecode(StateMachine& machine);
	~VideoDecode();

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
    WGPUBindGroup createBindGroup();
	void renderUi(const WGPURenderPassEncoder& renderPassEncoder);

	bool m_initUi = true;
	bool m_drawUi = false;
    Camera m_camera;
	TrackBall m_trackball;
    VideoDecoder m_videoDecoder;
	
    float ctrl_size ;
    float side_padding ;

    float bottom_margin;
    float ctrl_y;
    float play_x;
    float pause_x;
    bool m_isPressed = false;
};