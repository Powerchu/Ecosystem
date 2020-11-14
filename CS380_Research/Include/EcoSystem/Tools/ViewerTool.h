#ifndef _VIEWER_TOOL_H_
#define _VIEWER_TOOL_H_
#include "EcoSystem/Tools/Tools.h"

namespace CS380
{
	class ViewTool : public Tools
	{
	public:
		ViewTool(bool _open = true) noexcept;
		~ViewTool(void) noexcept;

		void Render(void) noexcept;

	private:

		int mnCurrSelection;
	};
}

#endif












