#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_wgpu.h>
#include <imgui_impl_glfw.h>
#include <imgui_internal.h>

#include <WebGPU/WgpContext.h>

#include "Application.h"
#include "Mouse.h"
#include "PrimitivePicking.h"

PrimitivePicking::PrimitivePicking(StateMachine& machine) : State(machine, States::PRIMITIVE_PICKING) {
	Mouse::instance().attach(Application::Window, false, true);

	wgpSetSurfaceColorFormat(WGPUTextureFormat::WGPUTextureFormat_BGRA8Unorm, Application::OnSurfaceChange);

	wgpVertexAttribute(VL_0).push_back(WGPUVertexAttribute{ NULL, WGPUVertexFormat_Float32x3, 0u, 0u });
	wgpVertexAttribute(VL_0).push_back(WGPUVertexAttribute{ NULL, WGPUVertexFormat_Float32x3, 3 * sizeof(float), 1u });
	wgpVertexAttribute(VL_0).push_back(WGPUVertexAttribute{ NULL, WGPUVertexFormat_Uint32, 6 * sizeof(float), 2u });
	wgpVertexBufferLayout(VL_0).push_back(WGPUVertexBufferLayout{ NULL, WGPUVertexStepMode_Vertex, 6u * sizeof(float) + sizeof(unsigned int) , wgpVertexAttribute(VL_0).size(), wgpVertexAttribute(VL_0).data()});

	m_camera.perspective(glm::radians(72.0f), static_cast<float>(Application::Width) / static_cast<float>(Application::Height), 0.1f, 2000.0f);
	m_camera.orthographic(0.0f, static_cast<float>(Application::Width), 0.0f, static_cast<float>(Application::Height), -1.0f, 1.0f);
	m_camera.lookAt(glm::vec3(0.0f, 12.0f, 25.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	m_camera.setMovingSpeed(20.0f);
	m_camera.setRotationSpeed(0.1f);

	m_teapot.loadModel("res/models/teapot.obj", false, false, true);
	m_trackball.reshape(Application::Width, Application::Height);

	wgpContext.setClearColor({ 0.0f, 0.0f, 1.0f, 1.0f });

	m_uniformBuffer.createBuffer(sizeof(Uniforms), WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform);
	m_computeBuffer.createBuffer(2 * sizeof(glm::mat4) + 4 * sizeof(float), WGPUBufferUsage_CopyDst | WGPUBufferUsage_CopySrc | WGPUBufferUsage_Uniform| WGPUBufferUsage_Storage);

	m_indexTexture.createEmpty(Application::Width, Application::Height, 1u, WGPUTextureUsage_TextureBinding | WGPUTextureUsage_RenderAttachment, WGPUTextureFormat_R32Uint);

	WGPURenderPassColorAttachment renderPassColorAttachment = {};
	renderPassColorAttachment.view = m_indexTexture.getTextureView();
	renderPassColorAttachment.resolveTarget = NULL;
	renderPassColorAttachment.loadOp = WGPULoadOp::WGPULoadOp_Clear;
	renderPassColorAttachment.storeOp = WGPUStoreOp::WGPUStoreOp_Store;
	renderPassColorAttachment.clearValue = { 0.0f, 0.0f, 1.0f, 1.0f };
	renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
	renderPassColorAttachments.push_back(renderPassColorAttachment);

	m_uniforms.projection = m_camera.getPerspectiveMatrix();
	m_uniforms.view = m_camera.getViewMatrix();
	m_uniforms.env = m_camera.getRotationMatrix();
	m_uniforms.model = glm::mat4(1.0f);
	m_uniforms.normal = Camera::GetNormalMatrix(m_uniforms.view * m_uniforms.model);
	m_uniforms.color = { 0.0f, 1.0f, 0.4f, 1.0f };
	m_uniforms.camPosition = m_camera.getPosition();
	m_uniforms.lightVP = glm::mat4(1.0f);
	m_uniforms.shadow = glm::mat4(1.0f);
	m_uniforms.lightPosition = glm::vec3(0.0f, 0.0f, 0.0f);

	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), 0, &m_uniforms, sizeof(Uniforms));

	wgpContext.addSahderModule("PICK_COMPUTE", "res/shader/pick_compute.wgsl");
	wgpContext.createComputePipeline("PICK_COMPUTE", "cs_main", "CP_PICK", std::bind(&PrimitivePicking::OnBindGroupLayoutsCompute, this));

	wgpContext.addSahderModule("PICK", "res/shader/pick.wgsl");
	wgpContext.createRenderPipeline("PICK", "RP_PICK", VL_0, std::bind(&PrimitivePicking::OnBindGroupLayoutsPick, this),
		1u,
		WGPUPrimitiveTopology_TriangleList,
		WGPUTextureFormat_Undefined,
		WGPUTextureFormat_Undefined,
		WGPUCompareFunction_Less,
		{ WRITE_DEPTH | DEPTH_STENCIL_STATE | BLEND_STATE | FRAGMENT_STATE, BlendMode::ALPHA_BLENDING, WGPUTextureFormat_R32Uint, WGPUCullMode_None}
	);

	wgpContext.addSahderModule("PICK_DEBUG", "res/shader/pick_debug.wgsl");
	wgpContext.createRenderPipeline("PICK_DEBUG", "RP_PICK_DEBUG", VL_NONE, std::bind(&PrimitivePicking::OnBindGroupLayoutsDebug, this));

	m_computeBindGroup = createComputeBindGroup();
	m_debugBindGroup = createDebugBindGroup();

	for (unsigned int k = 0; k < m_teapot.getMesh()->getIndexBuffer().size() / 3u; ++k) {
		unsigned int index0 = m_teapot.getMesh()->getIndexBuffer()[k * 3];
		unsigned int index1 = m_teapot.getMesh()->getIndexBuffer()[k * 3 + 1];
		unsigned int index2 = m_teapot.getMesh()->getIndexBuffer()[k * 3 + 2];

		m_vertices.push_back({ m_teapot.getMesh()->getVertexBuffer()[index0 * 6 + 0] , m_teapot.getMesh()->getVertexBuffer()[index0 * 6 + 1] , m_teapot.getMesh()->getVertexBuffer()[index0 * 6 + 2],
							m_teapot.getMesh()->getVertexBuffer()[index0 * 6 + 3] , m_teapot.getMesh()->getVertexBuffer()[index0 * 6 + 4] , m_teapot.getMesh()->getVertexBuffer()[index0 * 6 + 5],
							k});

		m_vertices.push_back({ m_teapot.getMesh()->getVertexBuffer()[index1 * 6 + 0] , m_teapot.getMesh()->getVertexBuffer()[index1 * 6 + 1] , m_teapot.getMesh()->getVertexBuffer()[index1 * 6 + 2],
							m_teapot.getMesh()->getVertexBuffer()[index1 * 6 + 3] , m_teapot.getMesh()->getVertexBuffer()[index1 * 6 + 4] , m_teapot.getMesh()->getVertexBuffer()[index1 * 6 + 5],
							k });

		m_vertices.push_back({ m_teapot.getMesh()->getVertexBuffer()[index2 * 6 + 0] , m_teapot.getMesh()->getVertexBuffer()[index2 * 6 + 1] , m_teapot.getMesh()->getVertexBuffer()[index2 * 6 + 2],
							m_teapot.getMesh()->getVertexBuffer()[index2 * 6 + 3] , m_teapot.getMesh()->getVertexBuffer()[index2 * 6 + 4] , m_teapot.getMesh()->getVertexBuffer()[index2 * 6 + 5],
							k });	

		m_indices.push_back(k * 3); m_indices.push_back(k * 3 + 1); m_indices.push_back(k * 3 + 2);
	}
	m_vertexBuffer.createBuffer(reinterpret_cast<const void*>(m_vertices.data()), sizeof(Vertex) * m_vertices.size(), WGPUBufferUsage_Vertex | WGPUBufferUsage_Storage);
	m_indexBuffer.createBuffer(reinterpret_cast<const void*>(m_indices.data()), sizeof(unsigned int) * m_indices.size(), WGPUBufferUsage_Index | WGPUBufferUsage_Storage);

	m_wgpTeapot.create(m_vertexBuffer, m_indexBuffer);

	m_wgpTeapot.setBindGroups("BG", std::bind(&PrimitivePicking::OnBindGroupsPick, this));
	wgpContext.OnDraw = std::bind(&PrimitivePicking::OnDraw, this, std::placeholders::_1, std::placeholders::_2);
}

