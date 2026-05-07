#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_wgpu.h>
#include <imgui_impl_glfw.h>
#include <imgui_internal.h>

#include <glm/ext.hpp>

#include <WebGPU/WgpContext.h>
#include <WebGPU/WgpRenderer.h>

#include "SkinnedMesh.h"
#include "Application.h"
#include "Mouse.h"

SkinnedMesh::SkinnedMesh(StateMachine& machine) : State(machine, States::SKINNED_MESH) {
	wgpSetSurfaceColorFormat(WGPUTextureFormat::WGPUTextureFormat_BGRA8Unorm, Application::OnSurfaceChange);

	m_camera.perspective(glm::radians(72.0f), static_cast<float>(Application::Width) / static_cast<float>(Application::Height), 0.1f, 2000.0f);
	m_camera.orthographic(0.0f, static_cast<float>(Application::Width), 0.0f, static_cast<float>(Application::Height), -1.0f, 1.0f);
	m_camera.lookAt(glm::vec3(0.0f, 50.0f, -100.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	m_camera.setRotationSpeed(0.1f);
	m_camera.setMovingSpeed(50.0f);

	m_dragon.loadModel("res/models/dragon_vrip_res4.ply", glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, glm::vec3(0.0f, 0.0f, 0.0f), 500.0f);
	m_dragon.generateNormals();

	m_quad.buildQuadXZ({ -100.0f, 20.0f, -100.0f }, { 200.0f, 200.0f }, 1u, 1u, false, true);

	m_uniformBuffer.createBuffer(sizeof(Uniforms), WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform);

	wgpContext.addSampler(wgpCreateSampler(WGPUFilterMode_Nearest, WGPUAddressMode_ClampToEdge, 1u, WGPUMipmapFilterMode_Nearest, WGPUCompareFunction_Less), SS_0);
	wgpContext.setClearColor({ 0.5f, 0.5f, 0.5f, 1.0f });
	wgpContext.addSahderModule("SHADOW_BASE", SHADOW_BASE_WGSL, true);
	wgpContext.createRenderPipeline("SHADOW_BASE", "RP_COLOR", VL_PN, std::bind(&SkinnedMesh::OnBindGroupLayouts, this));
    
	wgpContext.addSahderModule("SHADOW", SHADOW_WGSL, true);
	wgpContext.createRenderPipeline("SHADOW", "RP_SHADOW", 
		VL_PN, 
		std::bind(&SkinnedMesh::OnBindGroupLayoutsShadow, this),
		1u, 
		WGPUPrimitiveTopology_TriangleList,
		WGPUTextureFormat_Undefined,
		WGPUTextureFormat_Depth32Float,
		WGPUCompareFunction_Less,
		true,
		false,
		false
	);
	

	m_lightProjection = glm::ortho(-80.0f, 80.0f, -80.0f, 80.0f, -200.0f, 300.0f);
	m_lightView = glm::lookAt(glm::vec3(50.0f, 100.0f, -100.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	m_uniforms.projection = m_camera.getPerspectiveMatrix();
	m_uniforms.view = m_camera.getViewMatrix();
	m_uniforms.env = m_camera.getRotationMatrix();
	m_uniforms.model = glm::translate(glm::vec3(0.0f, -45.0f, 0.0f));
	m_uniforms.normal = Camera::GetNormalMatrix(m_camera.getViewMatrix() * m_uniforms.model);
	m_uniforms.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_uniforms.camPosition = m_camera.getPosition();
	m_uniforms.lightVP = m_lightProjection * m_lightView;
	m_uniforms.shadow = Camera::BIAS * m_uniforms.lightVP;
	m_uniforms.lightPosition = glm::vec3(50.0f, 100.0f, -100.0f);
	
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), 0, &m_uniforms, sizeof(Uniforms));

	m_wgpTextureShadow.createEmpty(1024u, 1024u, 1u, WGPUTextureUsage_TextureBinding | WGPUTextureUsage_RenderAttachment, WGPUTextureFormat_Depth32Float);
	
	m_wgpDragon.create(m_dragon);
	m_wgpDragon.addBindGroups("SHADOW", std::bind(&SkinnedMesh::OnBindGroupsShadow, this));
	m_wgpDragon.addBindGroups("COLOR", std::bind(&SkinnedMesh::OnBindGroups, this));

	m_wgpQuad.create(m_quad);
	m_wgpQuad.addBindGroups("SHADOW", std::bind(&SkinnedMesh::OnBindGroupsShadow, this));
	m_wgpQuad.addBindGroups("COLOR", std::bind(&SkinnedMesh::OnBindGroups, this));

	wgpContext.OnDraw = std::bind(&SkinnedMesh::OnDraw, this, std::placeholders::_1);
}

SkinnedMesh::~SkinnedMesh() {
	m_uniformBuffer.markForDelete();
	m_wgpTextureShadow.markForDelete();
}

void SkinnedMesh::fixedUpdate() {

}

void SkinnedMesh::update() {
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

	glm::vec3 position = m_camera.getPosition();

	RotateY(position, m_dt);
	m_camera.setPosition(position, true);

	m_uniforms.projection = m_camera.getPerspectiveMatrix();
	m_uniforms.view = m_camera.getViewMatrix();
	m_uniforms.env = m_camera.getRotationMatrix();
	m_uniforms.model = glm::translate(glm::vec3(0.0f, -45.0f, 0.0f));
	m_uniforms.normal = Camera::GetNormalMatrix(m_camera.getViewMatrix() * m_uniforms.model);
	m_uniforms.camPosition = position;
	m_uniforms.lightVP = m_lightProjection * m_lightView;
	m_uniforms.shadow = Camera::BIAS * m_uniforms.lightVP;
}

void SkinnedMesh::render() {
	wgpDraw();
}

void SkinnedMesh::OnDraw(const WGPURenderPassEncoder& renderPassEncoder) {

	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), offsetof(Uniforms, projection), &m_uniforms.projection, sizeof(Uniforms::projection));
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), offsetof(Uniforms, view), &m_uniforms.view, sizeof(Uniforms::view));
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), offsetof(Uniforms, env), &m_uniforms.env, sizeof(Uniforms::env));
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), offsetof(Uniforms, model), &m_uniforms.model, sizeof(Uniforms::model));
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), offsetof(Uniforms, normal), &m_uniforms.normal, sizeof(Uniforms::normal));
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), offsetof(Uniforms, camPosition), &m_uniforms.camPosition, sizeof(Uniforms::camPosition));
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), offsetof(Uniforms, lightVP), &m_uniforms.lightVP, sizeof(Uniforms::lightVP));
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), offsetof(Uniforms, shadow), &m_uniforms.shadow, sizeof(Uniforms::shadow));

	WgpRenderer::DrawDepth(m_wgpTextureShadow, std::bind(&SkinnedMesh::OnDrawShadow, this, std::placeholders::_1));


	wgpuRenderPassEncoderSetViewport(renderPassEncoder, 0.0f, 0.0f, static_cast<float>(Application::Width), static_cast<float>(Application::Height), 0.0f, 1.0f);

	wgpuRenderPassEncoderSetPipeline(renderPassEncoder, wgpContext.renderPipelines.at("RP_COLOR"));
	m_wgpDragon.setBindGroupsSlot("COLOR");
	m_wgpDragon.draw(renderPassEncoder);

	m_wgpQuad.setBindGroupsSlot("COLOR");
	m_wgpQuad.draw(renderPassEncoder);

	if (m_drawUi)
		renderUi(renderPassEncoder);
}

