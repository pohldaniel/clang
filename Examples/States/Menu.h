#pragma once

#include <WebGPU/WgpBuffer.h>
#include <WebGPU/WgpTexture.h>
#include <WebGPU/WgpModel.h>
#include <WebGPU/WgpData.h>

#include <States/StateMachine.h>

#include <ui/Empty.h>
#include <ui/Surface.h>
#include <ui/Button.h>
#include <ui/Label.h>
#include <ui/Canvas.h>

#include <thread/ILoadingListener.h>
#include <thread/LoadingManager.h>

class NoiseGenerationTask : public ILoadingListener {
public:

    NoiseGenerationTask(int width, int height, int seed,  std::function<void()> onUploadComplete)
        : m_width(width), m_height(height), m_seed(seed), m_onUploadComplete(onUploadComplete) {
    }

    void OnProcess() override;
	void OnComplete() override;

private:

    int m_width;
    int m_height;
    int m_seed;
    std::function<void()> m_onUploadComplete;
    std::vector<unsigned char> m_pixelData;
};

class Menu : public State {
	enum SelectedLayout {
		M_HORIZONTAL,
		M_VERTICAL,
		M_GRID
	};

public:

	Menu(StateMachine& machine);
	~Menu();

	void fixedUpdate() override;
	void update() override;
	void render() override;
	void OnDraw(const WGPUCommandEncoder& commandEncoder, const WGPURenderPassDescriptor& renderPassDescriptor);

	void OnMouseMotion(const Event::MouseMoveEvent& event) override;
	void OnScroll(const Event::MouseWheelEvent& event) override;
	void OnMouseButtonDown(const Event::MouseButtonEvent& event) override;
	void OnMouseButtonUp(const Event::MouseButtonEvent& event) override;
	void OnKeyDown(const Event::KeyboardEvent& event) override;
	void OnKeyUp(const Event::KeyboardEvent& event) override;
	void resize(int deltaW, int deltaH) override;

private:

	void renderUi(const WGPURenderPassEncoder& renderPassEncoder);

	bool m_initUi = true;
	bool m_drawUi = false;

	Empty* m_uiScene;
	CharacterSet m_characterSet;
	SelectedLayout m_layout = SelectedLayout::M_GRID;
};