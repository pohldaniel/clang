
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_wgpu.h>
#include <imgui_impl_glfw.h>
#include <imgui_internal.h>

#include <glm/ext.hpp>
#include <glm/gtc/type_ptr.hpp> 
#include <glm/gtx/polar_coordinates.hpp>

#include <WebGPU/WgpContext.h>
#include "ImageBasedLighting.h"
#include "Application.h"
#include "Mouse.h"

ImageBasedLighting::ImageBasedLighting(StateMachine& machine) : State(machine, States::IMAGE_BASED_LIGHTING) {

	m_camera.perspective(glm::radians(72.0f), static_cast<float>(Application::Width) / static_cast<float>(Application::Height), 0.1f, 1000.0f);
	m_camera.orthographic(0.0f, static_cast<float>(Application::Width), 0.0f, static_cast<float>(Application::Height), -1.0f, 1.0f);
	m_camera.lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	m_camera.setRotationSpeed(0.125f);
	m_camera.setMovingSpeed(10.0f);

	m_trackball.reshape(Application::Width, Application::Height);

	m_helmet.loadModel("res/models/helmet/helmet.obj");
	m_cube.buildCube({ -0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f }, 1u, 1u, false, false);
	m_cube.rewind();

	m_sphere.buildSphere({ 0.0f, 0.0f, 0.0f }, 1.0f, 49u, 49u, false, false);
	m_sphere.rewind();

	m_wgpTextureCube.loadHDRICubeFromFile("res/textures/venice_sunset_1k.hdr", false, true);
	m_wgpTexture.loadHDRIFromFile("res/textures/venice_sunset_1k.hdr", false, true);

	m_uniformBuffer.createBuffer(sizeof(Uniforms), WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform);

	wgpContext.setClearColor({ 0.1f, 0.2f, 0.3f, 1.0f });
	wgpContext.addSahderModule("TEXTURE", "res/shader/texture.wgsl");
	wgpContext.createRenderPipeline("TEXTURE", "RP_PTN", VL_PTN, std::bind(&ImageBasedLighting::OnBindGroupLayouts, this), 4u);

	wgpContext.addSahderModule("ENV_CUBE", "res/shader/env_cube.wgsl");
	wgpContext.createRenderPipeline("ENV_CUBE", "RP_ENV_CUBE", VL_P, std::bind(&ImageBasedLighting::OnBindGroupLayoutsEnvCube, this), 4u);

	wgpContext.addSahderModule("ENV_SPHERE", "res/shader/env_sphere.wgsl");
	wgpContext.createRenderPipeline("ENV_SPHERE", "RP_ENV_SPHERE", VL_P, std::bind(&ImageBasedLighting::OnBindGroupLayoutsEnvSphere, this), 4u);

	wgpContext.OnDraw = std::bind(&ImageBasedLighting::OnDraw, this, std::placeholders::_1);

	m_wgpHelmet.create(m_helmet);
	m_wgpHelmet.setBindGroups("BG", std::bind(&ImageBasedLighting::OnBindGroups, this));
	AddBindgroups(m_wgpHelmet);

	m_wgpCube.create(m_cube);
	m_wgpCube.setBindGroups("BG", std::bind(&ImageBasedLighting::OnBindGroupsEnvCube, this));
	AddBindgroups(m_wgpCube, m_wgpTextureCube, "RP_ENV_CUBE");

	m_wgpSphere.create(m_sphere);
	m_wgpSphere.setBindGroups("BG", std::bind(&ImageBasedLighting::OnBindGroupsEnvSphere, this));
	AddBindgroups(m_wgpSphere, m_wgpTexture, "RP_ENV_SPHERE");

	m_uniforms.projectionMatrix = m_camera.getPerspectiveMatrix();
	m_uniforms.viewMatrix = m_camera.getViewMatrix();
	m_uniforms.envMatrix = m_camera.getRotationMatrix();
	m_uniforms.modelMatrix = glm::mat4(1.0f);
	m_uniforms.normalMatrix = glm::mat4(1.0f);
	m_uniforms.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_uniforms.camPosition = glm::vec3(0.0f);

	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), 0, &m_uniforms, sizeof(Uniforms));
}

ImageBasedLighting::~ImageBasedLighting() {
	m_uniformBuffer.markForDelete();
}

void ImageBasedLighting::fixedUpdate() {

}

