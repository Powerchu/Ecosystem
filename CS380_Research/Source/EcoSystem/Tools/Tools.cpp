#include "EcoSystem/Tools/Tools.h"

CS380::Tools::Tools(std::string _name, bool _opened) noexcept
	: mName{ _name }, mbOpened{ _opened }
{

}

CS380::Tools::~Tools(void) noexcept
{
}

const std::string& CS380::Tools::GetName(void) const noexcept
{
	return mName;
}

bool* CS380::Tools::GetOpened(void) noexcept
{
	return &mbOpened;
}






