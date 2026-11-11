#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_wgpu.h>
#include <imgui_impl_glfw.h>
#include <imgui_internal.h>

#include <glm/ext.hpp>
#include <glm/gtc/type_ptr.hpp> 
#include <glm/gtx/polar_coordinates.hpp>

#include <WebGPU/WgpContext.h>
#include <States/Menu.h>

#include "Specularity.h"
#include "Application.h"
#include "Mouse.h"

Specularity::Specularity(StateMachine& machine) : State(machine, States::SPECULARITY) {

    m_uniformBuffer.createBuffer(sizeof(Uniforms), WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform);
	m_uniformLigthBuffer.createBuffer(sizeof(LightingUniforms), WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform);
	
	wgpContext.addSahderModule("BOAT", "res/shader/specularity.wgsl");
	wgpContext.createRenderPipeline("BOAT", "RP_PTNC", VL_PTNC, std::bind(&Specularity::OnBindGroupLayouts, this));

    m_boat.loadModel("res/models/fourareen/fourareen.obj", false, false, false, false, false, true);
	m_boat.generateColors();
	m_wgpBoat.create(m_boat);
	m_wgpBoat.setBindGroups("BG", std::bind(&Specularity::OnBindGroups, this));

    wgpContext.OnDraw = std::bind(&Specularity::OnDraw, this, std::placeholders::_1, std::placeholders::_2);
	
    float cx = cos(m_cameraState.angles.x);
	float sx = sin(m_cameraState.angles.x);
	float cy = cos(m_cameraState.angles.y);
	float sy = sin(m_cameraState.angles.y);
	glm::vec3 position = glm::vec3(cx * cy, sx * cy, sy) * std::exp(-m_cameraState.zoom);
	
	m_uniforms.projection = glm::perspective(glm::radians(45.0f), static_cast<float>(Application::Width) / static_cast<float>(Application::Height), 0.01f, 100.0f);
	m_uniforms.view = glm::lookAt(position, glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	m_uniforms.model = glm::mat4x4(1.0);
	m_uniforms.normal = glm::mat4(1.0f);	
	m_uniforms.color = { 0.0f, 1.0f, 0.4f, 1.0f };
	m_uniforms.camPosition = glm::vec3(0.0f);
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), 0, &m_uniforms, sizeof(Uniforms));

	m_lightingUniforms.directions[0] = { 0.5f, -0.9f, 0.1f, 0.0f };
	m_lightingUniforms.directions[1] = { 0.2f, 0.4f, 0.3f, 0.0f };
	m_lightingUniforms.colors[0] = { 1.0f, 0.9f, 0.6f, 1.0f };
	m_lightingUniforms.colors[1] = { 0.6f, 0.9f, 1.0f, 1.0f };
	updateLightingUniforms();
}

Specularity::~Specularity() {
	m_uniformBuffer.markForDelete();
	m_uniformLigthBuffer.markForDelete();
}

void Specularity::fixedUpdate() {

}

void Specularity::update() {
	updateDragInertia();
	updateLightingUniforms();
}

void Specularity::render() {
    wgpDraw();
}

void Specularity::OnDraw(const WGPUCommandEncoder& commandEncoder, const WGPURenderPassDescriptor& renderPassDescriptor) {
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), offsetof(Uniforms, projection), &m_uniforms.projection, sizeof(Uniforms::projection));
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), offsetof(Uniforms, view), &m_uniforms.view, sizeof(Uniforms::view));
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), offsetof(Uniforms, model), &m_uniforms.model, sizeof(Uniforms::model));
	
	WGPURenderPassEncoder renderPassEncoder = wgpuCommandEncoderBeginRenderPass(commandEncoder, &renderPassDescriptor);
	wgpuRenderPassEncoderSetViewport(renderPassEncoder, 0.0f, 0.0f, static_cast<float>(Application::Width), static_cast<float>(Application::Height), 0.0f, 1.0f);
	wgpuRenderPassEncoderSetPipeline(renderPassEncoder, wgpContext.renderPipelines.at("RP_PTNC"));

	m_wgpBoat.draw(renderPassEncoder);

	if (m_drawUi)
		renderUi(renderPassEncoder);

	wgpuRenderPassEncoderEnd(renderPassEncoder);
	wgpuRenderPassEncoderRelease(renderPassEncoder);
}

void Specularity::OnMouseButtonDown(const Event::MouseButtonEvent& event) {
	if (event.button == Event::MouseButtonEvent::BUTTON_RIGHT) {
		Mouse::instance().attach(Application::Window, true, true);
	}else if(event.button == Event::MouseButtonEvent::BUTTON_LEFT){
		m_drag.active = true;
		m_drag.startMouse = glm::vec2(-(float)event.x, (float)event.y);
		m_drag.startCameraState = m_cameraState;
	}	
}