void ImageBasedLighting::update() {
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

    if (glfwGetMouseButton(Application::Window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {	
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
	applyTransformation(m_trackball);

	m_uniforms.projectionMatrix = m_camera.getPerspectiveMatrix();
	m_uniforms.viewMatrix = m_camera.getViewMatrix();
	m_uniforms.envMatrix = m_camera.getRotationMatrix();
	m_uniforms.normalMatrix = Camera::GetNormalMatrix(m_camera.getViewMatrix() * m_uniforms.modelMatrix);
}

void ImageBasedLighting::render() {
    wgpDraw();
}

void ImageBasedLighting::OnDraw(const WGPURenderPassEncoder& renderPassEncoder) {

	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), offsetof(Uniforms, projectionMatrix), &m_uniforms.projectionMatrix, sizeof(Uniforms::projectionMatrix));
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), offsetof(Uniforms, viewMatrix), &m_uniforms.viewMatrix, sizeof(Uniforms::viewMatrix));
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), offsetof(Uniforms, envMatrix), &m_uniforms.envMatrix, sizeof(Uniforms::envMatrix));
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), offsetof(Uniforms, modelMatrix), &m_uniforms.modelMatrix, sizeof(Uniforms::modelMatrix));
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), offsetof(Uniforms, normalMatrix), &m_uniforms.normalMatrix, sizeof(Uniforms::normalMatrix));

	wgpuRenderPassEncoderSetViewport(renderPassEncoder, 0.0f, 0.0f, static_cast<float>(Application::Width), static_cast<float>(Application::Height), 0.0f, 1.0f);

	wgpuRenderPassEncoderSetPipeline(renderPassEncoder, wgpContext.renderPipelines.at("RP_ENV_CUBE"));
	m_wgpCube.draw(renderPassEncoder);

	//wgpuRenderPassEncoderSetPipeline(renderPassEncoder, wgpContext.renderPipelines.at("RP_ENV_SPHERE"));
	//m_wgpSphere.draw(renderPassEncoder);

	wgpuRenderPassEncoderSetPipeline(renderPassEncoder, wgpContext.renderPipelines.at("RP_PTN"));
	m_wgpHelmet.draw(renderPassEncoder);

	if (m_drawUi)
		renderUi(renderPassEncoder);
}

void ImageBasedLighting::OnMouseButtonDown(const Event::MouseButtonEvent& event) {
	if (event.button == Event::MouseButtonEvent::BUTTON_RIGHT) {
		Mouse::instance().attach(Application::Window, true, false, false);
	}

	if (event.button == Event::MouseButtonEvent::BUTTON_LEFT) {
		m_trackball.mouse(TrackBall::Button::ELeftButton, TrackBall::Modifier::ENoModifier, true, event.x, event.y);
		applyTransformation(m_trackball);
	}
}

void ImageBasedLighting::OnMouseButtonUp(const Event::MouseButtonEvent& event) {
	if (event.button == Event::MouseButtonEvent::BUTTON_RIGHT) {
		Mouse::instance().detach();
	} 

	if (event.button == Event::MouseButtonEvent::BUTTON_LEFT) {
		m_trackball.mouse(TrackBall::Button::ELeftButton, TrackBall::Modifier::ENoModifier, false, event.x, event.y);
		applyTransformation(m_trackball);
	} 
}

void ImageBasedLighting::OnMouseMotion(const Event::MouseMoveEvent& event) {
	m_trackball.motion(event.x, event.y);
	applyTransformation(m_trackball);
}

void ImageBasedLighting::OnScroll(double xoffset, double yoffset) {
	
}

void ImageBasedLighting::OnKeyDown(const Event::KeyboardEvent& event ){

}

void ImageBasedLighting::OnKeyUp(const Event::KeyboardEvent& event) {

}

void ImageBasedLighting::resize(int deltaW, int deltaH) {
	m_camera.perspective(glm::radians(72.0f), static_cast<float>(Application::Width) / static_cast<float>(Application::Height), 0.1f, 1000.0f);
	m_camera.orthographic(0.0f, static_cast<float>(Application::Width), 0.0f, static_cast<float>(Application::Height), -1.0f, 1.0f);
}

void ImageBasedLighting::applyTransformation(const TrackBall& arc) {
  m_uniforms.modelMatrix = arc.getTransform();
}