PrimitivePicking::~PrimitivePicking() {
	m_uniformBuffer.markForDelete();
	m_computeBuffer.markForDelete();
	m_vertexBuffer.markForDelete();
	m_indexBuffer.markForDelete();

	wgpuBindGroupRelease(m_computeBindGroup);
	wgpuBindGroupRelease(m_debugBindGroup);
}

void PrimitivePicking::fixedUpdate() {
	
}

void PrimitivePicking::update() {
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

	glm::vec3 position = m_camera.getPosition();
	RotateY(position, m_dt * 0.2f);
	m_camera.setPosition(position, true);

	m_trackball.idle();

	m_uniforms.projection = m_camera.getPerspectiveMatrix();
	m_uniforms.view = m_camera.getViewMatrix();
	m_uniforms.env = m_camera.getRotationMatrix();
	m_uniforms.model = m_trackball.getTransform();
	m_uniforms.normal = Camera::GetNormalMatrix(m_uniforms.view * m_uniforms.model);
	m_uniforms.color = { 0.0f, 1.0f, 0.4f, 1.0f };
	m_uniforms.camPosition = m_camera.getPosition();
	m_uniforms.lightVP = glm::mat4(1.0f);
	m_uniforms.shadow = glm::mat4(1.0f);
	m_uniforms.lightPosition = glm::vec3(0.0f, 0.0f, 0.0f);

	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), 0u, &m_uniforms, sizeof(Uniforms));

	glm::mat4 matrices[2] = { m_camera.getPerspectiveMatrix() * m_camera.getViewMatrix() , glm::mat4(1.0f) };
	float pickData[2] = { mouse.xPos(),  mouse.yPos() };

	wgpuQueueWriteBuffer(wgpContext.queue, m_computeBuffer.getBuffer(), 0u, matrices, 2 * sizeof(glm::mat4));
	wgpuQueueWriteBuffer(wgpContext.queue, m_computeBuffer.getBuffer(), 2 * sizeof(glm::mat4), pickData, 2 * sizeof(float));
}

