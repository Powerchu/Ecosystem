#ifndef _TOOLS_H_
#define _TOOLS_H_

#include <string>

namespace Ecosystem
{
	class Tools
	{
	public:
		Tools(std::string _name, bool _opened = true) noexcept;
		virtual ~Tools(void) noexcept;

		const std::string& GetName(void) const noexcept;
		bool* GetOpened(void) noexcept;

		virtual void Render(void) noexcept = 0;
		
	protected:

		bool mbOpened;
		std::string mName;

	};
}

#endif



