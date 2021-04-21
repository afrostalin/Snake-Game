// Copyright (C) 2021 Space Raccoon Game Studio. All rights reserved. Contacts: <business@space-raccoon.com>
// Created by AfroStalin

#include "StdAfx.h"
#include "Barrier.h"
#include "Field.h"

#include <CryRenderer/IRenderAuxGeom.h>

namespace Snake
{
	namespace
	{
		static void RegisterBarrierComponent(Schematyc::IEnvRegistrar& registrar)
		{
			Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
			{
				scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(CBarrier));
			}
		}

		CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterBarrierComponent);
	}

	void CBarrier::Initialize()
	{
		m_alignButton = Serialization::ActionButton(std::function<void()>([this]()
			{
				AlignToField();
			}));
	}

	Cry::Entity::EventFlags CBarrier::GetEventMask() const
	{
		return 	Cry::Entity::EventFlags();
	}

	void CBarrier::RegisterListener(CBarrier::SListener* pListener)
	{
		if (!stl::find(m_listeners, pListener))
			m_listeners.push_back(pListener);
	}

	void CBarrier::UnregisterListener(CBarrier::SListener* pListener)
	{
		stl::find_and_erase(m_listeners, pListener);
	}

	void CBarrier::OnCollideWithSnake(const int16& partIndex)
	{
		for (const auto& it : m_listeners)
		{
			it->OnCollideWithSnake(partIndex);
		}
	}

	void CBarrier::AlignToField()
	{
		auto pEntIT = gEnv->pEntitySystem->GetEntityIterator();
		while (IEntity * pEntity = pEntIT->Next())
		{
			if (CField * pField = pEntity->GetComponent<CField>())
			{
				const Vec3 closestPoint = pField->GetClosestPoint(GetEntity()->GetWorldPos(), 2.f);
		
				if (!closestPoint.IsZero())
				{
					GetEntity()->SetPos(closestPoint);
					m_alignedWorldPos = closestPoint;
				}
		
				break;
			}
		}
	}

	AABB CBarrier::GetBBox()
	{
		return AABB(GetEntity()->GetWorldPos(), bboxSize);
	}
}