void PrimitivePicking::render() {
	wgpDraw();
}

void PrimitivePicking::OnDraw(const WGPUCommandEncoder& commandEncoder, const WGPURenderPassDescriptor& renderPassDescriptor) {
	
	{	
		renderPassColorAttachments.push_back(renderPassDescriptor.colorAttachments[0]);

		WGPURenderPassDescriptor rndrPssDscrptor = renderPassDescriptor;
		rndrPssDscrptor.colorAttachments = renderPassColorAttachments.data();
		rndrPssDscrptor.colorAttachmentCount = renderPassColorAttachments.size();

		WGPURenderPassEncoder renderPassEncoder = wgpuCommandEncoderBeginRenderPass(commandEncoder, &rndrPssDscrptor);
		wgpuRenderPassEncoderSetViewport(renderPassEncoder, 0.0f, 0.0f, static_cast<float>(Application::Width), static_cast<float>(Application::Height), 0.0f, 1.0f);
		
		wgpuRenderPassEncoderSetPipeline(renderPassEncoder, wgpContext.renderPipelines.at("RP_PICK"));
		m_wgpTeapot.draw(renderPassEncoder);		
		
		wgpuRenderPassEncoderEnd(renderPassEncoder);
		wgpuRenderPassEncoderRelease(renderPassEncoder);

		renderPassColorAttachments.pop_back();	
	}

	if(m_debug){
		WGPURenderPassEncoder renderPassEncoder = wgpuCommandEncoderBeginRenderPass(commandEncoder, &renderPassDescriptor);
		wgpuRenderPassEncoderSetViewport(renderPassEncoder, 0.0f, 0.0f, static_cast<float>(Application::Width), static_cast<float>(Application::Height), 0.0f, 1.0f);

		wgpuRenderPassEncoderSetPipeline(renderPassEncoder, wgpContext.renderPipelines.at("RP_PICK_DEBUG"));
		
		wgpuRenderPassEncoderSetBindGroup(renderPassEncoder, 0u, m_debugBindGroup, 0u, NULL);
		wgpuRenderPassEncoderDraw(renderPassEncoder, 6u, 1u, 0, 0);

		wgpuRenderPassEncoderEnd(renderPassEncoder);
		wgpuRenderPassEncoderRelease(renderPassEncoder);
	}

	if(Mouse::instance().isVisibile()){
		WGPUComputePassEncoder computePassEncoder = wgpuCommandEncoderBeginComputePass(commandEncoder, NULL);
		wgpuComputePassEncoderSetPipeline(computePassEncoder, wgpContext.computePipelines.at("CP_PICK"));

		wgpuComputePassEncoderSetBindGroup(computePassEncoder, 0u, m_computeBindGroup, 0u, NULL);
		wgpuComputePassEncoderDispatchWorkgroups(computePassEncoder, 1u, 1u, 1u);
		wgpuComputePassEncoderEnd(computePassEncoder);
		wgpuComputePassEncoderRelease(computePassEncoder);
	}

	if (m_drawUi) {
		WGPURenderPassColorAttachment renderPassColorAttachment = renderPassDescriptor.colorAttachments[0];
		renderPassColorAttachment.loadOp = WGPULoadOp::WGPULoadOp_Load;

		WGPURenderPassDescriptor rndrPssDscrptor = renderPassDescriptor;
		rndrPssDscrptor.colorAttachments = &renderPassColorAttachment;

		WGPURenderPassEncoder renderPassEncoder = wgpuCommandEncoderBeginRenderPass(commandEncoder, &rndrPssDscrptor);
		wgpuRenderPassEncoderSetViewport(renderPassEncoder, 0.0f, 0.0f, static_cast<float>(Application::Width), static_cast<float>(Application::Height), 0.0f, 1.0f);
		renderUi(renderPassEncoder);
		wgpuRenderPassEncoderEnd(renderPassEncoder);
		wgpuRenderPassEncoderRelease(renderPassEncoder);
	}
}

