#pragma once

#include "Widget.h"

class Surface : public Widget {

public:

	Surface();
	Surface(const Surface& rhs);
	Surface(Surface&& rhs) noexcept;
	virtual ~Surface();

	void setColor(const glm::vec4& color);

private:
	
	bool OnInput(int mouseX, int mouseY, bool buttonLeft) override;
	void OnDraw() override;

	glm::vec4 m_color;
	glm::vec4 m_dragColor;
	glm::vec4 m_gripColor;

	bool m_isDragged;
	bool m_isResizing;
	int m_mouseX, m_mouseY;
	float m_controlSize;
};