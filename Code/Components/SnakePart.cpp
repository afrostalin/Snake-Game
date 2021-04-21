// Copyright (C) 2021 Space Raccoon Game Studio. All rights reserved. Contacts: <business@space-raccoon.com>
// Created by AfroStalin

#include "StdAfx.h"
#include "SnakePart.h"

namespace Snake
{
	namespace
	{
		static void RegisterSnakePartComponent(Schematyc::IEnvRegistrar& registrar)
		{
			Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
			{
				scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(CSnakePart));
			}
		}

		CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterSnakePartComponent);
	}

	void CSnakePart::Initialize()
	{
		GetEntity()->LoadGeometry(GetOrMakeEntitySlotId(), "objects/characters/snake/part.cgf");
		GetEntity()->SetFlags(GetEntity()->GetFlags() | ENTITY_FLAG_CASTSHADOW);
	}
}