void PrimitivePicking::OnMouseButtonDown(const Event::MouseButtonEvent& event) {
	if (event.button == Event::MouseButtonEvent::BUTTON_LEFT) {
		m_trackball.mouse(TrackBall::Button::ELeftButton, TrackBall::Modifier::ENoModifier, true, event.x, event.y);
		Mouse::instance().detach();	
	}

	if (event.button == Event::MouseButtonEvent::BUTTON_RIGHT) {
		Mouse::instance().attach(Application::Window, true, true, true);
	}

	uint32_t primitveId = 0u;
	wgpuQueueWriteBuffer(wgpContext.queue, m_computeBuffer.getBuffer(), 2 * sizeof(glm::mat4) + 2 * sizeof(float), &primitveId, sizeof(uint32_t));
}

void PrimitivePicking::OnMouseButtonUp(const Event::MouseButtonEvent& event) {
	if (event.button == Event::MouseButtonEvent::BUTTON_LEFT) {
		m_trackball.mouse(TrackBall::Button::ELeftButton, TrackBall::Modifier::ENoModifier, false, event.x, event.y);
		Mouse::instance().attach(Application::Window, false, true);
	} 

	if (event.button == Event::MouseButtonEvent::BUTTON_RIGHT) {
		Mouse::instance().attach(Application::Window, false, false, true);
	} 	
}

void PrimitivePicking::OnMouseMotion(const Event::MouseMoveEvent& event) {
	m_trackball.motion(event.x, event.y);
}

void PrimitivePicking::OnScroll(double xoffset, double yoffset) {

}

void PrimitivePicking::OnKeyDown(const Event::KeyboardEvent& event) {

}

void PrimitivePicking::OnKeyUp(const Event::KeyboardEvent& event) {

}

