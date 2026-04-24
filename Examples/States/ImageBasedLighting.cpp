
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_wgpu.h>
#include <imgui_impl_glfw.h>
#include <imgui_internal.h>

#include <glm/ext.hpp>
#include <glm/gtc/type_ptr.hpp> 
#include <glm/gtx/polar_coordinates.hpp>

#include <WebGPU/WgpContext.h>
#include <WebGPU/WgpRenderer.h>
#include "ImageBasedLighting.h"
#include "Application.h"
#include "Mouse.h"
#include <iostream>
ImageBasedLighting::ImageBasedLighting(StateMachine& machine) : State(machine, States::IMAGE_BASED_LIGHTING) {

	m_camera.perspective(glm::radians(45.0f), static_cast<float>(Application::Width) / static_cast<float>(Application::Height), 0.1f, 1000.0f);
	m_camera.orthographic(0.0f, static_cast<float>(Application::Width), 0.0f, static_cast<float>(Application::Height), -1.0f, 1.0f);
	m_camera.lookAt(glm::vec3(0.0f, 0.0f, 20.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	m_camera.setRotationSpeed(0.125f);
	m_camera.setMovingSpeed(10.0f);

	m_cube.buildCube({ -1.0f, -1.0f, -1.0f }, { 2.0f, 2.0f, 2.0f }, 1u, 1u, false, false);
	m_cube.rewind();
	m_sphere.buildSphere({ 0.0f, 0.0f, 0.0f }, 1.0f, 49u, 49u, true, true);

	wgpContext.addSampler(wgpCreateSampler(WGPUFilterMode_Linear, WGPUAddressMode_ClampToEdge, 1u, WGPUMipmapFilterMode_Nearest), SS_0);
	wgpContext.setClearColor({ 1.0f, 1.0f, 1.0f, 1.0f });

	m_uniformBuffer.createBuffer(sizeof(Uniforms), WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform);
	m_uniformModelBuffer.createBuffer(768u, WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform);
	m_uniformLightBuffer.createBuffer(128u, WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform);
	m_uniformMVPBuffer.createBuffer(64u, WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform);
	m_roughnessBuffer.createBuffer(4u, WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform);
	
	m_wgpTextureHDR.loadHDRIFromFile("res/textures/venice_sunset_1k.hdr", true, true);

	wgpContext.addSahderModule("PBR", "res/shader/pbr.wgsl");
	wgpContext.createRenderPipeline("PBR", "RP_PBR", VL_PTN, std::bind(&ImageBasedLighting::OnBindGroupLayoutsPBR, this), 4u);

	wgpContext.addSahderModule("SKYBOX", "res/shader/skybox.wgsl");
	wgpContext.createRenderPipeline("SKYBOX", "RP_SKYBOX", VL_P, std::bind(&ImageBasedLighting::OnBindGroupLayoutsSkybox, this), 4u);

	wgpContext.addSahderModule("IRRADIANCE", "res/shader/irradiance.wgsl");
	wgpContext.createRenderPipeline("IRRADIANCE", "RP_IRRADIANCE", VL_P, 
		std::bind(&ImageBasedLighting::OnBindGroupLayoutsIrradiance, this), 
		1u, 
		WGPUPrimitiveTopology_TriangleList,
		WGPUTextureFormat_RGBA16Float,
		false,
		false);

	wgpContext.addSahderModule("CUBE", "res/shader/cube_map.wgsl");
	wgpContext.createRenderPipeline("CUBE", "RP_CUBE", VL_P,
		std::bind(&ImageBasedLighting::OnBindGroupLayoutsCube, this),
		1u,
		WGPUPrimitiveTopology_TriangleList,
		WGPUTextureFormat_RGBA16Float,
		false,
		false);

	wgpContext.addSahderModule("PREFILTER", "res/shader/prefilter.wgsl");
	wgpContext.createRenderPipeline("PREFILTER", "RP_PREFILTER", VL_P,
		std::bind(&ImageBasedLighting::OnBindGroupLayoutsPrefilter, this),
		1u,
		WGPUPrimitiveTopology_TriangleList,
		WGPUTextureFormat_RGBA16Float,
		false,
		false);

	wgpContext.addSahderModule("BRDF", "res/shader/brdf.wgsl");
	wgpContext.createRenderPipeline("BRDF", "RP_BRDF", VL_NONE, 
		NULL, 
		1u, 
		WGPUPrimitiveTopology_TriangleList, 
		WGPUTextureFormat_RG16Float,
		false,
		false);

	m_uniforms.projectionMatrix = m_camera.getPerspectiveMatrix();
	m_uniforms.viewMatrix = m_camera.getViewMatrix();
	m_uniforms.envMatrix = m_camera.getRotationMatrix();
	m_uniforms.modelMatrix = glm::mat4(1.0f);
	m_uniforms.normalMatrix = glm::mat4(1.0f);
	m_uniforms.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_uniforms.camPosition = m_camera.getPosition();

	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), 0, &m_uniforms, sizeof(Uniforms));

	initMatrices();
	initIrradianceMatrices();
	initLights();

	m_wgpCube.create(m_cube);
	m_wgpSphere.create(m_sphere);

	m_wgpCube.setBindGroups("CUBE", std::bind(&ImageBasedLighting::OnBindGroupsCube, this));
	m_wgpTextureCube.createEmpty(512u, 512u, 6u, WGPUTextureUsage_TextureBinding | WGPUTextureUsage_RenderAttachment, WGPUTextureFormat_RGBA16Float);
	WgpRenderer::Draw(m_wgpTextureCube, std::bind(&ImageBasedLighting::OnDrawCube, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	m_wgpCube.setBindGroups("IRRADIANCE", std::bind(&ImageBasedLighting::OnBindGroupsIrradiance, this));
	m_wgpTextureIrradiance.createEmpty(32u, 32u, 6u, WGPUTextureUsage_TextureBinding | WGPUTextureUsage_RenderAttachment, WGPUTextureFormat_RGBA16Float);
	WgpRenderer::Draw(m_wgpTextureIrradiance, std::bind(&ImageBasedLighting::OnDrawIrradiance, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	m_wgpCube.setBindGroups("PREFILTER", std::bind(&ImageBasedLighting::OnBindGroupsPrefilter, this));
	m_wgpTexturePrefilter.createEmpty(256u, 256u, 6u, WGPUTextureUsage_TextureBinding | WGPUTextureUsage_RenderAttachment, WGPUTextureFormat_RGBA16Float, ROUGHNESS_LEVELS);
	WgpRenderer::Draw(m_wgpTexturePrefilter, std::bind(&ImageBasedLighting::OnDrawPrefilter, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	m_wgpTextureBrdf.createEmpty(512, 512, 1u, WGPUTextureUsage_TextureBinding | WGPUTextureUsage_RenderAttachment, WGPUTextureFormat_RG16Float);
	WgpRenderer::Draw(m_wgpTextureBrdf, std::bind(&ImageBasedLighting::OnDrawBrdf, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	m_wgpCube.setBindGroups("BG", std::bind(&ImageBasedLighting::OnBindGroupsSkybox, this));

	m_wgpSphere.create(m_sphere);
	m_wgpSphere.setBindGroups("BG", std::bind(&ImageBasedLighting::OnBindGroupsPBR, this));

	wgpContext.OnDraw = std::bind(&ImageBasedLighting::OnDraw, this, std::placeholders::_1);
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

	m_uniforms.projectionMatrix = m_camera.getPerspectiveMatrix();
	m_uniforms.viewMatrix = m_camera.getViewMatrix();
	m_uniforms.envMatrix = m_camera.getRotationMatrix();
	m_uniforms.normalMatrix = Camera::GetNormalMatrix(m_camera.getViewMatrix() * m_uniforms.modelMatrix);
	m_uniforms.camPosition = m_camera.getPosition();
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
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), offsetof(Uniforms, camPosition), &m_uniforms.camPosition, sizeof(Uniforms::camPosition));

	wgpuRenderPassEncoderSetViewport(renderPassEncoder, 0.0f, 0.0f, static_cast<float>(Application::Width), static_cast<float>(Application::Height), 0.0f, 1.0f);
	
	wgpuRenderPassEncoderSetPipeline(renderPassEncoder, wgpContext.renderPipelines.at("RP_SKYBOX"));
	m_wgpCube.draw(renderPassEncoder);

	wgpuRenderPassEncoderSetPipeline(renderPassEncoder, wgpContext.renderPipelines.at("RP_PBR"));
	m_wgpSphere.draw(renderPassEncoder, 12u);

	if (m_drawUi)
		renderUi(renderPassEncoder);
}

void ImageBasedLighting::OnMouseButtonDown(const Event::MouseButtonEvent& event) {
	if (event.button == Event::MouseButtonEvent::BUTTON_RIGHT) {
		Mouse::instance().attach(Application::Window, true, false, false);
	}
}

void ImageBasedLighting::OnMouseButtonUp(const Event::MouseButtonEvent& event) {
	if (event.button == Event::MouseButtonEvent::BUTTON_RIGHT) {
		Mouse::instance().detach();
	} 
}

void ImageBasedLighting::OnMouseMotion(const Event::MouseMoveEvent& event) {

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

std::vector<WGPUBindGroupLayout> ImageBasedLighting::OnBindGroupLayoutsPBR() {
	std::vector<WGPUBindGroupLayout> bindingLayouts(2);

	std::vector<WGPUBindGroupLayoutEntry> bindingLayoutEntries0(3);
	bindingLayoutEntries0[0].binding = 0u;
	bindingLayoutEntries0[0].visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
	bindingLayoutEntries0[0].buffer.type = WGPUBufferBindingType_Uniform;
	bindingLayoutEntries0[0].buffer.minBindingSize = sizeof(Uniforms);

	bindingLayoutEntries0[1].binding = 1u;
	bindingLayoutEntries0[1].visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
	bindingLayoutEntries0[1].buffer.type = WGPUBufferBindingType_Uniform;
	bindingLayoutEntries0[1].buffer.minBindingSize = 768u;

	bindingLayoutEntries0[2].binding = 2u;
	bindingLayoutEntries0[2].visibility = WGPUShaderStage_Fragment;
	bindingLayoutEntries0[2].buffer.type = WGPUBufferBindingType_Uniform;
	bindingLayoutEntries0[2].buffer.minBindingSize = 128u;

	WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor0 = {};
	bindGroupLayoutDescriptor0.entryCount = (uint32_t)bindingLayoutEntries0.size();
	bindGroupLayoutDescriptor0.entries = bindingLayoutEntries0.data();

	bindingLayouts[0] = wgpuDeviceCreateBindGroupLayout(wgpContext.device, &bindGroupLayoutDescriptor0);

	std::vector<WGPUBindGroupLayoutEntry> bindingLayoutEntries1(5);
	WGPUBindGroupLayoutEntry& samplerBindingLayout = bindingLayoutEntries1[0];
	samplerBindingLayout.binding = 0u;
	samplerBindingLayout.visibility = WGPUShaderStage_Fragment;
	samplerBindingLayout.sampler.type = WGPUSamplerBindingType_Filtering;

	WGPUBindGroupLayoutEntry& samplerBrdfBindingLayout = bindingLayoutEntries1[1];
	samplerBrdfBindingLayout.binding = 1u;
	samplerBrdfBindingLayout.visibility = WGPUShaderStage_Fragment;
	samplerBrdfBindingLayout.sampler.type = WGPUSamplerBindingType_Filtering;

	bindingLayoutEntries1[2].binding = 2u;
	bindingLayoutEntries1[2].visibility = WGPUShaderStage_Fragment;
	bindingLayoutEntries1[2].texture.viewDimension = WGPUTextureViewDimension_2D;
	bindingLayoutEntries1[2].texture.sampleType = WGPUTextureSampleType_Float;

	bindingLayoutEntries1[3].binding = 3u;
	bindingLayoutEntries1[3].visibility = WGPUShaderStage_Fragment;
	bindingLayoutEntries1[3].texture.viewDimension = WGPUTextureViewDimension_Cube;
	bindingLayoutEntries1[3].texture.sampleType = WGPUTextureSampleType_Float;

	bindingLayoutEntries1[4].binding = 4u;
	bindingLayoutEntries1[4].visibility = WGPUShaderStage_Fragment;
	bindingLayoutEntries1[4].texture.viewDimension = WGPUTextureViewDimension_Cube;
	bindingLayoutEntries1[4].texture.sampleType = WGPUTextureSampleType_Float;

	WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor1 = {};
	bindGroupLayoutDescriptor1.entryCount = (uint32_t)bindingLayoutEntries1.size();
	bindGroupLayoutDescriptor1.entries = bindingLayoutEntries1.data();

	bindingLayouts[1] = wgpuDeviceCreateBindGroupLayout(wgpContext.device, &bindGroupLayoutDescriptor1);

	return bindingLayouts;
}

std::vector<WGPUBindGroup> ImageBasedLighting::OnBindGroupsPBR() {
	std::vector<WGPUBindGroup> bindGroups(2);

	std::vector<WGPUBindGroupEntry> bindGroupEntries0(3);

	bindGroupEntries0[0].binding = 0u;
	bindGroupEntries0[0].buffer = m_uniformBuffer.getBuffer();
	bindGroupEntries0[0].offset = 0u;
	bindGroupEntries0[0].size = sizeof(Uniforms);

	bindGroupEntries0[1].binding = 1u;
	bindGroupEntries0[1].buffer = m_uniformModelBuffer.getBuffer();
	bindGroupEntries0[1].offset = 0u;
	bindGroupEntries0[1].size = 768u;

	bindGroupEntries0[2].binding = 2u;
	bindGroupEntries0[2].buffer = m_uniformLightBuffer.getBuffer();
	bindGroupEntries0[2].offset = 0u;
	bindGroupEntries0[2].size = 128u;

	WGPUBindGroupDescriptor bindGroupDesc0 = {};
	bindGroupDesc0.layout = wgpuRenderPipelineGetBindGroupLayout(wgpContext.renderPipelines.at("RP_PBR"), 0u);
	bindGroupDesc0.entryCount = (uint32_t)bindGroupEntries0.size();
	bindGroupDesc0.entries = bindGroupEntries0.data();
	bindGroups[0] = wgpuDeviceCreateBindGroup(wgpContext.device, &bindGroupDesc0);

	std::vector<WGPUBindGroupEntry> bindGroupEntries1(5);

	bindGroupEntries1[0].binding = 0u;
	bindGroupEntries1[0].sampler = wgpContext.getSampler(SS_LINEAR_REPEAT);


	bindGroupEntries1[1].binding = 1u;
	bindGroupEntries1[1].sampler = wgpContext.getSampler(SS_0);

	bindGroupEntries1[2].binding = 2u;
	bindGroupEntries1[2].textureView = m_wgpTextureBrdf.getTextureView();


	bindGroupEntries1[3].binding = 3u;
	bindGroupEntries1[3].textureView = m_wgpTextureIrradiance.getTextureView();

	bindGroupEntries1[4].binding = 4u;
	bindGroupEntries1[4].textureView = m_wgpTexturePrefilter.getTextureView();

	WGPUBindGroupDescriptor bindGroupDesc1 = {};
	bindGroupDesc1.layout = wgpuRenderPipelineGetBindGroupLayout(wgpContext.renderPipelines.at("RP_PBR"), 1u);
	bindGroupDesc1.entryCount = (uint32_t)bindGroupEntries1.size();
	bindGroupDesc1.entries = bindGroupEntries1.data();
	bindGroups[1] = wgpuDeviceCreateBindGroup(wgpContext.device, &bindGroupDesc1);

	return bindGroups;
}

std::vector<WGPUBindGroupLayout> ImageBasedLighting::OnBindGroupLayoutsIrradiance() {
	std::vector<WGPUBindGroupLayout> bindingLayouts(1);

	std::vector<WGPUBindGroupLayoutEntry> bindingLayoutEntries(3);
	bindingLayoutEntries[0].binding = 0u;
	bindingLayoutEntries[0].visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
	bindingLayoutEntries[0].buffer.type = WGPUBufferBindingType_Uniform;
	bindingLayoutEntries[0].buffer.minBindingSize = 64u;

	bindingLayoutEntries[1].binding = 1u;
	bindingLayoutEntries[1].visibility = WGPUShaderStage_Fragment;
	bindingLayoutEntries[1].sampler.type = WGPUSamplerBindingType_Filtering;

	bindingLayoutEntries[2].binding = 2u;
	bindingLayoutEntries[2].visibility = WGPUShaderStage_Fragment;
	bindingLayoutEntries[2].texture.viewDimension = WGPUTextureViewDimension_Cube;
	bindingLayoutEntries[2].texture.sampleType = WGPUTextureSampleType_Float;

	WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor = {};
	bindGroupLayoutDescriptor.entryCount = (uint32_t)bindingLayoutEntries.size();
	bindGroupLayoutDescriptor.entries = bindingLayoutEntries.data();

	bindingLayouts[0] = wgpuDeviceCreateBindGroupLayout(wgpContext.device, &bindGroupLayoutDescriptor);

	return bindingLayouts;
}

std::vector<WGPUBindGroup> ImageBasedLighting::OnBindGroupsIrradiance() {
	std::vector<WGPUBindGroup> bindGroups(1);
	std::vector<WGPUBindGroupEntry> bindGroupEntries(3);

	bindGroupEntries[0].binding = 0u;
	bindGroupEntries[0].buffer = m_uniformMVPBuffer.getBuffer();
	bindGroupEntries[0].offset = 0u;
	bindGroupEntries[0].size = wgpuBufferGetSize(m_uniformMVPBuffer.getBuffer());

	bindGroupEntries[1].binding = 1u;
	bindGroupEntries[1].sampler = wgpContext.getSampler(SS_LINEAR_REPEAT);

	bindGroupEntries[2].binding = 2u;
	bindGroupEntries[2].textureView = m_wgpTextureCube.getTextureView();

	WGPUBindGroupDescriptor bindGroupDesc = {};
	bindGroupDesc.layout = wgpuRenderPipelineGetBindGroupLayout(wgpContext.renderPipelines.at("RP_IRRADIANCE"), 0u);
	bindGroupDesc.entryCount = (uint32_t)bindGroupEntries.size();
	bindGroupDesc.entries = bindGroupEntries.data();
	bindGroups[0] = wgpuDeviceCreateBindGroup(wgpContext.device, &bindGroupDesc);

	return bindGroups;
}

std::vector<WGPUBindGroupLayout> ImageBasedLighting::OnBindGroupLayoutsCube() {
	std::vector<WGPUBindGroupLayout> bindingLayouts(1);

	std::vector<WGPUBindGroupLayoutEntry> bindingLayoutEntries(3);
	bindingLayoutEntries[0].binding = 0u;
	bindingLayoutEntries[0].visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
	bindingLayoutEntries[0].buffer.type = WGPUBufferBindingType_Uniform;
	bindingLayoutEntries[0].buffer.minBindingSize = 64u;

	bindingLayoutEntries[1].binding = 1u;
	bindingLayoutEntries[1].visibility = WGPUShaderStage_Fragment;
	bindingLayoutEntries[1].sampler.type = WGPUSamplerBindingType_Filtering;

	bindingLayoutEntries[2].binding = 2u;
	bindingLayoutEntries[2].visibility = WGPUShaderStage_Fragment;
	bindingLayoutEntries[2].texture.viewDimension = WGPUTextureViewDimension_2D;
	bindingLayoutEntries[2].texture.sampleType = WGPUTextureSampleType_Float;

	WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor = {};
	bindGroupLayoutDescriptor.entryCount = (uint32_t)bindingLayoutEntries.size();
	bindGroupLayoutDescriptor.entries = bindingLayoutEntries.data();

	bindingLayouts[0] = wgpuDeviceCreateBindGroupLayout(wgpContext.device, &bindGroupLayoutDescriptor);

	return bindingLayouts;
}

std::vector<WGPUBindGroup> ImageBasedLighting::OnBindGroupsCube() {
	std::vector<WGPUBindGroup> bindGroups(1);
	std::vector<WGPUBindGroupEntry> bindGroupEntries(3);

	bindGroupEntries[0].binding = 0u;
	bindGroupEntries[0].buffer = m_uniformMVPBuffer.getBuffer();
	bindGroupEntries[0].offset = 0u;
	bindGroupEntries[0].size = wgpuBufferGetSize(m_uniformMVPBuffer.getBuffer());

	bindGroupEntries[1].binding = 1u;
	bindGroupEntries[1].sampler = wgpContext.getSampler(SS_LINEAR_REPEAT);

	bindGroupEntries[2].binding = 2u;
	bindGroupEntries[2].textureView = m_wgpTextureHDR.getTextureView();

	WGPUBindGroupDescriptor bindGroupDesc = {};
	bindGroupDesc.layout = wgpuRenderPipelineGetBindGroupLayout(wgpContext.renderPipelines.at("RP_CUBE"), 0u);
	bindGroupDesc.entryCount = (uint32_t)bindGroupEntries.size();
	bindGroupDesc.entries = bindGroupEntries.data();
	bindGroups[0] = wgpuDeviceCreateBindGroup(wgpContext.device, &bindGroupDesc);

	return bindGroups;
}

std::vector<WGPUBindGroupLayout> ImageBasedLighting::OnBindGroupLayoutsPrefilter() {
	std::vector<WGPUBindGroupLayout> bindingLayouts(1);

	std::vector<WGPUBindGroupLayoutEntry> bindingLayoutEntries(4);
	bindingLayoutEntries[0].binding = 0u;
	bindingLayoutEntries[0].visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
	bindingLayoutEntries[0].buffer.type = WGPUBufferBindingType_Uniform;
	bindingLayoutEntries[0].buffer.minBindingSize = 64u;

	bindingLayoutEntries[1].binding = 1u;
	bindingLayoutEntries[1].visibility = WGPUShaderStage_Fragment;
	bindingLayoutEntries[1].sampler.type = WGPUSamplerBindingType_Filtering;

	bindingLayoutEntries[2].binding = 2u;
	bindingLayoutEntries[2].visibility = WGPUShaderStage_Fragment;
	bindingLayoutEntries[2].texture.viewDimension = WGPUTextureViewDimension_Cube;
	bindingLayoutEntries[2].texture.sampleType = WGPUTextureSampleType_Float;

	bindingLayoutEntries[3].binding = 3u;
	bindingLayoutEntries[3].visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
	bindingLayoutEntries[3].buffer.type = WGPUBufferBindingType_Uniform;
	bindingLayoutEntries[3].buffer.minBindingSize = 4u;

	WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor = {};
	bindGroupLayoutDescriptor.entryCount = (uint32_t)bindingLayoutEntries.size();
	bindGroupLayoutDescriptor.entries = bindingLayoutEntries.data();

	bindingLayouts[0] = wgpuDeviceCreateBindGroupLayout(wgpContext.device, &bindGroupLayoutDescriptor);

	return bindingLayouts;
}

std::vector<WGPUBindGroup> ImageBasedLighting::OnBindGroupsPrefilter() {
	std::vector<WGPUBindGroup> bindGroups(1);
	std::vector<WGPUBindGroupEntry> bindGroupEntries(4);

	bindGroupEntries[0].binding = 0u;
	bindGroupEntries[0].buffer = m_uniformMVPBuffer.getBuffer();
	bindGroupEntries[0].offset = 0u;
	bindGroupEntries[0].size = wgpuBufferGetSize(m_uniformMVPBuffer.getBuffer());

	bindGroupEntries[1].binding = 1u;
	bindGroupEntries[1].sampler = wgpContext.getSampler(SS_LINEAR_REPEAT);

	bindGroupEntries[2].binding = 2u;
	bindGroupEntries[2].textureView = m_wgpTextureCube.getTextureView();

	bindGroupEntries[3].binding = 3u;
	bindGroupEntries[3].buffer = m_roughnessBuffer.getBuffer();
	bindGroupEntries[3].offset = 0u;
	bindGroupEntries[3].size = wgpuBufferGetSize(m_roughnessBuffer.getBuffer());

	WGPUBindGroupDescriptor bindGroupDesc = {};
	bindGroupDesc.layout = wgpuRenderPipelineGetBindGroupLayout(wgpContext.renderPipelines.at("RP_PREFILTER"), 0u);
	bindGroupDesc.entryCount = (uint32_t)bindGroupEntries.size();
	bindGroupDesc.entries = bindGroupEntries.data();
	bindGroups[0] = wgpuDeviceCreateBindGroup(wgpContext.device, &bindGroupDesc);

	return bindGroups;
}

std::vector<WGPUBindGroupLayout> ImageBasedLighting::OnBindGroupLayoutsSkybox() {
	std::vector<WGPUBindGroupLayout> bindingLayouts(1);

	std::vector<WGPUBindGroupLayoutEntry> bindingLayoutEntries(3);
	bindingLayoutEntries[0].binding = 0u;
	bindingLayoutEntries[0].visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
	bindingLayoutEntries[0].buffer.type = WGPUBufferBindingType_Uniform;
	bindingLayoutEntries[0].buffer.minBindingSize = sizeof(Uniforms);

	bindingLayoutEntries[1].binding = 1u;
	bindingLayoutEntries[1].visibility = WGPUShaderStage_Fragment;
	bindingLayoutEntries[1].sampler.type = WGPUSamplerBindingType_Filtering;

	bindingLayoutEntries[2].binding = 2u;
	bindingLayoutEntries[2].visibility = WGPUShaderStage_Fragment;
	bindingLayoutEntries[2].texture.viewDimension = WGPUTextureViewDimension_Cube;
	bindingLayoutEntries[2].texture.sampleType = WGPUTextureSampleType_Float;

	WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor = {};
	bindGroupLayoutDescriptor.entryCount = (uint32_t)bindingLayoutEntries.size();
	bindGroupLayoutDescriptor.entries = bindingLayoutEntries.data();

	bindingLayouts[0] = wgpuDeviceCreateBindGroupLayout(wgpContext.device, &bindGroupLayoutDescriptor);

	return bindingLayouts;
}

std::vector<WGPUBindGroup> ImageBasedLighting::OnBindGroupsSkybox() {
	std::vector<WGPUBindGroup> bindGroups(1);
	std::vector<WGPUBindGroupEntry> bindGroupEntries(3);

	bindGroupEntries[0].binding = 0u;
	bindGroupEntries[0].buffer = m_uniformBuffer.getBuffer();
	bindGroupEntries[0].offset = 0u;
	bindGroupEntries[0].size = wgpuBufferGetSize(m_uniformBuffer.getBuffer());

	bindGroupEntries[1].binding = 1u;
	bindGroupEntries[1].sampler = wgpContext.getSampler(SS_LINEAR_REPEAT);

	bindGroupEntries[2].binding = 2u;
	bindGroupEntries[2].textureView = m_wgpTextureIrradiance.getTextureView();

	WGPUBindGroupDescriptor bindGroupDesc = {};
	bindGroupDesc.layout = wgpuRenderPipelineGetBindGroupLayout(wgpContext.renderPipelines.at("RP_SKYBOX"), 0u);
	bindGroupDesc.entryCount = (uint32_t)bindGroupEntries.size();
	bindGroupDesc.entries = bindGroupEntries.data();
	bindGroups[0] = wgpuDeviceCreateBindGroup(wgpContext.device, &bindGroupDesc);

	return bindGroups;
}

void ImageBasedLighting::OnDrawIrradiance(const WGPURenderPassEncoder& renderPassEncoder, uint32_t layer, uint32_t mip) {
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformMVPBuffer.getBuffer(), 0u, &m_mvpInvCube[layer], 64u);
	wgpuRenderPassEncoderSetPipeline(renderPassEncoder, wgpContext.renderPipelines.at("RP_IRRADIANCE"));
	wgpuRenderPassEncoderSetBindGroup(renderPassEncoder, 0u, m_wgpCube.getMeshes().back().getBindGroups("IRRADIANCE")[0], 0u, NULL);
	m_wgpCube.draw(renderPassEncoder);
}

void ImageBasedLighting::OnDrawCube(const WGPURenderPassEncoder& renderPassEncoder, uint32_t layer, uint32_t mip) {
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformMVPBuffer.getBuffer(), 0u, &m_mvpCube[layer], 64u);
	wgpuRenderPassEncoderSetPipeline(renderPassEncoder, wgpContext.renderPipelines.at("RP_CUBE"));
	wgpuRenderPassEncoderSetBindGroup(renderPassEncoder, 0u, m_wgpCube.getMeshes().back().getBindGroups("CUBE")[0], 0u, NULL);
	m_wgpCube.draw(renderPassEncoder);
}

void ImageBasedLighting::OnDrawPrefilter(const WGPURenderPassEncoder& renderPassEncoder, uint32_t layer, uint32_t mip) {
	float roughness_val = (float)mip / (float)(ROUGHNESS_LEVELS - 1);
	wgpuQueueWriteBuffer(wgpContext.queue, m_roughnessBuffer.getBuffer(), 0, &roughness_val, sizeof(float));
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformMVPBuffer.getBuffer(), 0u, &m_mvpInvCube[layer], 64u);
	wgpuRenderPassEncoderSetPipeline(renderPassEncoder, wgpContext.renderPipelines.at("RP_PREFILTER"));
	wgpuRenderPassEncoderSetBindGroup(renderPassEncoder, 0u, m_wgpCube.getMeshes().back().getBindGroups("PREFILTER")[0], 0u, NULL);
	m_wgpCube.draw(renderPassEncoder);
}

void ImageBasedLighting::OnDrawBrdf(const WGPURenderPassEncoder& renderPassEncoder, uint32_t layer, uint32_t mip) {
	wgpuRenderPassEncoderSetPipeline(renderPassEncoder, wgpContext.renderPipelines.at("RP_BRDF"));
	wgpuRenderPassEncoderDraw(renderPassEncoder, 3, 1, 0, 0);
}

void ImageBasedLighting::initMatrices() {
	const float distance = 2.8f;
	uint32_t idx = 0;
	for (uint32_t y = 0; y < 2; ++y) {
		for (uint32_t x = 0; x < 6; ++x) {
			float pos_x = (float)x * distance - (distance * (6 - 1)) / 2.0f;
			float pos_y = (float)y * distance - distance / 2.0f;
			m_models[idx] = glm::mat4(1.0f);
			m_models[idx] = glm::translate(m_models[idx], glm::vec3(pos_x, pos_y, 0.0f));
			idx++;
		}
	}
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformModelBuffer.getBuffer(), 0u, &m_models[0], 768u);
}

void ImageBasedLighting::initLights() {
	m_lights[0].position = { -10.0f, 10.0f, 10.0f };
	m_lights[1].position = { 10.0f, 10.0f, 10.0f };
	m_lights[2].position = { -10.0f, -10.0f, 10.0f };
	m_lights[3].position = { 10.0f, -10.0f, 10.0f };

	m_lights[0].color = { 100.0f, 100.0f, 100.0f };
	m_lights[1].color = { 100.0f, 100.0f, 100.0f };
	m_lights[2].color = { 100.0f, 100.0f, 100.0f };
	m_lights[3].color = { 100.0f, 100.0f, 100.0f };

	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformLightBuffer.getBuffer(), 0u, &m_lights[0], 128u);
}

void ImageBasedLighting::initIrradianceMatrices() {

	m_mvpInvCube[0] = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f) * glm::transpose(glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
	m_mvpInvCube[1] = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f) * glm::transpose(glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));

	m_mvpInvCube[2] = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f) * glm::transpose(glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
	m_mvpInvCube[3] = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f) * glm::transpose(glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));

	m_mvpInvCube[4] = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f) * glm::transpose(glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
	m_mvpInvCube[5] = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f) * glm::transpose(glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f)));

	m_mvpCube[0] = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f) * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	m_mvpCube[1] = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f) * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));

	m_mvpCube[2] = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f) * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
	m_mvpCube[3] = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f) * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	m_mvpCube[4] = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f) * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	m_mvpCube[5] = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f) * glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
}