void SkinnedMesh::OnMouseButtonDown(const Event::MouseButtonEvent& event) {
	if (event.button == Event::MouseButtonEvent::BUTTON_RIGHT) {
		Mouse::instance().attach(Application::Window, true, false, false);
	}
}

void SkinnedMesh::OnMouseButtonUp(const Event::MouseButtonEvent& event) {
	if (event.button == Event::MouseButtonEvent::BUTTON_RIGHT) {
		Mouse::instance().detach();
	} 
}

void SkinnedMesh::OnMouseMotion(const Event::MouseMoveEvent& event) {

}

void SkinnedMesh::OnScroll(double xoffset, double yoffset) {

}

void SkinnedMesh::OnKeyDown(const Event::KeyboardEvent& event) {

}

void SkinnedMesh::OnKeyUp(const Event::KeyboardEvent& event) {

}

void SkinnedMesh::resize(int deltaW, int deltaH) {
	m_camera.perspective(glm::radians(72.0f), static_cast<float>(Application::Width) / static_cast<float>(Application::Height), 0.1f, 2000.0f);
	m_camera.orthographic(0.0f, static_cast<float>(Application::Width), 0.0f, static_cast<float>(Application::Height), -1.0f, 1.0f);
}

void SkinnedMesh::renderUi(const WGPURenderPassEncoder& renderPassEncoder) {
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

std::vector<WGPUBindGroupLayout> SkinnedMesh::OnBindGroupLayouts() {
	std::vector<WGPUBindGroupLayout> bindingLayouts(1);

	std::vector<WGPUBindGroupLayoutEntry> bindingLayoutEntries(3);
	bindingLayoutEntries[0].binding = 0u;
	bindingLayoutEntries[0].visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
	bindingLayoutEntries[0].buffer.type = WGPUBufferBindingType::WGPUBufferBindingType_Uniform;
	bindingLayoutEntries[0].buffer.minBindingSize = sizeof(Uniforms);

	bindingLayoutEntries[1].binding = 1u;
	bindingLayoutEntries[1].visibility = WGPUShaderStage_Fragment;
	bindingLayoutEntries[1].texture.viewDimension = WGPUTextureViewDimension_2D;
	bindingLayoutEntries[1].texture.sampleType = WGPUTextureSampleType_Depth;

	bindingLayoutEntries[2].binding = 2u;
	bindingLayoutEntries[2].visibility = WGPUShaderStage_Fragment;
	bindingLayoutEntries[2].sampler.type = WGPUSamplerBindingType_Comparison;

	WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor = {};
	bindGroupLayoutDescriptor.entryCount = (uint32_t)bindingLayoutEntries.size();
	bindGroupLayoutDescriptor.entries = bindingLayoutEntries.data();

	bindingLayouts[0] = wgpuDeviceCreateBindGroupLayout(wgpContext.device, &bindGroupLayoutDescriptor);

	return bindingLayouts;
}

std::vector<WGPUBindGroup> SkinnedMesh::OnBindGroups() {
	std::vector<WGPUBindGroup> bindGroups(1);

	std::vector<WGPUBindGroupEntry> bindGroupEntries(3);
	bindGroupEntries[0].binding = 0u;
	bindGroupEntries[0].buffer = m_uniformBuffer.getBuffer();
	bindGroupEntries[0].offset = 0u;
	bindGroupEntries[0].size = sizeof(Uniforms);

	bindGroupEntries[1].binding = 1u;
	bindGroupEntries[1].textureView = m_wgpTextureShadow.getTextureView();

	bindGroupEntries[2].binding = 2u;
	bindGroupEntries[2].sampler = wgpContext.getSampler(SS_0);

	WGPUBindGroupDescriptor bindGroupDesc = {};
	bindGroupDesc.layout = wgpuRenderPipelineGetBindGroupLayout(wgpContext.renderPipelines.at("RP_COLOR"), 0u);
	bindGroupDesc.entryCount = (uint32_t)bindGroupEntries.size();
	bindGroupDesc.entries = bindGroupEntries.data();

	bindGroups[0] = wgpuDeviceCreateBindGroup(wgpContext.device, &bindGroupDesc);

	return bindGroups;
}

std::vector<WGPUBindGroupLayout> SkinnedMesh::OnBindGroupLayoutsShadow() {
	std::vector<WGPUBindGroupLayout> bindingLayouts(1);

	std::vector<WGPUBindGroupLayoutEntry> bindingLayoutEntries(1);
	bindingLayoutEntries[0].binding = 0u;
	bindingLayoutEntries[0].visibility = WGPUShaderStage_Vertex;
	bindingLayoutEntries[0].buffer.type = WGPUBufferBindingType::WGPUBufferBindingType_Uniform;
	bindingLayoutEntries[0].buffer.minBindingSize = sizeof(Uniforms);

	WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor = {};
	bindGroupLayoutDescriptor.entryCount = (uint32_t)bindingLayoutEntries.size();
	bindGroupLayoutDescriptor.entries = bindingLayoutEntries.data();

	bindingLayouts[0] = wgpuDeviceCreateBindGroupLayout(wgpContext.device, &bindGroupLayoutDescriptor);

	return bindingLayouts;
}

std::vector<WGPUBindGroup> SkinnedMesh::OnBindGroupsShadow() {
	std::vector<WGPUBindGroup> bindGroups(1);

	std::vector<WGPUBindGroupEntry> bindGroupEntries(1);
	bindGroupEntries[0].binding = 0u;
	bindGroupEntries[0].buffer = m_uniformBuffer.getBuffer();
	bindGroupEntries[0].offset = 0u;
	bindGroupEntries[0].size = sizeof(Uniforms);

	WGPUBindGroupDescriptor bindGroupDesc = {};
	bindGroupDesc.layout = wgpuRenderPipelineGetBindGroupLayout(wgpContext.renderPipelines.at("RP_SHADOW"), 0u);
	bindGroupDesc.entryCount = (uint32_t)bindGroupEntries.size();
	bindGroupDesc.entries = bindGroupEntries.data();

	bindGroups[0] = wgpuDeviceCreateBindGroup(wgpContext.device, &bindGroupDesc);

	return bindGroups;
}

void SkinnedMesh::OnDrawShadow(const WGPURenderPassEncoder& renderPassEncoder) {
	wgpuRenderPassEncoderSetPipeline(renderPassEncoder, wgpContext.renderPipelines.at("RP_SHADOW"));
	m_wgpDragon.setBindGroupsSlot("SHADOW");
	m_wgpDragon.draw(renderPassEncoder);

	m_wgpQuad.setBindGroupsSlot("SHADOW");
	m_wgpQuad.draw(renderPassEncoder);
}

glm::vec3& SkinnedMesh::RotateY(glm::vec3& p, float rad, const glm::vec3& centerOfRotation) {
	float x = p[0] - centerOfRotation[0];
	float z = p[2] - centerOfRotation[2];

	p[0] = z * sinf(rad) + x * cosf(rad) + centerOfRotation[0];
	p[2] = z * cosf(rad) - x * sinf(rad) + centerOfRotation[2];

	return p;
}