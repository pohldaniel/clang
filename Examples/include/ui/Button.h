#pragma once
#include <functional>
#include "Widget.h"

class Button : public Widget {

public:

	Button();
	Button(const Button& rhs);
	Button(Button&& rhs) noexcept;
	virtual ~Button();

	void setColor(const glm::vec4& color);
	void setOutlineColor(const glm::vec4& color);
	void setOutlineColorHover(const glm::vec4& color);
	void setOutlineThickness(float thickness);
	void setOnClick(std::function<void()> fun);

private:

	bool OnInput(int mouseX, int mouseY, bool buttonLeft) override;
	void OnDraw() override;
	void OnReset() override;
	bool OnMouseOver(int mouseX, int mouseY) override;

	glm::vec4 m_color;
	glm::vec4 m_outlineColor;
	glm::vec4 m_outlineColorHover;

	float m_thickness;
	bool m_isPressed;
	bool m_wasPressed;
	std::function<void()> m_onClick;
};