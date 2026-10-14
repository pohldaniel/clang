#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_wgpu.h>
#include <imgui_impl_glfw.h>
#include <imgui_internal.h>

#include <WebGPU/WgpContext.h>
#include <WebGPU/WgpRenderer.h>

#include <ui/UiContext.h>

#include <States/Wireframe.h>
#include <States/ImageBasedLighting.h>
#include <States/ShadowMapping.h>
#include <States/SkinnedMesh.h>
#include <States/ComputeParticleLogo.h>
#include <States/PrimitivePicking.h>
#include <States/StencilMask.h>
#include <States/DeferredRendering.h>
#include <States/VolumeRendering.h>
#include <States/OcclusionQuery.h>
#include <States/RenderBundles.h>
#include <States/NuklearUi.h>
#include <States/AudioDecode.h>
#include <States/VideoDecode.h>
#include <States/Cubes.h>
#include <States/Isometric.h>

#include "Menu.h"
#include "Mouse.h"
#include "Application.h"

Menu::Menu(StateMachine& machine) : State(machine, States::MENU) {
	Mouse::instance().attach(Application::Window, false, true);
	LoadingManager::Get().startBackgroundThreads(8);

	uiInitTextureArray();
    m_characterSet.loadFromFile("res/fonts/upheavtt.ttf", 24.0f);
    uiContext.textureViewFont = m_characterSet.texture.getTextureView();
	uiContext.wgpTexture.fillLayer({ 1.0f, 1.0f, 1.0f, 1.0f }, 1u);
    uiInit(static_cast<float>(Application::Width), static_cast<float>(Application::Height));

	float paddingBottom = 3.0f;
	m_uiScene = new Empty();
	m_uiScene->setPadding(20.0f, 20.0f);
	m_uiScene->setSpacing(25.0f, 25.0f);
	m_uiScene->setLayout(Layout::MASONRY);
	m_uiScene->setBorder(10.0f);

	Surface* surface = m_uiScene->addChild<Surface>();
	surface->setColor(glm::vec4(0.2f, 0.2f, 0.2f, 1.0f));
	surface->setPadding(20.0f, 20.0f);
	surface->setSpacing(25.0f, 25.0f);
	surface->setLayout(Layout::GRID);

	Button* button = surface->addChild<Button>();
	button->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	button->setOutlineColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	button->setPosition(0.05f, 0.05f);
	button->setOutlineThickness(5.0f);
	button->setPadding(5.0f, 5.0f);
	button->setOnClick([&]() {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new Wireframe(m_machine));
	});

	Label* label = button->addChild<Label>(m_characterSet);
	label->setText("Wireframe");
	label->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	label->setPadding(0.0f, paddingBottom);

	button = surface->addChild<Button>();
	button->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	button->setOutlineColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	button->setOutlineThickness(5.0f);
	button->setPadding(5.0f, 5.0f);
	button->setOnClick([&]() {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new ImageBasedLighting(m_machine));
	});

	label = button->addChild<Label>(m_characterSet);
	label->setText("Image Based Lighting");
	label->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	label->setPadding(0.0f, paddingBottom);
	
	button = surface->addChild<Button>();
	button->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	button->setOutlineColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	button->setOutlineThickness(5.0f);
	button->setPadding(5.0f, 5.0f);
	button->setOnClick([&]() {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new ShadowMapping(m_machine));
	});

	label = button->addChild<Label>(m_characterSet);
	label->setText("Shadow Mapping");
	label->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	label->setPadding(0.0f, paddingBottom);

	surface = m_uiScene->addChild<Surface>();
	surface->setColor(glm::vec4(0.2f, 0.2f, 0.2f, 1.0f));
	surface->setPadding(20.0f, 20.0f);
	surface->setSpacing(25.0f, 25.0f);
	surface->setLayout(Layout::GRID);

	button = surface->addChild<Button>();
	button->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	button->setOutlineColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	button->setOutlineThickness(5.0f);
	button->setPadding(5.0f, 5.0f);
	button->setOnClick([&]() {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new SkinnedMesh(m_machine));
	});

	label = button->addChild<Label>(m_characterSet);
	label->setText("Skinned Mesh");
	label->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	label->setPadding(0.0f, paddingBottom);

	button = surface->addChild<Button>();
	button->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	button->setOutlineColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	button->setOutlineThickness(5.0f);
	button->setPadding(5.0f, 5.0f);
	button->setOnClick([&]() {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new ComputeParticleLogo(m_machine));
	});

	label = button->addChild<Label>(m_characterSet);
	label->setText("Compute Particle Logo");
	label->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	label->setPadding(0.0f, paddingBottom);

	button = surface->addChild<Button>();
	button->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	button->setOutlineColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	button->setOutlineThickness(5.0f);
	button->setPadding(5.0f, 5.0f);
	button->setOnClick([&]() {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new PrimitivePicking(m_machine));
	});

	label = button->addChild<Label>(m_characterSet);
	label->setText("Primitive Picking");
	label->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	label->setPadding(0.0f, paddingBottom);

	button = surface->addChild<Button>();
	button->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	button->setOutlineColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	button->setOutlineThickness(5.0f);
	button->setPadding(5.0f, 5.0f);
	button->setOnClick([&]() {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new StencilMask(m_machine));
	});

	label = button->addChild<Label>(m_characterSet);
	label->setText("Stencil Mask");
	label->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	label->setPadding(0.0f, paddingBottom);

	surface = m_uiScene->addChild<Surface>();
	surface->setColor(glm::vec4(0.2f, 0.2f, 0.2f, 1.0f));
	surface->setPadding(20.0f, 20.0f);
	surface->setSpacing(25.0f, 25.0f);
	surface->setLayout(Layout::GRID);

	button = surface->addChild<Button>();
	button->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	button->setOutlineColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	button->setOutlineThickness(5.0f);
	button->setPadding(5.0f, 5.0f);
	button->setOnClick([&]() {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new DeferredRendering(m_machine));
	});

	label = button->addChild<Label>(m_characterSet);
	label->setText("Defferred Rendering");
	label->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	label->setPadding(0.0f, paddingBottom);

	button = surface->addChild<Button>();
	button->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	button->setOutlineColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	button->setOutlineThickness(5.0f);
	button->setPadding(5.0f, 5.0f);
	button->setOnClick([&]() {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new VolumeRendering(m_machine));
	});

	label = button->addChild<Label>(m_characterSet);
	label->setText("Volume Rendering");
	label->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	label->setPadding(0.0f, paddingBottom);

	button = surface->addChild<Button>();
	button->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	button->setOutlineColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	button->setOutlineThickness(5.0f);
	button->setPadding(5.0f, 5.0f);
	button->setOnClick([&]() {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new OcclusionQuery(m_machine));
	});

	label = button->addChild<Label>(m_characterSet);
	label->setText("Occlusion Query");
	label->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	label->setPadding(0.0f, paddingBottom);

	button = surface->addChild<Button>();
	button->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	button->setOutlineColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	button->setOutlineThickness(5.0f);
	button->setPadding(5.0f, 5.0f);
	button->setOnClick([&]() {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new RenderBundles(m_machine));
	});

	label = button->addChild<Label>(m_characterSet);
	label->setText("Render Bundles");
	label->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	label->setPadding(0.0f, paddingBottom);

	surface = m_uiScene->addChild<Surface>();
	surface->setColor(glm::vec4(0.2f, 0.2f, 0.2f, 1.0f));
	surface->setPadding(20.0f, 20.0f);
	surface->setSpacing(25.0f, 25.0f);
	surface->setLayout(Layout::GRID);

	button = surface->addChild<Button>();
	button->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	button->setOutlineColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	button->setOutlineThickness(5.0f);
	button->setPadding(5.0f, 5.0f);
	button->setOnClick([&]() {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new NuklearUi(m_machine));
	});

	label = button->addChild<Label>(m_characterSet);
	label->setText("Nuklear UI");
	label->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	label->setPadding(0.0f, paddingBottom);

	button = surface->addChild<Button>();
	button->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	button->setOutlineColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	button->setOutlineThickness(5.0f);
	button->setPadding(5.0f, 5.0f);
	button->setOnClick([&]() {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new AudioDecode(m_machine));
	});

	label = button->addChild<Label>(m_characterSet);
	label->setText("Audio Decode");
	label->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	label->setPadding(0.0f, paddingBottom);

	button = surface->addChild<Button>();
	button->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	button->setOutlineColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	button->setOutlineThickness(5.0f);
	button->setPadding(5.0f, 5.0f);
	button->setOnClick([&]() {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new VideoDecode(m_machine));
	});

	label = button->addChild<Label>(m_characterSet);
	label->setText("Video Decode");
	label->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	label->setPadding(0.0f, paddingBottom);

	button = surface->addChild<Button>();
	button->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	button->setOutlineColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	button->setOutlineThickness(5.0f);
	button->setPadding(5.0f, 5.0f);
	button->setOnClick([&]() {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new Cubes(m_machine));
	});

	label = button->addChild<Label>(m_characterSet);
	label->setText("Cubes");
	label->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	label->setPadding(0.0f, paddingBottom);

	surface = m_uiScene->addChild<Surface>();
	surface->setColor(glm::vec4(0.2f, 0.2f, 0.2f, 1.0f));
	surface->setPadding(20.0f, 20.0f);
	surface->setSpacing(15.0f, 0.0f);

	button = surface->addChild<Button>();
	button->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	button->setOutlineColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	button->setOutlineThickness(5.0f);
	button->setPadding(5.0f, 5.0f);
	button->setOnClick([&]() {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new Isometric(m_machine));
	});

	label = button->addChild<Label>(m_characterSet);
	label->setText("Isomeric");
	label->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	label->setPadding(0.0f, paddingBottom);

	button = surface->addChild<Button>();
	button->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	button->setOutlineColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	button->setOutlineThickness(5.0f);
	button->setPadding(5.0f, 5.0f);
	button->setOnClick([&]() {
		int randomSeed = rand();
		auto* noiseTask = new NoiseGenerationTask(512, 512, randomSeed, [this]() {

		});
		LoadingManager::Get().addTask(noiseTask);
	});

	label = button->addChild<Label>(m_characterSet);
	label->setText("Noise");
	label->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	label->setPadding(0.0f, paddingBottom);

	Canvas* canvas = surface->addChild<Canvas>();
	canvas->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	canvas->setWidth(400.0f);
	canvas->setHeight(400.0f);
	canvas->setLayer(1u);

	if (m_layout == SelectedLayout::M_GRID) {
		const std::vector<Surface*>& surfaces = m_uiScene->getChildren<Surface>();
		for (auto& surface : surfaces) {
			surface->setLayout(Layout::GRID);
		}
		surfaces.back()->setLayout(Layout::HORIZONTAL);
		m_uiScene->setLayout(Layout::MASONRY);
		m_uiScene->updateLayout(true);
	}

	if (m_layout == SelectedLayout::M_VERTICAL) {
		const std::vector<Surface*>& surfaces = m_uiScene->getChildren<Surface>();
		for (auto& surface : surfaces) {
			surface->setLayout(Layout::VERTICAL);			
		}
		m_uiScene->setLayout(Layout::HORIZONTAL);
		m_uiScene->updateLayout(true);
	}

	if (m_layout == SelectedLayout::M_HORIZONTAL) {
		const std::vector<Surface*>& surfaces = m_uiScene->getChildren<Surface>();
		for (auto& surface : surfaces) {
			surface->setLayout(Layout::HORIZONTAL);
			
		}
		m_uiScene->setLayout(Layout::VERTICAL);
		m_uiScene->updateLayout(true);
	}

	wgpContext.setClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
	wgpContext.OnDraw = std::bind(&Menu::OnDraw, this, std::placeholders::_1, std::placeholders::_2);
}