void ImageBasedLighting::renderUi(const WGPURenderPassEncoder& renderPassEncoder) {
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
		ImGui::DockBuilderDockWindow("Settings", dock_id_left);
	}

	// render widgets
	ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Text("Application average FPS %.1f", static_cast<double>(ImGui::GetIO().Framerate));
	ImGui::End();

	ImGui::Render();
	ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), renderPassEncoder);
}

std::vector <WGPUBindGroupLayout> ImageBasedLighting::OnBindGroupLayouts() {
	std::vector<WGPUBindGroupLayout> bindingLayouts(2);

	std::vector<WGPUBindGroupLayoutEntry> bindingLayoutEntries0(2);

	WGPUBindGroupLayoutEntry& uniformLayout = bindingLayoutEntries0[0];
	uniformLayout.binding = 0u;
	uniformLayout.visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
	uniformLayout.buffer.type = WGPUBufferBindingType_Uniform;
	uniformLayout.buffer.minBindingSize = sizeof(Uniforms);

	WGPUBindGroupLayoutEntry& samplerBindingLayout = bindingLayoutEntries0[1];
	samplerBindingLayout.binding = 1u;
	samplerBindingLayout.visibility = WGPUShaderStage_Fragment;
	samplerBindingLayout.sampler.type = WGPUSamplerBindingType_Filtering;

	WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor0 = {};
	bindGroupLayoutDescriptor0.entryCount = (uint32_t)bindingLayoutEntries0.size();
	bindGroupLayoutDescriptor0.entries = bindingLayoutEntries0.data();

	bindingLayouts[0] = wgpuDeviceCreateBindGroupLayout(wgpContext.device, &bindGroupLayoutDescriptor0);

	std::vector<WGPUBindGroupLayoutEntry> bindingLayoutEntries1(1);

	WGPUBindGroupLayoutEntry& textureBindingLayout = bindingLayoutEntries1[0];
	textureBindingLayout.binding = 0u;
	textureBindingLayout.visibility = WGPUShaderStage_Fragment;
	textureBindingLayout.texture.sampleType = WGPUTextureSampleType_Float;
	textureBindingLayout.texture.viewDimension = WGPUTextureViewDimension_2D;

	WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor1 = {};
	bindGroupLayoutDescriptor1.entryCount = (uint32_t)bindingLayoutEntries1.size();
	bindGroupLayoutDescriptor1.entries = bindingLayoutEntries1.data();

	bindingLayouts[1] = wgpuDeviceCreateBindGroupLayout(wgpContext.device, &bindGroupLayoutDescriptor1);

	return bindingLayouts;
}

std::vector<WGPUBindGroup> ImageBasedLighting::OnBindGroups() {
	std::vector<WGPUBindGroup> bindGroups(1);

	std::vector<WGPUBindGroupEntry> bindGroupEntries0(2);

	bindGroupEntries0[0].binding = 0u;
	bindGroupEntries0[0].buffer = m_uniformBuffer.getBuffer();
	bindGroupEntries0[0].offset = 0u;
	bindGroupEntries0[0].size = wgpuBufferGetSize(m_uniformBuffer.getBuffer());

	bindGroupEntries0[1].binding = 1u;
	bindGroupEntries0[1].sampler = wgpContext.getSampler(SS_LINEAR_REPEAT);

	WGPUBindGroupDescriptor bindGroupDesc0 = {};
	bindGroupDesc0.layout = wgpuRenderPipelineGetBindGroupLayout(wgpContext.renderPipelines.at("RP_PTN"), 0u);
	bindGroupDesc0.entryCount = (uint32_t)bindGroupEntries0.size();
	bindGroupDesc0.entries = bindGroupEntries0.data();

	bindGroups[0] = wgpuDeviceCreateBindGroup(wgpContext.device, &bindGroupDesc0);

	return bindGroups;
}

void ImageBasedLighting::AddBindgroups(const WgpModel& model) {
	for (std::list<WgpMesh>::const_iterator it = model.getMeshes().begin(); it != model.getMeshes().end(); ++it) {
		const WgpMesh& mesh = *it;

		std::vector<WGPUBindGroupEntry> bindingGroupEntries(1);
		bindingGroupEntries[0].binding = 0u;
		bindingGroupEntries[0].textureView = mesh.getTexture().getTextureView();

		WGPUBindGroupDescriptor bindGroupDesc = {};
		bindGroupDesc.layout = wgpuRenderPipelineGetBindGroupLayout(wgpContext.renderPipelines.at("RP_PTN"), 1u);
		bindGroupDesc.entryCount = (uint32_t)bindingGroupEntries.size();
		bindGroupDesc.entries = bindingGroupEntries.data();

		mesh.addBindGroup("BG", wgpuDeviceCreateBindGroup(wgpContext.device, &bindGroupDesc));
	}
}