void PrimitivePicking::resize(int deltaW, int deltaH) {
	m_camera.perspective(glm::radians(72.0f), static_cast<float>(Application::Width) / static_cast<float>(Application::Height), 0.1f, 2000.0f);
	m_camera.orthographic(0.0f, static_cast<float>(Application::Width), 0.0f, static_cast<float>(Application::Height), -1.0f, 1.0f);
	m_indexTexture.resize(Application::Width, Application::Height);
	wgpuBindGroupLayoutRelease(wgpuComputePipelineGetBindGroupLayout(wgpContext.computePipelines.at("CP_PICK"), 0u));
	wgpuBindGroupLayoutRelease(wgpuRenderPipelineGetBindGroupLayout(wgpContext.renderPipelines.at("RP_PICK_DEBUG"), 0u));

	renderPassColorAttachments[0].view = m_indexTexture.getTextureView();

	wgpuBindGroupRelease(m_computeBindGroup);
	wgpuBindGroupRelease(m_debugBindGroup);

	m_computeBindGroup = createComputeBindGroup();
	m_debugBindGroup = createDebugBindGroup();
}

void PrimitivePicking::renderUi(const WGPURenderPassEncoder& renderPassEncoder) {
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
	if (ImGui::Button("Toggle Debug")) {
		m_debug = !m_debug;
	}
	ImGui::End();
	ImGui::Render();
	ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), renderPassEncoder);
}

std::vector<WGPUBindGroupLayout> PrimitivePicking::OnBindGroupLayoutsPick() {
	std::vector<WGPUBindGroupLayout> bindingLayouts(1);

	std::vector<WGPUBindGroupLayoutEntry> bindingLayoutEntries(2);

	bindingLayoutEntries[0].binding = 0u;
	bindingLayoutEntries[0].visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
	bindingLayoutEntries[0].buffer.type = WGPUBufferBindingType::WGPUBufferBindingType_Uniform;
	bindingLayoutEntries[0].buffer.minBindingSize = wgpuBufferGetSize(m_uniformBuffer.getBuffer());

	bindingLayoutEntries[1].binding = 1u;
	bindingLayoutEntries[1].visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
	bindingLayoutEntries[1].buffer.type = WGPUBufferBindingType::WGPUBufferBindingType_Uniform;
	bindingLayoutEntries[1].buffer.minBindingSize = wgpuBufferGetSize(m_computeBuffer.getBuffer());

	WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor = {};
	bindGroupLayoutDescriptor.entryCount = (uint32_t)bindingLayoutEntries.size();
	bindGroupLayoutDescriptor.entries = bindingLayoutEntries.data();

	bindingLayouts[0] = wgpuDeviceCreateBindGroupLayout(wgpContext.device, &bindGroupLayoutDescriptor);
	return bindingLayouts;
}

std::vector<WGPUBindGroupLayout> PrimitivePicking::OnBindGroupLayoutsCompute() {
	std::vector<WGPUBindGroupLayout> bindingLayouts(1);

	std::vector<WGPUBindGroupLayoutEntry> bindingLayoutEntries(2);

	bindingLayoutEntries[0].binding = 0u;
	bindingLayoutEntries[0].visibility = WGPUShaderStage_Compute;
	bindingLayoutEntries[0].buffer.type = WGPUBufferBindingType::WGPUBufferBindingType_Storage;
	bindingLayoutEntries[0].buffer.minBindingSize = wgpuBufferGetSize(m_computeBuffer.getBuffer());

	bindingLayoutEntries[1].binding = 1u;
	bindingLayoutEntries[1].visibility = WGPUShaderStage_Compute;
	bindingLayoutEntries[1].texture.viewDimension = WGPUTextureViewDimension::WGPUTextureViewDimension_2D;
	bindingLayoutEntries[1].texture.sampleType = WGPUTextureSampleType::WGPUTextureSampleType_Uint;

	WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor = {};
	bindGroupLayoutDescriptor.entryCount = (uint32_t)bindingLayoutEntries.size();
	bindGroupLayoutDescriptor.entries = bindingLayoutEntries.data();

	bindingLayouts[0] = wgpuDeviceCreateBindGroupLayout(wgpContext.device, &bindGroupLayoutDescriptor);

	return bindingLayouts;
}

