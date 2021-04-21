// Copyright (C) 2021 Space Raccoon Game Studio. All rights reserved. Contacts: <business@space-raccoon.com>
// Created by AfroStalin

#pragma once

#include <CryEntitySystem/IEntityComponent.h>
#include <CryEntitySystem/IEntity.h>

namespace Snake
{
	class CSnakePart final : public IEntityComponent
	{
	public:
		CSnakePart() = default;
		virtual ~CSnakePart() {}

		static void ReflectType(Schematyc::CTypeDesc<CSnakePart>& desc)
		{
			desc.SetGUID("{F4AA4835-0D04-467D-8BB1-F47BC2FB0EAB}"_cry_guid);
		}

		// IEntityComponent
		virtual void   Initialize() override;
		// ~IEntityComponent
	};
}