std::vector<WGPUBindGroupLayout> ImageBasedLighting::OnBindGroupLayoutsEnvCube() {
	std::vector<WGPUBindGroupLayout> bindingLayouts(2);

	std::vector<WGPUBindGroupLayoutEntry> bindingLayoutEntries0(2);

	WGPUBindGroupLayoutEntry& uniformLayout = bindingLayoutEntries0[0];
	uniformLayout.binding = 0u;
	uniformLayout.visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
	uniformLayout.buffer.type = WGPUBufferBindingType_Uniform;
	uniformLayout.buffer.minBindingSize = sizeof(Uniforms);

	WGPUBindGroupLayoutEntry& samplerBindingLayout = bindingLayoutEntries0[1];
	samplerBindingLayout.binding = 1u;
	samplerBindingLayout.visibility = WGPUShaderStage_Fragment;
	//samplerBindingLayout.sampler.type = WGPUSamplerBindingType_NonFiltering;
	samplerBindingLayout.sampler.type = WGPUSamplerBindingType_Filtering;

	WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor0 = {};
	bindGroupLayoutDescriptor0.entryCount = (uint32_t)bindingLayoutEntries0.size();
	bindGroupLayoutDescriptor0.entries = bindingLayoutEntries0.data();

	bindingLayouts[0] = wgpuDeviceCreateBindGroupLayout(wgpContext.device, &bindGroupLayoutDescriptor0);

	std::vector<WGPUBindGroupLayoutEntry> bindingLayoutEntries1(1);

	WGPUBindGroupLayoutEntry& textureBindingLayout = bindingLayoutEntries1[0];
	textureBindingLayout.binding = 0u;
	textureBindingLayout.visibility = WGPUShaderStage_Fragment;
	textureBindingLayout.texture.viewDimension = WGPUTextureViewDimension_Cube;
	//textureBindingLayout.texture.sampleType = WGPUTextureSampleType_UnfilterableFloat;
	textureBindingLayout.texture.sampleType = WGPUTextureSampleType_Float;
	
	WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor1 = {};
	bindGroupLayoutDescriptor1.entryCount = (uint32_t)bindingLayoutEntries1.size();
	bindGroupLayoutDescriptor1.entries = bindingLayoutEntries1.data();

	bindingLayouts[1] = wgpuDeviceCreateBindGroupLayout(wgpContext.device, &bindGroupLayoutDescriptor1);

	return bindingLayouts;
}

std::vector<WGPUBindGroup> ImageBasedLighting::OnBindGroupsEnvCube() {
	std::vector<WGPUBindGroup> bindGroups(1);

	std::vector<WGPUBindGroupEntry> bindGroupEntries0(2);

	bindGroupEntries0[0].binding = 0u;
	bindGroupEntries0[0].buffer = m_uniformBuffer.getBuffer();
	bindGroupEntries0[0].offset = 0u;
	bindGroupEntries0[0].size = wgpuBufferGetSize(m_uniformBuffer.getBuffer());

	bindGroupEntries0[1].binding = 1u;
	bindGroupEntries0[1].sampler = wgpContext.getSampler(SS_LINEAR_CLAMP);

	WGPUBindGroupDescriptor bindGroupDesc0 = {};
	bindGroupDesc0.layout = wgpuRenderPipelineGetBindGroupLayout(wgpContext.renderPipelines.at("RP_ENV_CUBE"), 0u);
	bindGroupDesc0.entryCount = (uint32_t)bindGroupEntries0.size();
	bindGroupDesc0.entries = bindGroupEntries0.data();

	bindGroups[0] = wgpuDeviceCreateBindGroup(wgpContext.device, &bindGroupDesc0);

	return bindGroups;
}

