#include "Hooks/Biped.h"

#include "RE/Offset.h"

namespace Hooks
{
	void Biped::Install()
	{
		// All three ENABLE_SKYRIM_* defines are active in this build, so the
		// runtime must be selected at runtime rather than by preprocessor.
		auto& trampoline = SKSE::GetTrampoline();

		if (REL::Module::IsVR()) {
			// VR 1.4.15: hardcoded offsets, 5-byte E8 call, rdi = actor3D.
			// VR has only ever had one build, so offsets are stable.
			static const auto hook = REL::Relocation<std::uintptr_t>(
				REL::Offset(0x3727B0 + 0xCD));

			if (!REL::make_pattern<"E8">().match(hook.address())) {
				logger::info("HairSuppressionFix: hook site already patched, overwriting."sv);
			}

			// Resolve FindHeadPart independently rather than reading the E8
			// bytes, which BeardMaskFix may have already overwritten.
			_FindHeadPart = REL::Relocation<std::uintptr_t>(REL::Offset(0x36FAB0)).address();

			struct Patch : Xbyak::CodeGenerator
			{
				Patch(std::uintptr_t a_hookAddr)
				{
					mov(r8d, r12d);
					mov(rdx, rdi);
					mov(rax, reinterpret_cast<std::uintptr_t>(&DismemberHeadPartsFix));
					call(rax);
					jmp(ptr[rip]);
					dq(a_hookAddr + 0x5);
				}
			};

			Patch patch{ hook.address() };
			auto* patchBuf = trampoline.allocate(patch.getSize());
			std::memcpy(patchBuf, patch.getCode(), patch.getSize());
			trampoline.write_branch<5>(
				hook.address(),
				reinterpret_cast<std::uintptr_t>(patchBuf));

			logger::info("HairSuppressionFix: VR hook installed."sv);
		}
		else if (REL::Module::get().version() < SKSE::RUNTIME_SSE_1_6_317) {
			// SE 1.5.97: ID 24220, offset 0xCD, 5-byte E8 call, rdi = actor3D
			static const auto hook = REL::Relocation<std::uintptr_t>(REL::ID(24220), 0xCD);

			if (!REL::make_pattern<"E8">().match(hook.address())) {
				logger::info("HairSuppressionFix: hook site already patched, overwriting."sv);
			}

			// Resolve FindHeadPart via Address Library rather than reading the
			// E8 bytes, which BeardMaskFix may have already overwritten.
			_FindHeadPart = REL::Relocation<std::uintptr_t>(REL::ID(24202)).address();

			struct Patch : Xbyak::CodeGenerator
			{
				Patch(std::uintptr_t a_hookAddr)
				{
					mov(r8d, r12d);
					mov(rdx, rdi);
					mov(rax, reinterpret_cast<std::uintptr_t>(&DismemberHeadPartsFix));
					call(rax);
					jmp(ptr[rip]);
					dq(a_hookAddr + 0x5);
				}
			};

			Patch patch{ hook.address() };
			auto* patchBuf = trampoline.allocate(patch.getSize());
			std::memcpy(patchBuf, patch.getCode(), patch.getSize());
			trampoline.write_branch<5>(
				hook.address(),
				reinterpret_cast<std::uintptr_t>(patchBuf));

			logger::info("HairSuppressionFix: 1.5.97 hook installed."sv);
		}
		else {
			// AE 1.6.x+: ID 24724, offset 0xD2, 8-byte movzx, rsi = actor3D.
			// _FindHeadPart intentionally left unset — the original call runs
			// before our hook site here, so there is nothing to forward.
			static const auto hook = REL::Relocation<std::uintptr_t>(REL::ID(24724), 0xD2);

			if (!REL::make_pattern<"44 0F B6 8D 40 02 00 00">().match(hook.address())) {
				logger::info("HairSuppressionFix: hook site already patched, overwriting."sv);
			}

			struct Patch : Xbyak::CodeGenerator
			{
				Patch(std::uintptr_t a_hookAddr)
				{
					mov(r8d, r12d);
					mov(rdx, rsi);
					mov(rcx, rbp);
					mov(rax, reinterpret_cast<std::uintptr_t>(&DismemberHeadPartsFix));
					call(rax);
					movzx(r9d, byte[rbp + offsetof(RE::TESNPC, numHeadParts)]);
					jmp(ptr[rip]);
					dq(a_hookAddr + 0x8);
				}
			};

			Patch patch{ hook.address() };
			auto* patchBuf = trampoline.allocate(patch.getSize());
			std::memcpy(patchBuf, patch.getCode(), patch.getSize());
			trampoline.write_branch<6>(
				hook.address(),
				reinterpret_cast<std::uintptr_t>(patchBuf));

			logger::info("HairSuppressionFix: 1.6.x+ hook installed."sv);
		}
	}

