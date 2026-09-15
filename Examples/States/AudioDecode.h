#pragma once

#include <Nuklear/NkContext.h>
#include <WebGPU/WgpData.h>
#include <Sound/AudioDecoder.h>
#include <States/StateMachine.h>

#include "Camera.h"
#include "TrackBall.h"
#include "Transform.h"

class AudioDecode : public State {
	
public:

	AudioDecode(StateMachine& machine);
	~AudioDecode();

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

	void renderUi(const WGPURenderPassEncoder& renderPassEncoder);

	bool m_initUi = true;
	bool m_drawUi = false;

	Camera m_camera;
	Uniforms m_uniforms;
	TrackBall m_trackball;
	AudioDecoder m_audioDecoder;

	float btn_w;
    float btn_h;
    float spacing;
    float start_x;
    float total_block_h;
    float start_y;

    float ctrl_size;
    float side_padding;

    float bottom_margin;
    float ctrl_y;
    float play_x;
    float pause_x;

    int m_currentSong = 0;
    bool m_isPressed = false;
};