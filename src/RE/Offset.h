#pragma once

// Reference values for TESNPC::DismemberHeadParts and FindHeadPart.
//
// In a single-DLL build all runtimes must be representable at once, so these
// cannot be selected with #ifdef. Biped.cpp resolves the correct one at
// runtime via REL::Module::IsVR() / version checks, so these are kept here
// purely as documentation of where the numbers came from.
//
//   DismemberHeadParts   SE 1.5.97: Address Library ID 24220
//                        AE 1.6.x+: Address Library ID 24724
//                        VR 1.4.15: module offset 0x3727B0
//
//   FindHeadPart         SE 1.5.97: Address Library ID 24202
//                        AE 1.6.x+: not needed (hook site is a movzx, not a call)
//                        VR 1.4.15: module offset 0x36FAB0

namespace RE
{
	namespace Offset
	{
		namespace TESNPC
		{
			inline constexpr std::uint64_t DismemberHeadParts_SE_ID = 24220;
			inline constexpr std::uint64_t DismemberHeadParts_AE_ID = 24724;
			inline constexpr std::uintptr_t DismemberHeadParts_VR_Offset = 0x3727B0;

			inline constexpr std::uint64_t FindHeadPart_SE_ID = 24202;
			inline constexpr std::uintptr_t FindHeadPart_VR_Offset = 0x36FAB0;
		}
	}
}
