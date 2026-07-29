#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_wgpu.h>
#include <imgui_impl_glfw.h>
#include <imgui_internal.h>

#include <WebGPU/WgpContext.h>
#include <WebGPU/WgpRenderer.h>

#include <Nuklear/NkStyle.h>
#include <Nuklear/NkNodeEditor.h>
#include <Nuklear/NkCalculator.h>

#include "NuklearGui.h"
#include "Mouse.h"
#include "Application.h"

int style_id[] = {
	999,
	THEME_WHITE,
	THEME_RED,
	THEME_BLUE,
	THEME_DARK
};

const char* style_name[] = {
	"Default",
	"White",
	"Red",
	"Blue",
	"Dark"
};
static int selected_item = 0;
struct nk_colorf backgroundf = { 0.2f, 0.2f, 0.2f, 1.0f };

NuklearGui::NuklearGui(StateMachine& machine) : State(machine, States::NUKLEAR_GUI) {
	Mouse::instance().attach(Application::Window, false, true);
	wgpSetSurfaceColorFormat(WGPUTextureFormat::WGPUTextureFormat_BGRA8Unorm, Application::OnSurfaceChange);
	wgpSetSurfaceDepthFormat(WGPUTextureFormat::WGPUTextureFormat_Depth24Plus, Application::OnSurfaceChange);

	nkInit(static_cast<float>(Application::Width), static_cast<float>(Application::Height));
	nkInitFont("res/fonts/upheavtt.ttf");
	nkInitIcon("res/textures/ui-icons-buttons-set-blue.png");

	m_camera.perspective(glm::radians(72.0f), static_cast<float>(Application::Width) / static_cast<float>(Application::Height), 0.1f, 1000.0f);
	m_camera.orthographic(0.0f, static_cast<float>(Application::Width), 0.0f, static_cast<float>(Application::Height), -1.0f, 1.0f);
	m_camera.lookAt(glm::vec3(0.0f, 15.0f, -50.0f), glm::vec3(0.0f, 15.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	m_camera.setMovingSpeed(50.0f);
	m_camera.setRotationSpeed(0.1f);

	m_trackball.reshape(Application::Width, Application::Height);

	wgpContext.setClearColor({ 0.2f, 0.2f, 0.2f, 1.0f });
	wgpContext.OnDraw = std::bind(&NuklearGui::OnDraw, this, std::placeholders::_1, std::placeholders::_2);
	nkContext.OnFillBuffer = std::bind(&NuklearGui::OnFillBuffer, this, std::placeholders::_1);
}

NuklearGui::~NuklearGui() {
	nkShutDown();
}

void NuklearGui::fixedUpdate() {

}

void NuklearGui::update() {

	Mouse &mouse = Mouse::instance();

	glm::vec3 direction = glm::vec3();

	float dx = 0.0f;
	float dy = 0.0f;
	bool move = false;

	if (glfwGetKey(Application::Window, GLFW_KEY_W) == GLFW_PRESS) {
		direction += glm::vec3(0.0f, 0.0f, 1.0f);
		move |= true;
	}

	if (glfwGetKey(Application::Window, GLFW_KEY_S) == GLFW_PRESS) {
		direction += glm::vec3(0.0f, 0.0f, -1.0f);
		move |= true;
	}

	if (glfwGetKey(Application::Window, GLFW_KEY_A) == GLFW_PRESS) {
		direction += glm::vec3(-1.0f, 0.0f, 0.0f);
		move |= true;
	}

	if (glfwGetKey(Application::Window, GLFW_KEY_D) == GLFW_PRESS) {
		direction += glm::vec3(1.0f, 0.0f, 0.0f);
		move |= true;
	}

	if (glfwGetKey(Application::Window, GLFW_KEY_Q) == GLFW_PRESS) {
		direction += glm::vec3(0.0f, -1.0f, 0.0f);
		move |= true;
	}

	if (glfwGetKey(Application::Window, GLFW_KEY_E) == GLFW_PRESS) {
		direction += glm::vec3(0.0f, 1.0f, 0.0f);
		move |= true;
	}

    if (mouse.buttonDownInvisible(GLFW_MOUSE_BUTTON_RIGHT)) {	
		dx = mouse.xDelta();
		dy = mouse.yDelta();
	}
	
    if (move || dx != 0.0f || dy != 0.0f) {
		if (dx || dy) {		
			m_camera.rotate(dx, dy);
		}

		if (move) {
			m_camera.move(direction * m_dt);
		}
	}
	m_trackball.idle();

	nkUpdateInput(mouse.xPos(), mouse.yPos(), mouse.buttonDown(GLFW_MOUSE_BUTTON_LEFT), mouse.buttonDown(GLFW_MOUSE_BUTTON_RIGHT), Application::ScrollDelta);
}

void NuklearGui::render() {
	wgpDraw();
}

void NuklearGui::OnDraw(const WGPUCommandEncoder& commandEncoder, const WGPURenderPassDescriptor& renderPassDescriptor) {
	{
		WGPURenderPassColorAttachment renderPassColorAttachment = renderPassDescriptor.colorAttachments[0];
		renderPassColorAttachment.loadOp = WGPULoadOp::WGPULoadOp_Load;

		WGPURenderPassDescriptor rndrPssDscrptor = renderPassDescriptor;
		rndrPssDscrptor.colorAttachments = &renderPassColorAttachment;

		nkDraw(commandEncoder, rndrPssDscrptor);
	}
}

void NuklearGui::OnFillBuffer(nk_context& nkCntxt) {
	calculator();
	node_editor();
	if (nk_begin(&nkCntxt, "Demo", nk_rect(430, 10, 230, 250),
		NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
		NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE))
	{

		nk_layout_row_dynamic(&nkCntxt, 20, 1);
		nk_label(&nkCntxt, "background:", NK_TEXT_LEFT);
		nk_layout_row_dynamic(&nkCntxt, 25, 1);
		if (nk_combo_begin_color(&nkCntxt, nk_rgba_cf(backgroundf), nk_vec2(nk_widget_width(&nkCntxt), 400))) {

			nk_layout_row_dynamic(&nkCntxt, 120, 1);
			backgroundf = nk_color_picker(&nkCntxt, backgroundf, NK_RGBA);
			nk_layout_row_dynamic(&nkCntxt, 25, 1);

			struct nk_color background = nk_rgba_cf(backgroundf);
			background.r = (nk_byte)nk_propertyi(&nkCntxt, "#R:", 0, background.r, 255, 1, 1);
			background.g = (nk_byte)nk_propertyi(&nkCntxt, "#G:", 0, background.g, 255, 1, 1);
			background.b = (nk_byte)nk_propertyi(&nkCntxt, "#B:", 0, background.b, 255, 1, 1);
			background.a = (nk_byte)nk_propertyi(&nkCntxt, "#A:", 0, background.a, 255, 1, 1);
			nk_combo_end(&nkCntxt);
			
			backgroundf = nk_color_cf(background);
			wgpContext.setClearColor({ backgroundf.r, backgroundf.g, backgroundf.b, backgroundf.a });
		}

		nk_layout_row_dynamic(&nkCntxt, 25, 2);
		nk_label(&nkCntxt, "GUI skin:", NK_TEXT_LEFT);
		if (nk_combo_begin_label(&nkCntxt, style_name[selected_item], nk_vec2(nk_widget_width(&nkCntxt), 200))) {
			int i;
			nk_layout_row_dynamic(&nkCntxt, 25, 1);
			for (i = 0; i < sizeof(style_id) / sizeof(style_id[0]); ++i)
				if (nk_combo_item_label(&nkCntxt, style_name[i], NK_TEXT_LEFT)) {
					selected_item = i;
					set_style(static_cast<theme>(style_id[i]));
				}
			nk_combo_end(&nkCntxt);
		}
	}
	nk_end(&nkCntxt);	
}

void NuklearGui::OnMouseMotion(const Event::MouseMoveEvent& event) {
	m_trackball.motion(event.x, event.y);
}

void NuklearGui::OnMouseButtonDown(const Event::MouseButtonEvent& event) {	
	if (event.button == Event::MouseButtonEvent::BUTTON_LEFT) {
		m_trackball.mouse(TrackBall::Button::ELeftButton, TrackBall::Modifier::ENoModifier, true, event.x, event.y);
		Mouse::instance().attach(Application::Window, false, true);
	}

	if (event.button == Event::MouseButtonEvent::BUTTON_RIGHT)
		Mouse::instance().attach(Application::Window, true, true, true);
}

void NuklearGui::OnMouseButtonUp(const Event::MouseButtonEvent& event) {
	if (event.button == Event::MouseButtonEvent::BUTTON_LEFT) {
		m_trackball.mouse(TrackBall::Button::ELeftButton, TrackBall::Modifier::ENoModifier, false, event.x, event.y);
		Mouse::instance().attach(Application::Window, false, true);
	} 

	if (event.button == Event::MouseButtonEvent::BUTTON_RIGHT)
		Mouse::instance().attach(Application::Window, false, false, true);
}

void NuklearGui::OnScroll(double xoffset, double yoffset) {
	
}

void NuklearGui::OnKeyDown(const Event::KeyboardEvent& event) {

}

void NuklearGui::OnKeyUp(const Event::KeyboardEvent& event) {

}

void NuklearGui::resize(int deltaW, int deltaH) {
	nkResize(static_cast<float>(Application::Width), static_cast<float>(Application::Height));
	m_camera.perspective(glm::radians(72.0f), static_cast<float>(Application::Width) / static_cast<float>(Application::Height), 0.1f, 1000.0f);
	m_camera.orthographic(0.0f, static_cast<float>(Application::Width), 0.0f, static_cast<float>(Application::Height), -1.0f, 1.0f);
	m_trackball.reshape(Application::Width, Application::Height);	
}

void NuklearGui::renderUi(const WGPURenderPassEncoder& renderPassEncoder) {
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