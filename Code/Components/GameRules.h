// Copyright (C) 2021 Space Raccoon Game Studio. All rights reserved. Contacts: <business@space-raccoon.com>
// Created by AfroStalin

#pragma once

#include <CryEntitySystem/IEntityComponent.h>
#include <CryEntitySystem/IEntity.h>
#include <CrySchematyc/ResourceTypes.h>
#include <CrySchematyc/MathTypes.h>

namespace Snake
{
	enum class EGameRulesType
	{
		Invalid,
		Default,
	};

	class CGameRules final : public IEntityComponent
	{
	public:
		CGameRules() = default;
		virtual ~CGameRules() {}
	public:
		
		static void ReflectType(Schematyc::CTypeDesc<CGameRules>& desc)
		{
			desc.SetGUID("{872F80F6-4906-4EB7-80D3-C81D3DDA463E}"_cry_guid);
			desc.SetEditorCategory("Game");
			desc.SetLabel("GameRules");
			desc.SetDescription("GameRules");
			desc.SetComponentFlags({ IEntityComponent::EFlags::Transform, IEntityComponent::EFlags::Socket, IEntityComponent::EFlags::Attach, IEntityComponent::EFlags::NetNotReplicate });

			desc.AddMember(&CGameRules::m_type, 'type', "Type", "Type", "Game rules type", EGameRulesType::Invalid);
		}

		// IEntityComponent
		virtual void   Initialize() override;
		// ~IEntityComponent
	public:
		EGameRulesType GetType() const { return m_type; }
	protected:
		EGameRulesType m_type = EGameRulesType::Invalid;
	};

	static void ReflectType(Schematyc::CTypeDesc<EGameRulesType>& desc)
	{
		desc.SetGUID("{6A466AC6-C974-4CF7-89B0-12DBA7007F98}"_cry_guid);
		desc.SetLabel("GameRules type");
		desc.SetDescription("GameRules type");
		desc.SetDefaultValue(EGameRulesType::Invalid);
		desc.AddConstant(EGameRulesType::Invalid, "Invalid", "Invalid");
		desc.AddConstant(EGameRulesType::Default, "Default", "Default");
	}
}