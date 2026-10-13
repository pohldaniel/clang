#include <glm/gtc/type_ptr.hpp>
#include "Canvas.h"

Canvas::Canvas() : Widget(), m_color(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)), m_layer(0u){
	
}

Canvas::Canvas(const Canvas& rhs) :
	Widget(rhs),
	m_color(rhs.m_color),
    m_layer(rhs.m_layer){
}

Canvas::Canvas(Canvas&& rhs) noexcept :
	Widget(rhs),
	m_color(rhs.m_color),
	m_layer(rhs.m_layer) {
}

Canvas::~Canvas() {

}

void Canvas::setColor(const glm::vec4& color) {
	m_color = color;
}

void Canvas::setLayer(uint32_t layer) {
	m_layer = layer;
}

void Canvas::OnDraw() {

	UiInstance uiInstance = {};

	std::memcpy(uiInstance.transform, glm::value_ptr(getWorldTransformation() * glm::scale(glm::vec3(m_width, m_height, 1.0f))), sizeof(glm::mat4));
	std::memcpy(uiInstance.color, glm::value_ptr(m_color), sizeof(glm::vec4));

	uiInstance.textureRect[0] = 0.0f;
	uiInstance.textureRect[1] = 0.0f;
	uiInstance.textureRect[2] = 1.0f;
	uiInstance.textureRect[3] = 1.0f;
	uiInstance.textureLayer = m_layer;

	pushWidget(UiPipelineType::Standard, uiInstance);
}

void Canvas::OnLayoutChanged() {
	if (!m_isLayoutDirty)
		return;
}

bool Canvas::OnInput(int mouseX, int mouseY, bool buttonLeft) {
	return true;
}