Menu::~Menu() {

}

void Menu::fixedUpdate() {

}

void Menu::update() {
	LoadingManager::Get().update();
	Mouse &mouse = Mouse::instance();	
	m_uiScene->input(mouse.xPos(), mouse.yPos(), mouse.buttonDown(GLFW_MOUSE_BUTTON_LEFT));
	m_uiScene->draw();
}

void Menu::render() {
	wgpDraw();
}

void Menu::OnDraw(const WGPUCommandEncoder& commandEncoder, const WGPURenderPassDescriptor& renderPassDescriptor) {
	uiDraw(commandEncoder, renderPassDescriptor);
}

void Menu::OnMouseMotion(const Event::MouseMoveEvent& event) {

}

void Menu::OnMouseButtonDown(const Event::MouseButtonEvent& event) {	
	if (event.button == Event::MouseButtonEvent::BUTTON_LEFT) {
		Mouse::instance().attach(Application::Window, false, true);
	}

	if (event.button == Event::MouseButtonEvent::BUTTON_RIGHT)
		Mouse::instance().attach(Application::Window, true, true, true);
}

void Menu::OnMouseButtonUp(const Event::MouseButtonEvent& event) {
	if (event.button == Event::MouseButtonEvent::BUTTON_LEFT) {
		Mouse::instance().attach(Application::Window, false, true);
	} 

	if (event.button == Event::MouseButtonEvent::BUTTON_RIGHT)
		Mouse::instance().attach(Application::Window, false, false, true);
}

