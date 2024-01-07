#pragma once

#include <vector>

#include "util/macros.h"

class CInterface {
	DECLARE_SINGLETON(CInterface);
public:
	void _Init(struct GLFWwindow* pWindow);
	bool Initialize();
	void Shutdown();

	void QueueBringToFront() { m_ShouldBringToFront = true; m_ShouldDraw = true; }

	void Draw();
private:
	bool m_ShouldDraw = true;
	bool m_ShouldBringToFront = false;

	std::vector<class CCallback*> m_Callbacks;

	CInterface() { };
};