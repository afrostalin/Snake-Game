// Copyright (C) 2021 Space Raccoon Game Studio. All rights reserved. Contacts: <business@space-raccoon.com>
// Created by AfroStalin

#include "StdAfx.h"
#include "Fruit.h"
#include "Snake.h"
#include <CryPhysics/IPhysics.h>

namespace Snake
{
	namespace
	{
		static void RegisterFruitComponent(Schematyc::IEnvRegistrar& registrar)
		{
			Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
			{
				scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(CFruit));
			}
		}

		CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterFruitComponent);
	}

	void CFruit::Initialize()
	{
		GetEntity()->LoadGeometry(GetOrMakeEntitySlotId(), "objects/other/apple.cgf");
		GetEntity()->SetFlags(GetEntity()->GetFlags() | ENTITY_FLAG_CASTSHADOW);
	}

	Cry::Entity::EventFlags CFruit::GetEventMask() const
	{
		return Cry::Entity::EEvent::Update;
	}

	void CFruit::ProcessEvent(const SEntityEvent& event)
	{
		switch (event.event)
		{
		case Cry::Entity::EEvent::Update:
		{
			const float frameTime = event.fParam[0];

#if REPLAY_SYSTEM
			if (g_pGame->pReplaySystem->IsPaused())
				return;
#endif

			// Pseudo animation
			{
				const float speed = 0.5f;
				const float valueZ = speed * frameTime;
				const float max_z = 1.f;

				if (current_z >= max_z)
				{
					up = !up;
					current_z = 0.f;
				}

				current_z += valueZ;

				if (up)
				{
					GetEntity()->SetPos(GetEntity()->GetWorldPos() + Vec3(0.f, 0.f, valueZ));
				}
				else
				{
					GetEntity()->SetPos(GetEntity()->GetWorldPos() - Vec3(0.f, 0.f, valueZ));
				}

				Quat rotation = GetEntity()->GetRotation();
				rotation.SetRotationAA(DEG2RAD(rotationDegrees), Vec3(0.f, 0.f, 1.f));
				rotationDegrees--;

				if (rotationDegrees <= (-360))
					rotationDegrees = 0;

				GetEntity()->SetRotation(rotation);
			}

			// Self destruction

#if REPLAY_SYSTEM
			if (g_pGame->pReplaySystem->IsPlaying())
				return;
#endif
			{
				if (m_destructionTime != 0.f && m_timer >= m_destructionTime)
				{
					gEnv->pEntitySystem->RemoveEntity(GetEntityId());
				}
				else
				{
					m_timer += frameTime;
				}
			}

			break;
		}
		default:
			break;
		}
	}

	void CFruit::OnShutDown()
	{
		if (CField* pField = GetField())
		{
			pField->OnFruitRemoved(GetEntityId());
		}
	}

	void CFruit::OnCollideWithSnake(CSnake* pSnake)
	{
		if (!IsSpecialFruit())
			return;

#if REPLAY_SYSTEM
		if (g_pGame->pReplaySystem->IsPlaying())
			return;
#endif 
	}

	SFruitState CFruit::GetState()
	{
		SFruitState state;
		state.m_currentPos = GetEntity()->GetWorldPos();
		state.m_timer = m_timer;

		return state;
	}

	CField* CFruit::GetField()
	{
		auto pEntIT = gEnv->pEntitySystem->GetEntityIterator();
		while (IEntity* pEntity = pEntIT->Next())
		{
			if (CField* pField = pEntity->GetComponent<CField>())
				return pField;
		}

		return nullptr;
	}
}