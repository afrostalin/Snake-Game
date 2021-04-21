// Copyright (C) 2021 Space Raccoon Game Studio. All rights reserved. Contacts: <business@space-raccoon.com>
// Created by AfroStalin

#include "StdAfx.h"
#include "GameRules.h"

namespace Snake
{
	namespace
	{
		static void RegisterGameRulesComponent(Schematyc::IEnvRegistrar& registrar)
		{
			Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
			{
				scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(CGameRules));
			}
		}

		CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterGameRulesComponent);
	}

	void CGameRules::Initialize()
	{
	}
}