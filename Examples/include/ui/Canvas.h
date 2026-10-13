#pragma once

#include "Widget.h"

class Canvas : public Widget {

public:

	Canvas();
	Canvas(const Canvas& rhs);
	Canvas(Canvas&& rhs) noexcept;
	virtual ~Canvas();

	void setColor(const glm::vec4& color);
	void setLayer(uint32_t layer);

private:

	bool OnInput(int mouseX, int mouseY, bool buttonLeft) override;
	void OnDraw() override;
	void OnLayoutChanged() override;

	glm::vec4 m_color;
	uint32_t m_layer;
};