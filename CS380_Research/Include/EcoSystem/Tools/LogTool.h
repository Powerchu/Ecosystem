#ifndef _LOG_TOOL_H_
#define _LOG_TOOL_H_
#include "EcoSystem/Tools/Tools.h"

namespace CS380
{
	class LogTool :public Tools
	{
	public:
		LogTool(bool _opened = true) noexcept;
		~LogTool(void) noexcept;

		void Render(void) noexcept;

	private:

	};

}


#endif






