// Copyright (C) 2021 Space Raccoon Game Studio. All rights reserved. Contacts: <business@space-raccoon.com>
// Created by AfroStalin

#pragma once

#include <CryEntitySystem/IEntityComponent.h>
#include <CryEntitySystem/IEntity.h>

namespace Snake
{
	class CFieldCornerPoint final : public IEntityComponent
	{
	public:
		CFieldCornerPoint() = default;
		virtual ~CFieldCornerPoint() {}

		static void ReflectType(Schematyc::CTypeDesc<CFieldCornerPoint>& desc)
		{
			desc.SetGUID("{4486BD12-351A-4DB7-BBC7-BEAACA863C7D}"_cry_guid);
			desc.SetEditorCategory("Game");
			desc.SetLabel("Field corner point");
			desc.SetDescription("Setup corner points for  field");
			desc.SetComponentFlags({ IEntityComponent::EFlags::Transform, IEntityComponent::EFlags::Socket, IEntityComponent::EFlags::Attach, IEntityComponent::EFlags::NetNotReplicate });
		}

		// IEntityComponent
		virtual void   Initialize() override;
		virtual Cry::Entity::EventFlags GetEventMask() const override;
		virtual void   ProcessEvent(const SEntityEvent& event) override;
		// ~IEntityComponent
	};
}