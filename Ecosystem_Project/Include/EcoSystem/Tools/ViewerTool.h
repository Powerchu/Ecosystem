#ifndef _VIEWER_TOOL_H_
#define _VIEWER_TOOL_H_
#include "EcoSystem/Tools/Tools.h"

namespace Ecosystem
{
	class ViewTool : public Tools
	{
	public:
		ViewTool(bool _open = true) noexcept;
		~ViewTool(void) noexcept;

		void Render(void) noexcept;

	private:

		[[maybe_unused]] int mnCurrSelection;
	};
}

#endif












