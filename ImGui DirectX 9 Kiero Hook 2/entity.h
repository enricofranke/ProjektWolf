#pragma once
#include "includes.h"

class Entity {
public:
	bool GetDormant()
	{
		return *reinterpret_cast<bool*>(this + signatures::m_bDormant);
	}
};