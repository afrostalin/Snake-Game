// Copyright (C) 2021 Space Raccoon Game Studio. All rights reserved. Contacts: <business@space-raccoon.com>
// Created by AfroStalin

#include "StdAfx.h"
#include "Camera.h"
#include "Snake.h"

namespace Snake
{
	namespace
	{
		static void RegisterCameraComponent(Schematyc::IEnvRegistrar& registrar)
		{
			Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
			{
				scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(CSnakeCamera));
			}
		}

		CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterCameraComponent);
	}

	void CSnakeCamera::Initialize()
	{
		m_pCameraComponent = m_pEntity->GetOrCreateComponent<Cry::DefaultComponents::CCameraComponent>();
	}

	Cry::Entity::EventFlags CSnakeCamera::GetEventMask() const
	{
		return Cry::Entity::EEvent::Update;
	}

	void CSnakeCamera::ProcessEvent(const SEntityEvent& event)
	{
		switch (event.event)
		{
		case Cry::Entity::EEvent::Update:
		{
			const float frameTime = event.fParam[0];
			Update(frameTime);
			break;
		}
		default:
			break;
		}
	}

	void CSnakeCamera::Update(float frameTime)
	{
		IEntity* m_pSnake = gEnv->pEntitySystem->GetEntity(LOCAL_PLAYER_ENTITY_ID);

#if REPLAY_SYSTEM
		if (g_pGame->pReplaySystem->IsPlaying() || g_pGame->pReplaySystem->IsPaused())
		{
			m_pSnake = gEnv->pEntitySystem->GetEntity(g_pGame->pReplaySystem->GetReplaySnakeID());
		}
#endif

		if (m_pSnake)
		{
			switch (m_mode)
			{
			case Snake::ECameraMode::Default:
			{
				// World position
				{
					const Vec3 nextPos = m_pSnake->GetWorldPos();
					const float speed = g_pGameCVars->pl_cameraInterpolationSpeed;
					Vec3 newPos = GetEntity()->GetWorldPos();

					Interpolate(newPos.x, nextPos.x, speed, frameTime);
					Interpolate(newPos.y, nextPos.y, speed, frameTime);
					Interpolate(newPos.z, nextPos.z, speed, frameTime);

					Matrix34 tm = IDENTITY;
					tm.SetTranslation(newPos);

					GetEntity()->SetWorldTM(tm);
				}

				// Local offset
				{
					Matrix34 localTransform = IDENTITY;

					localTransform.SetRotation33(Matrix33::CreateRotationX(DEG2RAD(g_pGameCVars->pl_cameraSlope)));

					localTransform.SetTranslation(Vec3(g_pGameCVars->pl_cameraXoffset, g_pGameCVars->pl_cameraYoffset, g_pGameCVars->pl_cameraDistanceFromPlayer));

					m_pCameraComponent->SetTransformMatrix(localTransform);

					m_lastWorldTM.SetTranslation(GetEntity()->GetWorldPos() + localTransform.GetTranslation());
					m_lastWorldTM.SetRotation33(Matrix33::CreateRotationX(DEG2RAD(g_pGameCVars->pl_cameraSlope)));
				}
			}
			break;
			case Snake::ECameraMode::Top:
			{
				// World position
				{
					const int terrainSize = gEnv->p3DEngine->GetTerrainSize();
					const float terrainHeight = gEnv->p3DEngine->GetTerrainZ(float(terrainSize / 2), float(terrainSize / 2));
					const Vec3 nextPos = Vec3(float(terrainSize / 2), float(terrainSize / 2), terrainHeight);
					const float speed = g_pGameCVars->pl_cameraInterpolationSpeed;

					Vec3 newPos = GetEntity()->GetWorldPos();

					Interpolate(newPos.x, nextPos.x, speed, frameTime);
					Interpolate(newPos.y, nextPos.y, speed, frameTime);
					Interpolate(newPos.z, nextPos.z, speed, frameTime);

					Matrix34 tm = IDENTITY;
					tm.SetTranslation(newPos);

					GetEntity()->SetWorldTM(tm);
				}

				// Local offset
				{
					Matrix34 localTransform = IDENTITY;

					localTransform.SetRotation33(Matrix33::CreateRotationX(DEG2RAD(-90)));

					localTransform.SetTranslation(Vec3(0.f, 0.f, g_pGameCVars->pl_cameraDistanceFromPlayerTOP));

					m_pCameraComponent->SetTransformMatrix(localTransform);

					m_lastWorldTM.SetTranslation(GetEntity()->GetWorldPos() + localTransform.GetTranslation());
					m_lastWorldTM.SetRotation33(Matrix33::CreateRotationX(DEG2RAD(-90)));
				}
			}
			break;
			case Snake::ECameraMode::FreeMode:
			{
				CSnake* pSnake = m_pSnake->GetComponent<CSnake>();
				if (pSnake == nullptr)
					return;

				const CEnumFlags<CSnake::EInputFlag>& inputFlags = pSnake->GetInputFlags();
				const Quat& lookOrientation = pSnake->GetLookOrientation();

				const Vec3 camPos = GetEntity()->GetWorldPos();
				const Vec3 snakePos = m_pSnake->GetWorldPos();

				Matrix34 worldTM = IDENTITY;
				worldTM.SetRotation33(Matrix33(lookOrientation));

				const float moveSpeed = 15.f;

				bool worldTmUpdated = false;

				if (!lookOrientation.IsIdentity())
					worldTmUpdated = true;

				Vec3 finalVelocity = ZERO;

				if (inputFlags & CSnake::EInputFlag::MoveLeft)
				{
					finalVelocity -= lookOrientation.GetColumn0();
					worldTmUpdated = true;
				}

				if (inputFlags & CSnake::EInputFlag::MoveRight)
				{
					finalVelocity += lookOrientation.GetColumn0();
					worldTmUpdated = true;
				}

				if (inputFlags & CSnake::EInputFlag::MoveForward)
				{
					finalVelocity += lookOrientation.GetColumn1();
					worldTmUpdated = true;
				}

				if (inputFlags & CSnake::EInputFlag::MoveBack)
				{
					finalVelocity -= lookOrientation.GetColumn1();
					worldTmUpdated = true;
				}

				if (worldTmUpdated)
				{
					finalVelocity *= 0.001f + (moveSpeed * 0.01f);

					worldTM.SetTranslation(camPos + finalVelocity);

					m_lastWorldTM = worldTM;

					GetEntity()->SetWorldTM(worldTM);
				}

				if (!worldTmUpdated && m_lastWorldTM.IsValid() && !m_lastWorldTM.IsIdentity())
				{
					GetEntity()->SetWorldTM(m_lastWorldTM);
				}

				// Local offset
				{
					Matrix34 localTransform = IDENTITY;
					m_pCameraComponent->SetTransformMatrix(localTransform);
				}
			}
			break;
			default:
				break;
			}
		}
	}
}