std::vector<WGPUBindGroupLayout> PrimitivePicking::OnBindGroupLayoutsDebug() {
	std::vector<WGPUBindGroupLayout> bindingLayouts(1);

	std::vector<WGPUBindGroupLayoutEntry> bindingLayoutEntries(1);

	bindingLayoutEntries[0].binding = 0u;
	bindingLayoutEntries[0].visibility = WGPUShaderStage_Fragment;
	bindingLayoutEntries[0].texture.viewDimension = WGPUTextureViewDimension_2D;
	bindingLayoutEntries[0].texture.sampleType = WGPUTextureSampleType_Uint;

	WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor = {};
	bindGroupLayoutDescriptor.entryCount = (uint32_t)bindingLayoutEntries.size();
	bindGroupLayoutDescriptor.entries = bindingLayoutEntries.data();

	bindingLayouts[0] = wgpuDeviceCreateBindGroupLayout(wgpContext.device, &bindGroupLayoutDescriptor);
	return bindingLayouts;
}

std::vector<WGPUBindGroup> PrimitivePicking::OnBindGroupsPick() {
	std::vector<WGPUBindGroup> bindGroups(1);

	std::vector<WGPUBindGroupEntry> bindings(2);

	bindings[0].binding = 0u;
	bindings[0].buffer = m_uniformBuffer.getBuffer();
	bindings[0].offset = 0u;
	bindings[0].size = wgpuBufferGetSize(m_uniformBuffer.getBuffer());

	bindings[1].binding = 1u;
	bindings[1].buffer = m_computeBuffer.getBuffer();
	bindings[1].offset = 0u;
	bindings[1].size = wgpuBufferGetSize(m_computeBuffer.getBuffer());

	WGPUBindGroupDescriptor bindGroupDesc = {};
	bindGroupDesc.layout = wgpuRenderPipelineGetBindGroupLayout(wgpContext.renderPipelines.at("RP_PICK"), 0u);
	bindGroupDesc.entryCount = (uint32_t)bindings.size();
	bindGroupDesc.entries = bindings.data();

	bindGroups[0] = wgpuDeviceCreateBindGroup(wgpContext.device, &bindGroupDesc);
	return bindGroups;
}

WGPUBindGroup PrimitivePicking::createComputeBindGroup() {
	std::vector<WGPUBindGroupEntry> entries(2);

	entries[0].binding = 0u;
	entries[0].buffer = m_computeBuffer.getBuffer();
	entries[0].offset = 0u;
	entries[0].size = wgpuBufferGetSize(m_computeBuffer.getBuffer());

	entries[1].binding = 1u;
	entries[1].textureView = m_indexTexture.getTextureView();

	WGPUBindGroupDescriptor bindGroupDesc = {};
	bindGroupDesc.layout = wgpuComputePipelineGetBindGroupLayout(wgpContext.computePipelines.at("CP_PICK"), 0u);
	bindGroupDesc.entryCount = (uint32_t)entries.size();
	bindGroupDesc.entries = (WGPUBindGroupEntry*)entries.data();
	return wgpuDeviceCreateBindGroup(wgpContext.device, &bindGroupDesc);
}

WGPUBindGroup PrimitivePicking::createDebugBindGroup() {
	std::vector<WGPUBindGroupEntry> bindGroupEntries(1);

	bindGroupEntries[0].binding = 0u;
	bindGroupEntries[0].textureView = m_indexTexture.getTextureView();

	WGPUBindGroupDescriptor bindGroupDesc = {};
	bindGroupDesc.layout = wgpuRenderPipelineGetBindGroupLayout(wgpContext.renderPipelines.at("RP_PICK_DEBUG"), 0u);
	bindGroupDesc.entryCount = (uint32_t)bindGroupEntries.size();
	bindGroupDesc.entries = bindGroupEntries.data();

	return wgpuDeviceCreateBindGroup(wgpContext.device, &bindGroupDesc);
}

glm::vec3& PrimitivePicking::RotateY(glm::vec3& p, float rad, const glm::vec3& centerOfRotation) {
	float x = p[0] - centerOfRotation[0];
	float z = p[2] - centerOfRotation[2];

	p[0] = z * sinf(rad) + x * cosf(rad) + centerOfRotation[0];
	p[2] = z * cosf(rad) - x * sinf(rad) + centerOfRotation[2];

	return p;
}