#pragma once
#include <list>
#include <vector>
#include <memory>
#include <functional>
#include <set>
#include <unordered_set>

#include "../ui/UiContext.h"
#include "../scene/Node.h"
#include "../Object.h"

enum class Layout {
	HORIZONTAL,
	VERTICAL,
	GRID,
	MASONRY
};

class Widget : public Node, public Object2D {

public:

	Widget();
	Widget(const Widget& rhs);
	Widget(Widget&& rhs) noexcept;
	virtual ~Widget();

	
	void input(int mouseX, int mouseY, bool buttonLeft = false);
	void wheelInput(int mouseX, int mouseY, float delta);

	void updateLayout();
	void draw();

	virtual void setScale(float sx, float sy) override;
	void setScale(const glm::vec2& scale) override;
	void setScale(float s) override;

	void setScaleAbsolute(float sx, float sy);
	void setScaleAbsolute(const glm::vec2& scale);
	void setScaleAbsolute(float s);

	void setPosition(float x, float y) override;
	void setPosition(const glm::vec2& position) override;

	void setOrientation(float degrees) override;

	void translate(const glm::vec2& trans) override;
	void translate(float dx, float dy) override;

	void translateRelative(const glm::vec2& trans) override;
	void translateRelative(float dx, float dy) override;

	void scale(const glm::vec2& scale) override;
	void scale(float sx, float sy) override;
	void scale(float s) override;

	void rotate(float degrees) override;

	const glm::mat4& getWorldTransformation() const;
	const glm::vec2& getWorldPosition(bool update = true) const;
	const glm::vec2& getWorldScale(bool update = true) const;
	const float getWorldOrientation(bool update = true) const;
	void updateWorldTransformation() const;
	
	float getWidth();
	float getHeight();

	virtual void setWidth(float width, bool silent = false);
	virtual void setHeight(float height, bool silent = false);
	
	void setSpacing(float spacingX, float spacingY, bool silent = false);
	void setLayout(Layout layout, bool silent = false);
	virtual void setPadding(float paddingX, float paddingY, bool silent = false);
	
	void setBorder(float border);

protected:

	void OnTransformChanged();
	void OnInvalidate();

	virtual bool OnInput(int mouseX, int mouseY, bool buttonLeft = false);
	virtual void OnMouseWheel(int mouseX, int mouseY, float delta);
	virtual void OnLayoutChanged();
	virtual void OnDraw() = 0;
	virtual bool OnMouseOver(int mouseX, int mouseY);
	virtual void OnReset();

	virtual bool inputTree(int mouseX, int mouseY, bool buttonLeft);
	

	void drawTree();			
	void pushWidget(UiPipelineType type, const UiInstance& instance);
	void pushToFront();

	mutable bool m_isLayoutDirty;

	float m_width;
	float m_height;

	float m_paddingX;
	float m_paddingY;
	
	bool m_isMovable;
	float m_border;
	bool m_hasFocus;

	static Widget* ActiveWidget;

private:

	
	void resetTree();
	

	mutable glm::mat4 m_modelMatrix;
	mutable bool m_isDirty;
	
	float m_spacingX;
	float m_spacingY;
	Layout m_layout;
	
	static glm::vec2 WorldPosition;
	static glm::vec2 WorldScale;
	static float WorldOrientation;
	
};

