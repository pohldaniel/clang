#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_wgpu.h>
#include <imgui_impl_glfw.h>
#include <imgui_internal.h>

#include <WebGPU/WgpContext.h>
#include <WebGPU/WgpRenderer.h>

#include <ui/UiContext.h>

#include "Menu.h"
#include "Mouse.h"
#include "Application.h"

Menu::Menu(StateMachine& machine) : State(machine, States::MENU) {
	Mouse::instance().attach(Application::Window, false, true);

    m_characterSet.loadFromFile("res/fonts/upheavtt.ttf", 24.0f);
    uiContext.textureView = m_characterSet.texture.getTextureView();
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
	/*button->setOnClick([&]() {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new Wireframe(m_machine));
	});*/

	Label* label = button->addChild<Label>(m_characterSet);
	label->setText("Wireframe");
	label->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	label->setPadding(0.0f, paddingBottom);

	button = surface->addChild<Button>();
	button->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	button->setOutlineColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	button->setPosition(0.05f, 0.35f);
	button->setOutlineThickness(5.0f);
	button->setPadding(5.0f, 5.0f);
	/*button->setOnClick([&]() {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new Compute(m_machine));
	});*/

	label = button->addChild<Label>(m_characterSet);
	label->setText("Compute");
	label->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	label->setPadding(0.0f, paddingBottom);

	button = surface->addChild<Button>();
	button->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	button->setOutlineColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	button->setPosition(0.35f, 0.35f);
	button->setOutlineThickness(5.0f);
	button->setPadding(5.0f, 5.0f);
	/*button->setOnClick([&]() {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new Specularity(m_machine));
	});*/

	label = button->addChild<Label>(m_characterSet);
	label->setText("Specularity");
	label->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	label->setPadding(0.0f, paddingBottom);

	button = surface->addChild<Button>();
	button->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	button->setOutlineColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	button->setPosition(0.05f, 0.5f);
	button->setOutlineThickness(5.0f);
	button->setPadding(5.0f, 5.0f);
	/*button->setOnClick([&]() {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new NormalMap(m_machine));
	});*/

	label = button->addChild<Label>(m_characterSet);
	label->setText("Normal Map");
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
	/*button->setOnClick([&]() {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new MSDFFont(m_machine));
	});*/

	label = button->addChild<Label>(m_characterSet);
	label->setText("MSDF Font");
	label->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	label->setPadding(0.0f, paddingBottom);

	button = surface->addChild<Button>();
	button->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	button->setOutlineColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	button->setOutlineThickness(5.0f);
	button->setPadding(5.0f, 5.0f);
	/*button->setOnClick([&]() {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new InstancedCube(m_machine));
	});*/

	label = button->addChild<Label>(m_characterSet);
	label->setText("Instanced Cube");
	label->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	label->setPadding(5.0f, paddingBottom);

	button = surface->addChild<Button>();
	button->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	button->setOutlineColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	button->setOutlineThickness(5.0f);
	button->setPadding(5.0f, 5.0f);
	/*button->setOnClick([&]() {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new ImageBasedLighting(m_machine));
	});*/

	label = button->addChild<Label>(m_characterSet);
	label->setText("Image Based Lighting");
	label->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	label->setPadding(0.0f, paddingBottom);
	
	button = surface->addChild<Button>();
	button->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	button->setOutlineColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	button->setOutlineThickness(5.0f);
	button->setPadding(5.0f, 5.0f);
	/*button->setOnClick([&]() {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new ShadowMapping(m_machine));
	});*/

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
	/*button->setOnClick([&]() {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new SkinnedMesh(m_machine));
	});*/

	label = button->addChild<Label>(m_characterSet);
	label->setText("Skinned Mesh");
	label->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	label->setPadding(0.0f, paddingBottom);

	button = surface->addChild<Button>();
	button->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	button->setOutlineColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	button->setOutlineThickness(5.0f);
	button->setPadding(5.0f, 5.0f);
	/*button->setOnClick([&]() {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new ComputeParticleLogo(m_machine));
	});*/

	label = button->addChild<Label>(m_characterSet);
	label->setText("Compute Particle Logo");
	label->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	label->setPadding(0.0f, paddingBottom);

	button = surface->addChild<Button>();
	button->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	button->setOutlineColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	button->setOutlineThickness(5.0f);
	button->setPadding(5.0f, 5.0f);
	/*button->setOnClick([&]() {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new PrimitivePicking(m_machine));
	});*/

	label = button->addChild<Label>(m_characterSet);
	label->setText("Primitive Picking");
	label->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	label->setPadding(0.0f, paddingBottom);

	button = surface->addChild<Button>();
	button->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	button->setOutlineColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	button->setOutlineThickness(5.0f);
	button->setPadding(5.0f, 5.0f);
	/*button->setOnClick([&]() {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new StencilMask(m_machine));
	});*/

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
	/*button->setOnClick([&]() {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new DeferredRendering(m_machine));
	});*/

	label = button->addChild<Label>(m_characterSet);
	label->setText("Defferred Rendering");
	label->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	label->setPadding(0.0f, paddingBottom);

	button = surface->addChild<Button>();
	button->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	button->setOutlineColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	button->setOutlineThickness(5.0f);
	button->setPadding(5.0f, 5.0f);
	/*button->setOnClick([&]() {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new VolumeRendering(m_machine));
	});*/

	label = button->addChild<Label>(m_characterSet);
	label->setText("Volume Rendering");
	label->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	label->setPadding(0.0f, paddingBottom);

	button = surface->addChild<Button>();
	button->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	button->setOutlineColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	button->setOutlineThickness(5.0f);
	button->setPadding(5.0f, 5.0f);
	/*button->setOnClick([&]() {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new OcclusionQuery(m_machine));
	});*/

	label = button->addChild<Label>(m_characterSet);
	label->setText("Occlusion Query");
	label->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	label->setPadding(0.0f, paddingBottom);

	button = surface->addChild<Button>();
	button->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	button->setOutlineColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	button->setOutlineThickness(5.0f);
	button->setPadding(5.0f, 5.0f);
	/*button->setOnClick([&]() {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new RenderBundles(m_machine));
	});*/

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
	/*button->setOnClick([&]() {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new NuklearUi(m_machine));
	});*/

	label = button->addChild<Label>(m_characterSet);
	label->setText("Nuklear UI");
	label->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	label->setPadding(0.0f, paddingBottom);

	button = surface->addChild<Button>();
	button->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	button->setOutlineColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	button->setOutlineThickness(5.0f);
	button->setPadding(5.0f, 5.0f);
	/*button->setOnClick([&]() {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new AudioDecode(m_machine));
	});*/

	label = button->addChild<Label>(m_characterSet);
	label->setText("Audio Decode");
	label->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	label->setPadding(0.0f, paddingBottom);

	button = surface->addChild<Button>();
	button->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	button->setOutlineColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	button->setOutlineThickness(5.0f);
	button->setPadding(5.0f, 5.0f);
	/*button->setOnClick([&]() {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new VideoDecode(m_machine));
	});*/

	label = button->addChild<Label>(m_characterSet);
	label->setText("Video Decode");
	label->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	label->setPadding(0.0f, paddingBottom);

	button = surface->addChild<Button>();
	button->setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	button->setOutlineColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	button->setOutlineThickness(5.0f);
	button->setPadding(5.0f, 5.0f);
	/*button->setOnClick([&]() {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new Cubes(m_machine));
	});*/

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
	/*button->setOnClick([&]() {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new Isometric(m_machine));
	});*/

	label = button->addChild<Label>(m_characterSet);
	label->setText("Isomeric");
	label->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	label->setPadding(0.0f, paddingBottom);

	if (m_layout == SelectedLayout::M_VERTICAL) {
		const std::vector<Surface*>& surfaces = m_uiScene->getChildren<Surface>();
		for (auto& surface : surfaces) {
			surface->setLayout(Layout::VERTICAL);
			m_uiScene->setLayout(Layout::HORIZONTAL);
		}
		m_uiScene->updateLayout();
	}

	if (m_layout == SelectedLayout::M_HORIZONTAL) {
		const std::vector<Surface*>& surfaces = m_uiScene->getChildren<Surface>();
		for (auto& surface : surfaces) {
			surface->setLayout(Layout::HORIZONTAL);
			m_uiScene->setLayout(Layout::VERTICAL);
		}
		m_uiScene->updateLayout();
	}

	wgpContext.setClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
	wgpContext.OnDraw = std::bind(&Menu::OnDraw, this, std::placeholders::_1, std::placeholders::_2);
}

Menu::~Menu() {

}

void Menu::fixedUpdate() {

}

void Menu::update() {
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

void Menu::OnScroll(double xoffset, double yoffset) {
	
}

void Menu::OnKeyDown(const Event::KeyboardEvent& event) {

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