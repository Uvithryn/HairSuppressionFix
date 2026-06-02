#pragma once

#include <xbyak/xbyak.h>

namespace Hooks
{
	class Biped
	{
	public:
		static void Install();

	private:
		static RE::BGSHeadPart* DismemberHeadPartsFix(
			RE::TESNPC* a_npc,
			RE::NiAVObject* a_actor3D,
			std::uint32_t a_wornMask);

		inline static REL::Relocation<RE::BGSHeadPart* (*)(RE::TESNPC*, RE::BGSHeadPart::HeadPartType)>
			_FindHeadPart;
	};
}