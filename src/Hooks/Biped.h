#pragma once

#include <xbyak/xbyak.h>

namespace Hooks
{
	class Biped
	{
	public:
		static void Install();

	private:
		// Single signature for all runtimes. On 1.5.97 and VR we replace a
		// call to FindHeadPart, so the return value must be forwarded. On
		// 1.6.x+ we replace a movzx and the return value is ignored.
		static RE::BGSHeadPart* DismemberHeadPartsFix(
			RE::TESNPC* a_npc,
			RE::NiAVObject* a_actor3D,
			std::uint32_t a_wornMask);

		// Set on 1.5.97 and VR only. Left at 0 on 1.6.x+, which is how
		// DismemberHeadPartsFix knows whether to call through.
		inline static REL::Relocation<RE::BGSHeadPart* (*)(RE::TESNPC*, RE::BGSHeadPart::HeadPartType)>
			_FindHeadPart;
	};
}
