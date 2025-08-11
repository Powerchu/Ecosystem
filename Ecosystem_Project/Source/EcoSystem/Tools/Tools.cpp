#include "EcoSystem/Tools/Tools.h"

Ecosystem::Tools::Tools(std::string _name, bool _opened) noexcept
	: mbOpened{ _opened }, mName{ _name }
{

}

Ecosystem::Tools::~Tools(void) noexcept
{
}

const std::string& Ecosystem::Tools::GetName(void) const noexcept
{
	return mName;
}

bool* Ecosystem::Tools::GetOpened(void) noexcept
{
	return &mbOpened;
}






