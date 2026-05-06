#include "Hooks/Biped.h"

#include "RE/Offset.h"

namespace Hooks
{
	void Biped::Install()
	{
		// Hook into TESNPC::DismemberHeadParts to fix hair/beard suppression.
		// Same hook site as BeardMaskFix (ID 24724). If BeardMaskFix patched
		// first, we overwrite its hook — this plugin is a strict superset.

#ifndef ENABLE_SKYRIM_VR
		// SE/AE: hook at offset 0xD2 (8-byte movzx instruction)
		static const auto hook = REL::Relocation<std::uintptr_t>(
			REL::ID(24724), 0xD2);

		if (!REL::make_pattern<"44 0F B6 8D 40 02 00 00">().match(hook.address())) {
			logger::info("HairSuppressionFix: hook site already patched, overwriting."sv);
		}

		struct Patch : Xbyak::CodeGenerator
		{
			Patch()
			{
				mov(r8d, r12d);   // arg3: wornMask
				mov(rdx, rsi);    // arg2: actor3D
				mov(rcx, rbp);    // arg1: npc
				mov(rax, reinterpret_cast<std::uintptr_t>(&DismemberHeadPartsFix));
				call(rax);

				// Re-execute overwritten instruction
				movzx(r9d, byte[rbp + offsetof(RE::TESNPC, numHeadParts)]);

				jmp(ptr[rip]);
				dq(hook.address() + 0x8);
			}
		};

		Patch patch{};

		auto& trampoline = SKSE::GetTrampoline();
		auto* patchBuf = trampoline.allocate(patch.getSize());
		std::memcpy(patchBuf, patch.getCode(), patch.getSize());
		trampoline.write_branch<6>(
			hook.address(),
			reinterpret_cast<std::uintptr_t>(patchBuf));

		logger::info("HairSuppressionFix: SE/AE hook installed."sv);
#else
		// VR: hook at offset 0xCD (5-byte call instruction)
		static const auto hook = REL::Relocation<std::uintptr_t>(
			REL::Offset(0x3727B0 + 0xCD));

		if (!REL::make_pattern<"E8">().match(hook.address())) {
			logger::info("HairSuppressionFix: VR hook site already patched, overwriting."sv);
		}

		struct Patch : Xbyak::CodeGenerator
		{
			Patch()
			{
				mov(r8d, r12d);   // arg3: wornMask
				mov(rdx, rdi);    // arg2: actor3D (rdi on VR)
				mov(rax, reinterpret_cast<std::uintptr_t>(&DismemberHeadPartsFix));
				call(rax);

				jmp(ptr[rip]);
				dq(hook.address() + 0x5);
			}
		};

		Patch patch{};

		auto& trampoline = SKSE::GetTrampoline();
		auto* patchBuf = trampoline.allocate(patch.getSize());
		std::memcpy(patchBuf, patch.getCode(), patch.getSize());
		_FindHeadPart = trampoline.write_branch<5>(
			hook.address(),
			reinterpret_cast<std::uintptr_t>(patchBuf));

		logger::info("HairSuppressionFix: VR hook installed."sv);
#endif
	}

#ifndef ENABLE_SKYRIM_VR
	void Biped::DismemberHeadPartsFix(
		RE::TESNPC* a_npc,
		RE::NiAVObject* a_actor3D,
		std::uint32_t a_wornMask)
	{
#else
	RE::BGSHeadPart* Biped::DismemberHeadPartsFix(
		RE::TESNPC* a_npc,
		RE::NiAVObject* a_actor3D,
		std::uint32_t a_wornMask)
	{
		auto result = _FindHeadPart(a_npc, RE::BGSHeadPart::HeadPartType::kHair);
#endif

		// Slot checks: bit N = slot (30 + N)
		static constexpr std::uint32_t hairSlot     = 1U << (31 - 30);  // bit 1
		static constexpr std::uint32_t longHairSlot = 1U << (41 - 30);  // bit 11
		static constexpr std::uint32_t beardSlot    = 1U << (44 - 30);  // bit 14

		// Hide all hair (main + extras) only when BOTH 31 and 41 are occupied.
		// Slot 31 alone (e.g. helmets) is handled by vanilla — we don't touch it.
		const bool hideAllHair = ((a_wornMask & hairSlot) != 0) && ((a_wornMask & longHairSlot) != 0);
		const bool hideBeard   = (a_wornMask & beardSlot) != 0;

		// Find kHair and kFacialHair head parts on this NPC
		RE::BGSHeadPart* hairPart  = nullptr;
		RE::BGSHeadPart* beardPart = nullptr;

		if (!a_npc->headParts || a_npc->numHeadParts == 0) {
#ifndef ENABLE_SKYRIM_VR
			return;
#else
			return result;
#endif
		}

		for (const auto& headPart : std::span(a_npc->headParts, a_npc->numHeadParts)) {
			if (!headPart) {
				continue;
			}

			switch (headPart->type.get()) {
			case RE::BGSHeadPart::HeadPartType::kHair:
				hairPart = headPart;
				break;
			case RE::BGSHeadPart::HeadPartType::kFacialHair:
				beardPart = headPart;
				break;
			default:
				break;
			}

			if (hairPart && beardPart) {
				break;
			}
		}

		// Set or clear kHidden on a head part's 3D node and its extra parts
		auto setHidden = [&](RE::BGSHeadPart* a_headPart, bool a_hide) {
			if (!a_headPart) {
				return;
			}

			if (auto node3D = a_actor3D->GetObjectByName(a_headPart->formEditorID)) {
				if (a_hide) {
					node3D->flags.set(RE::NiAVObject::Flag::kHidden);
				}
				else {
					node3D->flags.reset(RE::NiAVObject::Flag::kHidden);
				}
			}

			for (const auto& extraPart : a_headPart->extraParts) {
				if (!extraPart) {
					continue;
				}

				if (auto extra3D = a_actor3D->GetObjectByName(extraPart->formEditorID)) {
					if (a_hide) {
						extra3D->flags.set(RE::NiAVObject::Flag::kHidden);
					}
					else {
						extra3D->flags.reset(RE::NiAVObject::Flag::kHidden);
					}
				}
			}
		};

		setHidden(hairPart, hideAllHair);
		setHidden(beardPart, hideBeard);

#ifdef ENABLE_SKYRIM_VR
		return result;
#endif
	}
}