void Specularity::OnMouseButtonUp(const Event::MouseButtonEvent& event) {
	if (event.button == Event::MouseButtonEvent::BUTTON_RIGHT) {
		Mouse::instance().detach();
	}else if(event.button == Event::MouseButtonEvent::BUTTON_LEFT){
		m_drag.active = false;	
	}		
}

void Specularity::OnMouseMotion(const Event::MouseMoveEvent& event) {
	if (m_drag.active) {
		glm::vec2 currentMouse = glm::vec2(-(float)event.x, (float)event.y);
		glm::vec2 delta = (currentMouse - m_drag.startMouse) * m_drag.sensitivity;
		m_cameraState.angles = m_drag.startCameraState.angles + delta;
		m_cameraState.angles.y = glm::clamp(m_cameraState.angles.y, -glm::pi<float>() / 2 + 1e-5f, glm::pi<float>() / 2 - 1e-5f);
		updateViewMatrix();
		m_drag.velocity = delta - m_drag.previousDelta;
		m_drag.previousDelta = delta;
	}
}

void Specularity::OnScroll(const Event::MouseWheelEvent& event){
	m_cameraState.zoom += m_drag.scrollSensitivity * (float)event.delta;
	m_cameraState.zoom = glm::clamp(m_cameraState.zoom, -2.0f, 2.0f);
	updateViewMatrix();
}

void Specularity::OnKeyDown(const Event::KeyboardEvent& event){
	if (event.keyCode == GLFW_KEY_LEFT_ALT) {	
	    m_drawUi = !m_drawUi;
    }

	if (event.keyCode == GLFW_KEY_ESCAPE) {
		wgpCleanState();
		m_isRunning = false;
		m_machine.addStateAtBottom(new Menu(m_machine));
	}
}

void Specularity::OnKeyUp(const Event::KeyboardEvent& event) {

}

void Specularity::resize(int deltaW, int deltaH) {
	float ratio = static_cast<float>(Application::Width) / static_cast<float>(Application::Height);
	m_uniforms.projection = glm::perspective(glm::radians(45.0f), ratio, 0.01f, 100.0f);
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), offsetof(Uniforms, projection), &m_uniforms.projection, sizeof(Uniforms::projection));
}

namespace ImGui {
	bool DragDirection(const char* label, glm::vec4& direction) {
		glm::vec2 angles = glm::degrees(glm::polar(glm::vec3(direction)));
		bool changed = ImGui::DragFloat2(label, glm::value_ptr(angles));
		direction = glm::vec4(glm::euclidean(glm::radians(angles)), direction.w);
		return changed;
	}
}

void Specularity::renderUi(const WGPURenderPassEncoder& renderPassEncoder) {
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
		ImGui::DockBuilderDockWindow("Lighting", dock_id_left);
	}

	{
		bool changed = false;
		ImGui::Begin("Lighting");
		changed = ImGui::ColorEdit3("Color #0", glm::value_ptr(m_lightingUniforms.colors[0])) || changed;
		changed = ImGui::DragDirection("Direction #0", m_lightingUniforms.directions[0]) || changed;
		changed = ImGui::ColorEdit3("Color #1", glm::value_ptr(m_lightingUniforms.colors[1])) || changed;
		changed = ImGui::DragDirection("Direction #1", m_lightingUniforms.directions[1]) || changed;
		changed = ImGui::SliderFloat("Hardness", &m_lightingUniforms.hardness, 1.0f, 100.0f) || changed;
		changed = ImGui::SliderFloat("K Diffuse", &m_lightingUniforms.kd, 0.0f, 1.0f) || changed;
		changed = ImGui::SliderFloat("K Specular", &m_lightingUniforms.ks, 0.0f, 1.0f) || changed;
		ImGui::End();
		m_updateLight = changed;
	}

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), renderPassEncoder);
}

