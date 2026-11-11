#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_wgpu.h>
#include <imgui_impl_glfw.h>
#include <imgui_internal.h>


#include <WebGPU/WgpContext.h>
#include <States/Menu.h>

#include "InstancedCube.h"
#include "Application.h"
#include "Mouse.h"

InstancedCube::InstancedCube(StateMachine& machine) : State(machine, States::INSTANCED_CUBE) {

	m_camera.perspective(glm::radians(72.0f), static_cast<float>(Application::Width) / static_cast<float>(Application::Height), 0.1f, 1000.0f);
	m_camera.orthographic(0.0f, static_cast<float>(Application::Width), 0.0f, static_cast<float>(Application::Height), -1.0f, 1.0f);
	m_camera.lookAt(glm::vec3(0.0f, 0.0f, 12.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	m_camera.setRotationSpeed(0.125f);
	m_camera.setMovingSpeed(10.0f);

	m_uniformBuffer.createBuffer(1024u, WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform);

	wgpContext.setClearColor({ 0.1f, 0.2f, 0.3f, 1.0f });
	wgpContext.addSahderModule("FONT", "res/shader/instance.wgsl");
	wgpContext.createRenderPipeline("FONT", "RP_INSTANCED", VL_PTN, std::bind(&InstancedCube::OnBindGroupLayouts, this));
	wgpContext.OnDraw = std::bind(&InstancedCube::OnDraw, this, std::placeholders::_1, std::placeholders::_2);

	m_cube.buildCube({ -1.0f, -1.0f, -1.0f }, { 2.0f, 2.0f, 2.0f }, 1u, 1u, true, true, false);
	m_wgpCube.create(m_cube);
	m_wgpCube.setBindGroups("BG", std::bind(&InstancedCube::OnBindGroups, this));

	initMVPMatrices();
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), 0u, &m_mvps[0], 1024u);
}

InstancedCube::~InstancedCube() {
	m_uniformBuffer.markForDelete();
}

void InstancedCube::fixedUpdate() {

}

void InstancedCube::update() {
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

	updateMVPMatrices();
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), 0u, &m_mvps[0], 1024u);
}

void InstancedCube::render() {
    wgpDraw();
}

void InstancedCube::OnDraw(const WGPUCommandEncoder& commandEncoder, const WGPURenderPassDescriptor& renderPassDescriptor) {

	WGPURenderPassEncoder renderPassEncoder = wgpuCommandEncoderBeginRenderPass(commandEncoder, &renderPassDescriptor);
	wgpuRenderPassEncoderSetViewport(renderPassEncoder, 0.0f, 0.0f, static_cast<float>(Application::Width), static_cast<float>(Application::Height), 0.0f, 1.0f);

	wgpuRenderPassEncoderSetPipeline(renderPassEncoder, wgpContext.renderPipelines.at("RP_INSTANCED"));
	m_wgpCube.draw(renderPassEncoder, 16u);

	if (m_drawUi)
		renderUi(renderPassEncoder);

	wgpuRenderPassEncoderEnd(renderPassEncoder);
	wgpuRenderPassEncoderRelease(renderPassEncoder);
}

void InstancedCube::OnMouseButtonDown(const Event::MouseButtonEvent& event) {
	if (event.button == Event::MouseButtonEvent::BUTTON_RIGHT) {
		Mouse::instance().attach(Application::Window, true, true);
	}
}

void InstancedCube::OnMouseButtonUp(const Event::MouseButtonEvent& event) {
	if (event.button == Event::MouseButtonEvent::BUTTON_RIGHT) {
		Mouse::instance().detach();
	} 
}

void InstancedCube::OnMouseMotion(const Event::MouseMoveEvent& event) {

}

void InstancedCube::OnScroll(const Event::MouseWheelEvent& event) {
	
}

void InstancedCube::OnKeyDown(const Event::KeyboardEvent& event ){
	if (event.keyCode == GLFW_KEY_LEFT_ALT) {	
	    m_drawUi = !m_drawUi;
    }

	if (event.keyCode == GLFW_KEY_ESCAPE) {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new Menu(m_machine));
	}
}

void InstancedCube::OnKeyUp(const Event::KeyboardEvent& event) {

}