void Menu::OnScroll(const Event::MouseWheelEvent& event) {
	m_uiScene->wheelInput(event.x, event.y, event.delta);
}

void Menu::OnKeyDown(const Event::KeyboardEvent& event) {
	if (event.keyCode == GLFW_KEY_ESCAPE) {
		uiShutDown();
		wgpCleanState();
		m_isRunning = false;
	}
}

void Menu::OnKeyUp(const Event::KeyboardEvent& event) {

}

void Menu::resize(int deltaW, int deltaH) {
	uiResize(static_cast<float>(Application::Width), static_cast<float>(Application::Height));
}

void Menu::renderUi(const WGPURenderPassEncoder& renderPassEncoder) {
	ImGui_ImplWGPU_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
		ImGuiWindowFlags_NoBackground;

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("InvisibleWindow", nullptr, windowFlags);
	ImGui::PopStyleVar(3);

	ImGuiID dockSpaceId = ImGui::GetID("MainDockSpace");
	ImGui::DockSpace(dockSpaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
	ImGui::End();

	if (m_initUi) {
		m_initUi = false;
		ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Left, 0.2f, nullptr, &dockSpaceId);
		ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Right, 0.2f, nullptr, &dockSpaceId);
		ImGuiID dock_id_down = ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Down, 0.2f, nullptr, &dockSpaceId);
		ImGuiID dock_id_up = ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Up, 0.2f, nullptr, &dockSpaceId);
		ImGui::DockBuilderDockWindow("Settings", dock_id_left);
	}

	ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	
	ImGui::End();

	ImGui::Render();
	ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), renderPassEncoder);
}