std::vector <WGPUBindGroupLayout> Specularity::OnBindGroupLayouts() {
	std::vector<WGPUBindGroupLayout> bindingLayouts(1);

	std::vector<WGPUBindGroupLayoutEntry> bindingLayoutEntries(4);

	WGPUBindGroupLayoutEntry& uniformLayout = bindingLayoutEntries[0];
	uniformLayout.binding = 0;
	uniformLayout.visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
	uniformLayout.buffer.type = WGPUBufferBindingType::WGPUBufferBindingType_Uniform;
	uniformLayout.buffer.minBindingSize = sizeof(Uniforms);

	WGPUBindGroupLayoutEntry& textureBindingLayout = bindingLayoutEntries[1];
	textureBindingLayout.binding = 1;
	textureBindingLayout.visibility = WGPUShaderStage_Fragment;
	textureBindingLayout.texture.sampleType = WGPUTextureSampleType::WGPUTextureSampleType_Float;
	textureBindingLayout.texture.viewDimension = WGPUTextureViewDimension::WGPUTextureViewDimension_2D;

	WGPUBindGroupLayoutEntry& samplerBindingLayout = bindingLayoutEntries[2];
	samplerBindingLayout.binding = 2;
	samplerBindingLayout.visibility = WGPUShaderStage_Fragment;
	samplerBindingLayout.sampler.type = WGPUSamplerBindingType::WGPUSamplerBindingType_Filtering;

	WGPUBindGroupLayoutEntry& lightUniformLayout = bindingLayoutEntries[3];
	lightUniformLayout.binding = 3;
	lightUniformLayout.visibility = WGPUShaderStage_Fragment;
	lightUniformLayout.buffer.type = WGPUBufferBindingType::WGPUBufferBindingType_Uniform;
	lightUniformLayout.buffer.minBindingSize = sizeof(LightingUniforms);

	WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor = {};
	bindGroupLayoutDescriptor.entryCount = (uint32_t)bindingLayoutEntries.size();
	bindGroupLayoutDescriptor.entries = bindingLayoutEntries.data();

	bindingLayouts[0] = wgpuDeviceCreateBindGroupLayout(wgpContext.device, &bindGroupLayoutDescriptor);
	return bindingLayouts;
}

std::vector<WGPUBindGroup> Specularity::OnBindGroups() {
	std::vector<WGPUBindGroup> bindGroups(1);
	
	std::vector<WGPUBindGroupEntry> bindings(4);

	bindings[0].binding = 0;
	bindings[0].buffer = m_uniformBuffer.getBuffer();
	bindings[0].offset = 0;
	bindings[0].size = sizeof(Uniforms);

	bindings[1].binding = 1;
	bindings[1].textureView = m_wgpBoat.getMeshes().begin()->getTexture().getTextureView();

	bindings[2].binding = 2;
	bindings[2].sampler = wgpContext.getSampler(SS_LINEAR_CLAMP);

	bindings[3].binding = 3;
	bindings[3].buffer = m_uniformLigthBuffer.getBuffer();
	bindings[3].offset = 0;
	bindings[3].size = sizeof(LightingUniforms);

	WGPUBindGroupDescriptor bindGroupDesc = {};
	bindGroupDesc.layout = wgpuRenderPipelineGetBindGroupLayout(wgpContext.renderPipelines.at("RP_PTNC"), 0);
	bindGroupDesc.entryCount = (uint32_t)bindings.size();
	bindGroupDesc.entries = bindings.data();

	bindGroups[0] = wgpuDeviceCreateBindGroup(wgpContext.device, &bindGroupDesc);
	return bindGroups;
}

void Specularity::updateViewMatrix() {
	float cx = cos(m_cameraState.angles.x);
	float sx = sin(m_cameraState.angles.x);
	float cy = cos(m_cameraState.angles.y);
	float sy = sin(m_cameraState.angles.y);
	glm::vec3 position = glm::vec3(cx * cy, sx * cy, sy) * std::exp(-m_cameraState.zoom);
	m_uniforms.view = glm::lookAt(position, glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), offsetof(Uniforms, view), &m_uniforms.view, sizeof(Uniforms::view));

	m_uniforms.camPosition = position;
    wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), offsetof(Uniforms, camPosition), &m_uniforms.camPosition, sizeof(Uniforms::camPosition));
}

void Specularity::updateLightingUniforms() {
	if (m_updateLight) {
		wgpuQueueWriteBuffer(wgpContext.queue, m_uniformLigthBuffer.getBuffer(), 0u, &m_lightingUniforms, sizeof(LightingUniforms));
		m_updateLight = false;
	}
}

void Specularity::updateDragInertia() {
	constexpr float eps = 1e-4f;
	if (!m_drag.active) {
		if (std::abs(m_drag.velocity.x) < eps && std::abs(m_drag.velocity.y) < eps) {
			return;
		}
		m_cameraState.angles += m_drag.velocity;
		m_cameraState.angles.y = glm::clamp(m_cameraState.angles.y, -glm::pi<float>() / 2 + 1e-5f, glm::pi<float>() / 2 - 1e-5f);
		m_drag.velocity *= m_drag.intertia;
		updateViewMatrix();
	}
}
