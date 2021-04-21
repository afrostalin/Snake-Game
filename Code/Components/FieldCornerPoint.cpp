// Copyright (C) 2021 Space Raccoon Game Studio. All rights reserved. Contacts: <business@space-raccoon.com>
// Created by AfroStalin

#include "StdAfx.h"
#include "FieldCornerPoint.h"

namespace Snake
{
	namespace
	{
		static void RegisterCornerPointComponent(Schematyc::IEnvRegistrar& registrar)
		{
			Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
			{
				scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(CFieldCornerPoint));
			}
		}

		CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterCornerPointComponent);
	}


	void CFieldCornerPoint::Initialize()
	{
		if (gEnv->IsEditor())
		{
			m_pEntity->LoadGeometry(GetOrMakeEntitySlotId(), "%ENGINE%/EngineAssets/Objects/primitive_pyramid.cgf");
		}
	}

	Cry::Entity::EventFlags CFieldCornerPoint::GetEventMask() const
	{
		if (gEnv->IsEditor())
		{
			return EEntityEvent::Reset;
		}
		else
		{
			return Cry::Entity::EventFlags();
		}
	}

	void CFieldCornerPoint::ProcessEvent(const SEntityEvent& event)
	{
		switch (event.event)
		{
		case ENTITY_EVENT_RESET:
		{
			//! nParam[0] is 1 if entering game mode, 0 if exiting
			int mode = (int)event.nParam[0];

			if (mode == 1)
			{
				m_pEntity->Hide(true);
			}
			else if (mode == 0)
			{
				m_pEntity->Hide(false);
			}

			break;
		}
		default:
			break;
		}
	}
}