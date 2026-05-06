#pragma once

namespace RE
{
	namespace Offset
	{
		namespace TESNPC
		{
			// Address Library ID 24724 (SE/AE), hardcoded 0x3727B0 (VR)
#ifndef ENABLE_SKYRIM_VR
			inline constexpr auto DismemberHeadParts = REL::ID(24724);
#else
			inline constexpr auto DismemberHeadParts = REL::Offset(0x3727B0);
#endif
		}
	}
}