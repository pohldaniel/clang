#include <glm/gtc/type_ptr.hpp> 

#include "Label.h"

Label::Label(const CharacterSet& characterSet) : Widget(), characterSet(characterSet), m_color(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)) {

}

Label::Label(const Label& rhs) :
	Widget(rhs),
	characterSet(rhs.characterSet),
	m_text(rhs.m_text),
	m_color(rhs.m_color) {

}

Label::Label(Label&& rhs) noexcept :
	Widget(rhs),
	characterSet(rhs.characterSet),
	m_text(std::move(rhs.m_text)),
	m_color(std::move(rhs.m_color)) {

}

Label::~Label() {

}

void Label::setColor(const glm::vec4& textColor) {
	m_color = textColor;
}

void Label::setText(const std::string& text) {
	m_text = text;
	m_width = characterSet.getWidth(m_text);
	m_height = characterSet.lineHeight;
	OnInvalidate();
}

void Label::OnLayoutChanged() {
	if (!m_isLayoutDirty)
		return;
	m_width = characterSet.getWidth(m_text);
	m_height = characterSet.lineHeight;

	m_isLayoutDirty = false;
}

void Label::OnDraw() {
	glm::vec2 currentCursor = { m_paddingX, 0.0f };
	for (char c : m_text) {
		
		const Char& ch = characterSet.getCharacter(c);
		float dx = ch.pos[0];
		float dy = (m_paddingY + ch.pos[1]);
		float gw = ch.size[0];
		float gh = ch.size[1];

		UiInstance uiInstance = {};
		glm::mat4 glyphTransform = glm::translate(glm::vec3(currentCursor[0] + dx, -dy, 0.0f)) * glm::scale(glm::vec3(gw, gh, 1.0f));

		std::memcpy(uiInstance.transform, glm::value_ptr(getWorldTransformation() * glyphTransform), sizeof(glm::mat4));
		std::memcpy(uiInstance.color, glm::value_ptr(m_color), sizeof(glm::vec4));

		uiInstance.textureRect[0] = ch.textureOffset[0];
		uiInstance.textureRect[1] = ch.textureOffset[1];
		uiInstance.textureRect[2] = ch.textureSize[0];
		uiInstance.textureRect[3] = ch.textureSize[1];
		uiInstance.textureLayer = static_cast<float>(characterSet.layer);
		pushWidget(UiPipelineType::Text, uiInstance);

		currentCursor[0] += ch.advance;
	}
}

void Label::setPadding(float paddingX, float paddingY, bool silent) {
	Widget::setPadding(paddingX, paddingY, silent);
	m_width = characterSet.getWidth(m_text);
	m_height = characterSet.lineHeight;
}