float hash2D(int x, int y, int seed) {

	int n = x + y * 57 + seed * 131;
	n = (n << 13) ^ n;
	return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}

float bilinearInterpolate(float x, float y, int seed) {
	int intX = static_cast<int>(floor(x));
	int intY = static_cast<int>(floor(y));
	float fracX = x - intX;
	float fracY = y - intY;

	float v1 = hash2D(intX, intY, seed);
	float v2 = hash2D(intX + 1, intY, seed);
	float v3 = hash2D(intX, intY + 1, seed);
	float v4 = hash2D(intX + 1, intY + 1, seed);

	float fadeX = fracX * fracX * (3.0f - 2.0f * fracX);
	float fadeY = fracY * fracY * (3.0f - 2.0f * fracY);

	float i1 = v1 + fadeX * (v2 - v1);
	float i2 = v3 + fadeX * (v4 - v3);
	return i1 + fadeY * (i2 - i1);
}


float generateFractalNoise(float x, float y, int seed, int octaves = 4) {
	float total = 0.0f;
	float amplitude = 1.0f;
	float frequency = 0.05f;
	float maxValue = 0.0f;

	for (int i = 0; i < octaves; ++i) {
		total += bilinearInterpolate(x * frequency, y * frequency, seed) * amplitude;
		maxValue += amplitude;
		amplitude *= 0.5f;
		frequency *= 2.0f;
	}

	return total / maxValue;
}

void NoiseGenerationTask::OnProcess() {
	m_pixelData.resize(m_width * m_height * 4);

	for (int y = 0; y < m_height; ++y) {
		for (int x = 0; x < m_width; ++x) {
			float noiseValue = generateFractalNoise(static_cast<float>(x), static_cast<float>(y), m_seed, 5);
			unsigned char byteValue = static_cast<unsigned char>((noiseValue + 1.0f) * 0.5f * 255.0f);

			int index = (y * m_width + x) * 4;
			m_pixelData[index + 0] = byteValue;
			m_pixelData[index + 1] = byteValue;
			m_pixelData[index + 2] = byteValue;
			m_pixelData[index + 3] = 255;
		}
	}
}

void NoiseGenerationTask::OnComplete() {
	WGPUTexelCopyTextureInfo copyDst = {};
	copyDst.texture = uiContext.wgpTexture.getTexture();
	copyDst.mipLevel = 0u;
	copyDst.origin = { 0u, 0u, 1u };
	copyDst.aspect = WGPUTextureAspect_All;

	WGPUTexelCopyBufferLayout dataLayout = {};
	dataLayout.offset = 0u;
	dataLayout.bytesPerRow = m_width * 4u;
	dataLayout.rowsPerImage = m_height;

	WGPUExtent3D writeSize = { static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height), 1u };

	wgpuQueueWriteTexture(
		wgpContext.queue,
		&copyDst,
		m_pixelData.data(),
		m_pixelData.size(),
		&dataLayout,
		&writeSize
	);

	if (m_onUploadComplete) {
		m_onUploadComplete();
	}

	delete this;
}