std::vector<WGPUBindGroupLayout> ImageBasedLighting::OnBindGroupLayoutsEnvSphere() {
	std::vector<WGPUBindGroupLayout> bindingLayouts(2);

	std::vector<WGPUBindGroupLayoutEntry> bindingLayoutEntries0(2);

	WGPUBindGroupLayoutEntry& uniformLayout = bindingLayoutEntries0[0];
	uniformLayout.binding = 0u;
	uniformLayout.visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
	uniformLayout.buffer.type = WGPUBufferBindingType_Uniform;
	uniformLayout.buffer.minBindingSize = sizeof(Uniforms);

	WGPUBindGroupLayoutEntry& samplerBindingLayout = bindingLayoutEntries0[1];
	samplerBindingLayout.binding = 1u;
	samplerBindingLayout.visibility = WGPUShaderStage_Fragment;
	//samplerBindingLayout.sampler.type = WGPUSamplerBindingType_NonFiltering;
	samplerBindingLayout.sampler.type = WGPUSamplerBindingType_Filtering;

	WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor0 = {};
	bindGroupLayoutDescriptor0.entryCount = (uint32_t)bindingLayoutEntries0.size();
	bindGroupLayoutDescriptor0.entries = bindingLayoutEntries0.data();

	bindingLayouts[0] = wgpuDeviceCreateBindGroupLayout(wgpContext.device, &bindGroupLayoutDescriptor0);

	std::vector<WGPUBindGroupLayoutEntry> bindingLayoutEntries1(1);

	WGPUBindGroupLayoutEntry& textureBindingLayout = bindingLayoutEntries1[0];
	textureBindingLayout.binding = 0u;
	textureBindingLayout.visibility = WGPUShaderStage_Fragment;
	textureBindingLayout.texture.viewDimension = WGPUTextureViewDimension_2D;
	//textureBindingLayout.texture.sampleType = WGPUTextureSampleType_UnfilterableFloat;
	textureBindingLayout.texture.sampleType = WGPUTextureSampleType_Float;

	WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor1 = {};
	bindGroupLayoutDescriptor1.entryCount = (uint32_t)bindingLayoutEntries1.size();
	bindGroupLayoutDescriptor1.entries = bindingLayoutEntries1.data();

	bindingLayouts[1] = wgpuDeviceCreateBindGroupLayout(wgpContext.device, &bindGroupLayoutDescriptor1);

	return bindingLayouts;
}

std::vector<WGPUBindGroup> ImageBasedLighting::OnBindGroupsEnvSphere() {
	std::vector<WGPUBindGroup> bindGroups(1);

	std::vector<WGPUBindGroupEntry> bindGroupEntries0(2);

	bindGroupEntries0[0].binding = 0u;
	bindGroupEntries0[0].buffer = m_uniformBuffer.getBuffer();
	bindGroupEntries0[0].offset = 0u;
	bindGroupEntries0[0].size = wgpuBufferGetSize(m_uniformBuffer.getBuffer());

	bindGroupEntries0[1].binding = 1u;
	bindGroupEntries0[1].sampler = wgpContext.getSampler(SS_LINEAR_CLAMP);

	WGPUBindGroupDescriptor bindGroupDesc0 = {};
	bindGroupDesc0.layout = wgpuRenderPipelineGetBindGroupLayout(wgpContext.renderPipelines.at("RP_ENV_SPHERE"), 0u);
	bindGroupDesc0.entryCount = (uint32_t)bindGroupEntries0.size();
	bindGroupDesc0.entries = bindGroupEntries0.data();

	bindGroups[0] = wgpuDeviceCreateBindGroup(wgpContext.device, &bindGroupDesc0);

	return bindGroups;
}

void ImageBasedLighting::AddBindgroups(const WgpModel& model, const WgpTexture& texture, std::string pipelineName) {
	for (std::list<WgpMesh>::const_iterator it = model.getMeshes().begin(); it != model.getMeshes().end(); ++it) {
		const WgpMesh& mesh = *it;

		std::vector<WGPUBindGroupEntry> bindingGroupEntries(1);
		bindingGroupEntries[0].binding = 0u;
		bindingGroupEntries[0].textureView = texture.getTextureView();

		WGPUBindGroupDescriptor bindGroupDesc = {};
		bindGroupDesc.layout = wgpuRenderPipelineGetBindGroupLayout(wgpContext.renderPipelines.at(pipelineName), 1u);
		bindGroupDesc.entryCount = (uint32_t)bindingGroupEntries.size();
		bindGroupDesc.entries = bindingGroupEntries.data();

		mesh.addBindGroup("BG", wgpuDeviceCreateBindGroup(wgpContext.device, &bindGroupDesc));
	}
}