void InstancedCube::resize(int deltaW, int deltaH) {
	m_camera.perspective(glm::radians(72.0f), static_cast<float>(Application::Width) / static_cast<float>(Application::Height), 0.1f, 1000.0f);
	m_camera.orthographic(0.0f, static_cast<float>(Application::Width), 0.0f, static_cast<float>(Application::Height), -1.0f, 1.0f);
}

void InstancedCube::renderUi(const WGPURenderPassEncoder& renderPassEncoder) {
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
		ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Left, 0.4f, nullptr, &dockSpaceId);
		ImGui::DockBuilderDockWindow("Settings", dock_id_left);
	}

	// render widgets
	ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::End();

	ImGui::Render();
	ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), renderPassEncoder);
}

std::vector <WGPUBindGroupLayout> InstancedCube::OnBindGroupLayouts() {
	std::vector<WGPUBindGroupLayout> bindingLayouts(1);

	std::vector<WGPUBindGroupLayoutEntry> bindingLayoutEntries(1);
	WGPUBindGroupLayoutEntry& uniformLayout = bindingLayoutEntries[0];
	uniformLayout.binding = 0u;
	uniformLayout.visibility = WGPUShaderStage_Vertex;
	uniformLayout.buffer.type = WGPUBufferBindingType::WGPUBufferBindingType_Uniform;
	uniformLayout.buffer.minBindingSize = 1024u;

	WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor = {};
	bindGroupLayoutDescriptor.entryCount = (uint32_t)bindingLayoutEntries.size();
	bindGroupLayoutDescriptor.entries = bindingLayoutEntries.data();

	bindingLayouts[0] = wgpuDeviceCreateBindGroupLayout(wgpContext.device, &bindGroupLayoutDescriptor);

	return bindingLayouts;
}

std::vector<WGPUBindGroup> InstancedCube::OnBindGroups() {
	std::vector<WGPUBindGroup> bindGroups(1);

	std::vector<WGPUBindGroupEntry> bindGroupEntries(1);
	bindGroupEntries[0].binding = 0u;
	bindGroupEntries[0].buffer = m_uniformBuffer.getBuffer();
	bindGroupEntries[0].offset = 0u;
	bindGroupEntries[0].size = 1024u;

	WGPUBindGroupDescriptor bindGroupDesc = {};
	bindGroupDesc.layout = wgpuRenderPipelineGetBindGroupLayout(wgpContext.renderPipelines.at("RP_INSTANCED"), 0u);
	bindGroupDesc.entryCount = (uint32_t)bindGroupEntries.size();
	bindGroupDesc.entries = bindGroupEntries.data();

	bindGroups[0] = wgpuDeviceCreateBindGroup(wgpContext.device, &bindGroupDesc);

	return bindGroups;
}

void InstancedCube::initMVPMatrices() {
	const float step = 4.0f;
	uint32_t m = 0u;
	for (uint32_t x = 0; x < 4u; x++) {
		for (uint32_t y = 0; y < 4u; y++) {
			m_mvps[m] = glm::mat4(1.0f);
			m_mvps[m] = glm::translate(m_mvps[m], glm::vec3(step * (x - 4u / 2.0f + 0.5f), step * (y - 4u / 2.0f + 0.5f), 0.0f));
			m_mvps[m] = m_camera.getPerspectiveMatrix() * m_camera.getViewMatrix() * m_mvps[m];
			++m;
		}
	}
}

void InstancedCube::updateMVPMatrices() {
	const float sec = (float)glfwGetTime();
	const float step = 4.0f;

	uint32_t m = 0u;
	for (uint32_t x = 0; x < 4u; ++x) {
		for (uint32_t y = 0; y < 4u; ++y) {
			m_mvps[m] = glm::mat4(1.0f);
			m_mvps[m] = glm::translate(m_mvps[m], glm::vec3(step * (x - 4u / 2.0f + 0.5f), step * (y - 4u / 2.0f + 0.5f), 0.0f));
			m_mvps[m] = glm::rotate(m_mvps[m], 1.0f, glm::vec3(sinf(((float)x + 0.5f) * sec), cosf(((float)y + 0.5f) * sec), 0.0f));			
			m_mvps[m] = m_camera.getPerspectiveMatrix() * m_camera.getViewMatrix() * m_mvps[m];				
			++m;
		}
	}
}