	RE::BGSHeadPart* Biped::DismemberHeadPartsFix(
		RE::TESNPC* a_npc,
		RE::NiAVObject* a_actor3D,
		std::uint32_t a_wornMask)
	{
		// On 1.5.97 and VR we replaced a call to FindHeadPart — invoke the
		// original so vanilla suppression still works, and forward its result.
		// On 1.6.x+ _FindHeadPart is never assigned, so this is skipped.
		RE::BGSHeadPart* result = nullptr;
		if (_FindHeadPart.address() != 0) {
			result = _FindHeadPart(a_npc, RE::BGSHeadPart::HeadPartType::kHair);
		}

		static constexpr std::uint32_t hairSlot     = 1U << (31 - 30);
		static constexpr std::uint32_t longHairSlot = 1U << (41 - 30);
		// [BEARD FIX] Remove the line below to compile without beard suppression
		static constexpr std::uint32_t beardSlot    = 1U << (44 - 30);

		const bool slot31 = (a_wornMask & hairSlot) != 0;
		const bool slot41 = (a_wornMask & longHairSlot) != 0;
		// [BEARD FIX] Remove the line below to compile without beard suppression
		const bool hideBeard = (a_wornMask & beardSlot) != 0;

		RE::BGSHeadPart* hairPart = nullptr;
		// [BEARD FIX] Remove the line below to compile without beard suppression
		RE::BGSHeadPart* beardPart = nullptr;

		if (!a_npc->headParts || a_npc->numHeadParts == 0 || !a_actor3D) {
			return result;
		}

		for (const auto& headPart : std::span(a_npc->headParts, a_npc->numHeadParts)) {
			if (!headPart) {
				continue;
			}

			switch (headPart->type.get()) {
			case RE::BGSHeadPart::HeadPartType::kHair:
				hairPart = headPart;
				break;
			// [BEARD FIX] Remove the case below to compile without beard suppression
			case RE::BGSHeadPart::HeadPartType::kFacialHair:
				beardPart = headPart;
				break;
			default:
				break;
			}

			// [BEARD FIX] To compile without beard suppression, change to: if (hairPart) break;
			if (hairPart && beardPart) {
				break;
			}
		}

		auto setHidden = [&](RE::BGSHeadPart* a_headPart, bool a_hide) {
			if (!a_headPart) {
				return;
			}

			if (!a_headPart->formEditorID.empty()) {
				if (auto node3D = a_actor3D->GetObjectByName(a_headPart->formEditorID)) {
					if (a_hide) {
						node3D->GetFlags().set(RE::NiAVObject::Flag::kHidden);
					}
					else {
						node3D->GetFlags().reset(RE::NiAVObject::Flag::kHidden);
					}
				}
			}

			for (const auto& extraPart : a_headPart->extraParts) {
				if (!extraPart) {
					continue;
				}

				if (extraPart->formEditorID.empty()) {
					continue;
				}

				if (auto extra3D = a_actor3D->GetObjectByName(extraPart->formEditorID)) {
					if (a_hide) {
						extra3D->GetFlags().set(RE::NiAVObject::Flag::kHidden);
					}
					else {
						extra3D->GetFlags().reset(RE::NiAVObject::Flag::kHidden);
					}
				}
			}
		};

		// Hair: only act when both slots are occupied (wig — vanilla misses
		// the second ARMA, so we hide everything) or when neither is occupied
		// (unequipped — restore visibility). When only one slot is occupied,
		// leave it alone: vanilla handles slot 31 correctly on its own, and
		// touching it here risks undoing that.
		if (slot31 && slot41) {
			setHidden(hairPart, true);
		}
		else if (!slot31 && !slot41) {
			setHidden(hairPart, false);
		}

		// [BEARD FIX] Remove the line below to compile without beard suppression
		setHidden(beardPart, hideBeard);

		return result;
	}
}
