#include <glm/gtc/type_ptr.hpp> 
#include "Button.h"

Button::Button() : Widget(),
m_color(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)), 
m_outlineColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)), 
m_outlineColorHover(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)),
m_thickness(0.0f),
m_isPressed(false),
m_wasPressed(false),
m_onClick(nullptr){
	
}

Button::Button(const Button& rhs) :
	Widget(rhs),
	m_color(rhs.m_color),
	m_outlineColorHover(rhs.m_outlineColorHover),
	m_thickness(rhs.m_thickness),
	m_isPressed(rhs.m_isPressed),
	m_wasPressed(rhs.m_wasPressed),
	m_onClick(rhs.m_onClick){
}

Button::Button(Button&& rhs) noexcept :
	Widget(rhs),
	m_color(rhs.m_color),
	m_outlineColorHover(rhs.m_outlineColorHover),
	m_thickness(rhs.m_thickness),
	m_isPressed(rhs.m_isPressed),
	m_wasPressed(rhs.m_wasPressed),
	m_onClick(std::move(rhs.m_onClick)) {
}

Button::~Button() {

}

void Button::setColor(const glm::vec4& color) {
	m_color = color;
}

void Button::setOutlineColor(const glm::vec4& color) {
	m_outlineColor = color;
}

void Button::setOutlineColorHover(const glm::vec4& color) {
	m_outlineColorHover = color;
}

void Button::setOutlineThickness(float thickness) {
	m_thickness = thickness;
}

void Button::OnDraw() {
	UiInstance buttonInst = {};
	std::memcpy(buttonInst.transform, glm::value_ptr(getWorldTransformation() * glm::scale(glm::vec3(m_width, m_height, 1.0f))), sizeof(glm::mat4));
	std::memcpy(buttonInst.color, glm::value_ptr(m_color), sizeof(glm::vec4));

	buttonInst.textureRect[0] = 0.0f;
	buttonInst.textureRect[1] = 0.0f;
	buttonInst.textureRect[2] = 1.0f;
	buttonInst.textureRect[3] = 1.0f;
	buttonInst.textureLayer = 0.0f;

	buttonInst.flipAndTile[0] = 0.0f;
	buttonInst.flipAndTile[1] = 0.0f;
	buttonInst.flipAndTile[2] = 0.0f;

	pushWidget(UiPipelineType::MaskWrite, buttonInst);

	
	float outlineWidth = m_width + (m_thickness * 2.0f);
	float outlineHeight = m_height + (m_thickness * 2.0f);

	glm::mat4 offsetMatrix = glm::translate(glm::vec3(-m_thickness, -m_thickness, 0.0f));
	glm::mat4 transformOutline = getWorldTransformation() * offsetMatrix * glm::scale(glm::vec3(outlineWidth, outlineHeight, 1.0f));

	UiInstance outlineInst = {};
	std::memcpy(outlineInst.transform, glm::value_ptr(transformOutline), sizeof(glm::mat4));
	std::memcpy(outlineInst.color, glm::value_ptr(m_outlineColor), sizeof(glm::vec4));

	outlineInst.textureRect[0] = 0.0f;
	outlineInst.textureRect[1] = 0.0f;
	outlineInst.textureRect[2] = 1.0f;
	outlineInst.textureRect[3] = 1.0f;
	outlineInst.textureLayer = 0.0f;

	outlineInst.flipAndTile[0] = 0.0f;
	outlineInst.flipAndTile[1] = 0.0f;
	outlineInst.flipAndTile[2] = 0.0f;

	pushWidget(UiPipelineType::OutlineRead, outlineInst);
}

bool Button::OnInput(int mouseX, int mouseY, bool buttonLeft) {
	bool isHovered = OnMouseOver(mouseX, mouseY);

	if (isHovered) {
		m_outlineColor = m_outlineColorHover;
	}else {
		m_outlineColor = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
	}

	if (buttonLeft) {
		if (isHovered && (!m_wasPressed || m_isPressed)) {
			m_isPressed = true;
		}
	}else {
		if (m_isPressed && isHovered && m_onClick) {
			m_onClick();
		}

		m_isPressed = false;
	}

	m_wasPressed = buttonLeft;

	return !m_isPressed;
}

void Button::setOnClick(std::function<void()> fun) {
	m_onClick = fun;
}

void Button::OnReset() {
	Widget::OnReset();
	m_outlineColor = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
}

bool Button::OnMouseOver(int mouseX, int mouseY) {
	glm::vec2 scale = getWorldScale();
	float visualWidth = m_width * scale[0];
	float visualHeight = m_height * scale[1];
	float visualThickness = m_thickness * scale[0];

	glm::vec2 position = getWorldPosition();

	return (mouseX > position[0] - visualThickness && mouseX < position[0] + visualWidth + visualThickness &&
		mouseY > position[1] - visualThickness && mouseY < position[1] + visualHeight + visualThickness);
}
