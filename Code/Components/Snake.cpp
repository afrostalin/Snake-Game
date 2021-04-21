// Copyright (C) 2020 Space Raccoon Game Studio. All rights reserved. Contacts: <business@space-raccoon.com>
// Created by AfroStalin

#include "StdAfx.h"
#include "Snake.h"
#include "SnakePart.h"
#include "GameRules.h"
#include "GamePlugin.h"

#include <CryRenderer/IRenderAuxGeom.h>
#include <CryInput/IHardwareMouse.h>
#include <CrySchematyc/Env/Elements/EnvComponent.h>
#include <CryCore/StaticInstanceList.h>
#include <CryNetwork/Rmi.h>
#include <CryMath/Random.h>
#include <IForceFeedbackSystem.h>
#include <CrySystem/IConsole.h>
#include <CrySystem/ConsoleRegistration.h>

namespace Snake
{
	void CSnake::Initialize()
	{
		GetEntity()->LoadGeometry(GetOrMakeEntitySlotId(), "objects/characters/snake/head.cgf");

		GetEntity()->GetNetEntity()->BindToNetwork();
		GetEntity()->SetFlags(GetEntity()->GetFlags() | ENTITY_FLAG_CASTSHADOW);

		m_snakeParts[0] = SSnakePart();
	}

	void CSnake::OnShutDown()
	{
		gEnv->pEntitySystem->RemoveEntity(m_cameraEntityID);
	}

	void CSnake::InitializeLocalPlayer()
	{
		// Camera entity. It's splitted from snake (interpolation, free look)
		{
			SEntitySpawnParams params;
			params.pClass = gEnv->pEntitySystem->GetClassRegistry()->GetDefaultClass();
			params.vPosition = GetEntity()->GetWorldPos();

			IEntity* pCameraEntity = gEnv->pEntitySystem->SpawnEntity(params);
			pCameraEntity->CreateComponent<CSnakeCamera>();
			m_cameraEntityID = pCameraEntity->GetId();
		}

		m_pInputComponent = m_pEntity->GetOrCreateComponent<Cry::DefaultComponents::CInputComponent>();
		m_pAudioListener = m_pEntity->GetOrCreateComponent<Cry::Audio::DefaultComponents::CListenerComponent>();

		m_pInputComponent->RegisterAction("player", "moveleft", [this](int activationMode, float value)
			{
				if (m_isAlive && !gEnv->pGameFramework->IsGamePaused() || GetGameraMode() == ECameraMode::FreeMode)
				{
					HandleInputFlagChange(EInputFlag::MoveLeft, (EActionActivationMode)activationMode);
				}
			});

		m_pInputComponent->BindAction("player", "moveleft", eAID_KeyboardMouse, EKeyId::eKI_A);
		m_pInputComponent->BindAction("player", "moveleft", eAID_KeyboardMouse, EKeyId::eKI_Left);
		m_pInputComponent->BindAction("player", "moveleft", eAID_KeyboardMouse, EKeyId::eKI_Orbis_Left);
		m_pInputComponent->BindAction("player", "moveleft", eAID_KeyboardMouse, EKeyId::eKI_XI_DPadLeft);

		m_pInputComponent->RegisterAction("player", "moveright", [this](int activationMode, float value)
			{
				if (m_isAlive && !gEnv->pGameFramework->IsGamePaused() || GetGameraMode() == ECameraMode::FreeMode)
				{
					HandleInputFlagChange(EInputFlag::MoveRight, (EActionActivationMode)activationMode);
				}
			});
		m_pInputComponent->BindAction("player", "moveright", eAID_KeyboardMouse, EKeyId::eKI_D);
		m_pInputComponent->BindAction("player", "moveright", eAID_KeyboardMouse, EKeyId::eKI_Right);
		m_pInputComponent->BindAction("player", "moveright", eAID_KeyboardMouse, EKeyId::eKI_Orbis_Right);
		m_pInputComponent->BindAction("player", "moveright", eAID_KeyboardMouse, EKeyId::eKI_XI_DPadRight);

		m_pInputComponent->RegisterAction("player", "moveforward", [this](int activationMode, float value)
			{
				if (m_isAlive && !gEnv->pGameFramework->IsGamePaused() || GetGameraMode() == ECameraMode::FreeMode)
				{
					HandleInputFlagChange(EInputFlag::MoveForward, (EActionActivationMode)activationMode);
				}
			});
		m_pInputComponent->BindAction("player", "moveforward", eAID_KeyboardMouse, EKeyId::eKI_W);
		m_pInputComponent->BindAction("player", "moveforward", eAID_KeyboardMouse, EKeyId::eKI_Up);
		m_pInputComponent->BindAction("player", "moveforward", eAID_KeyboardMouse, EKeyId::eKI_Orbis_Up);
		m_pInputComponent->BindAction("player", "moveforward", eAID_KeyboardMouse, EKeyId::eKI_XI_DPadUp);

		m_pInputComponent->RegisterAction("player", "moveback", [this](int activationMode, float value)
			{
				if (m_isAlive && !gEnv->pGameFramework->IsGamePaused() || GetGameraMode() == ECameraMode::FreeMode)
				{
					HandleInputFlagChange(EInputFlag::MoveBack, (EActionActivationMode)activationMode);
				}
			});
		m_pInputComponent->BindAction("player", "moveback", eAID_KeyboardMouse, EKeyId::eKI_S);
		m_pInputComponent->BindAction("player", "moveback", eAID_KeyboardMouse, EKeyId::eKI_Down);
		m_pInputComponent->BindAction("player", "moveback", eAID_KeyboardMouse, EKeyId::eKI_Orbis_Down);
		m_pInputComponent->BindAction("player", "moveback", eAID_KeyboardMouse, EKeyId::eKI_XI_DPadDown);

		// Pause
		{
			m_pInputComponent->RegisterAction("game", "pause", [this](int activationMode, float value)
				{
					if (m_isAlive)
					{
						if (activationMode == EActionActivationMode::eAAM_OnPress)
						{
							if (!gEnv->IsEditor())
							{
								if (gEnv->pGameFramework->IsGamePaused())
								{
									gEnv->pGameFramework->PauseGame(false, false);
								}
								else
								{
									gEnv->pGameFramework->PauseGame(true, false);
								}
							}
						}
					}
				});
			m_pInputComponent->BindAction("game", "pause", eAID_KeyboardMouse, EKeyId::eKI_Escape);
			m_pInputComponent->BindAction("game", "pause", eAID_KeyboardMouse, EKeyId::eKI_XI_Start);
		}

		// Camera mode
		{
			m_pInputComponent->RegisterAction("game", "change_camera_mode", [this](int activationMode, float value)
				{
					if (m_isAlive)
					{
						if (g_pGameCVars->pl_cameraModeChangingType == 0)
						{
							if (activationMode == EActionActivationMode::eAAM_OnPress || activationMode == EActionActivationMode::eAAM_OnRelease)
							{
								if (GetGameraMode() == ECameraMode::Default)
								{
									ChangeCameraMode(ECameraMode::Top);
								}
								else
								{
									ChangeCameraMode(ECameraMode::Default);
								}
							}
						}
						else
						{
							if (activationMode == EActionActivationMode::eAAM_OnPress)
							{
								if (GetGameraMode() == ECameraMode::Default)
								{
									ChangeCameraMode(ECameraMode::Top);
								}
								else
								{
									ChangeCameraMode(ECameraMode::Default);
								}
							}
						}
					}
				});
			m_pInputComponent->BindAction("game", "change_camera_mode", eAID_KeyboardMouse, EKeyId::eKI_LAlt);
			m_pInputComponent->BindAction("game", "change_camera_mode", eAID_KeyboardMouse, EKeyId::eKI_RAlt);
			m_pInputComponent->BindAction("game", "change_camera_mode", eAID_KeyboardMouse, EKeyId::eKI_XI_ShoulderR);
			m_pInputComponent->BindAction("game", "change_camera_mode", eAID_KeyboardMouse, EKeyId::eKI_Orbis_R1);
		}

#ifndef RELEASE
		// Free camera
		m_pInputComponent->RegisterAction("player", "enable_free_camera", [this](int activationMode, float value)
			{
				if (activationMode == eAAM_OnPress)
				{
					const ECameraMode currentCamMod = GetGameraMode();

					if (currentCamMod != ECameraMode::FreeMode)
					{
						m_lastCameraMode = currentCamMod;

						ChangeCameraMode(ECameraMode::FreeMode);
					}
					else
					{
						ChangeCameraMode(m_lastCameraMode);
					}
				}
			});
		m_pInputComponent->BindAction("player", "enable_free_camera", eAID_KeyboardMouse, EKeyId::eKI_F5);
#endif


		m_pInputComponent->RegisterAction("player", "mouse_rotateyaw", [this](int activationMode, float value) { m_mouseDeltaRotation.x -= value; });
		m_pInputComponent->BindAction("player", "mouse_rotateyaw", eAID_KeyboardMouse, EKeyId::eKI_MouseX);

		m_pInputComponent->RegisterAction("player", "mouse_rotatepitch", [this](int activationMode, float value) { m_mouseDeltaRotation.y -= value; });
		m_pInputComponent->BindAction("player", "mouse_rotatepitch", eAID_KeyboardMouse, EKeyId::eKI_MouseY);

		if (g_pGameCVars->g_controlType == 1 && gEnv->pInput->HasInputDeviceOfType(EInputDeviceType::eIDT_Gamepad))
		{
			IInputDevice* pDevice = gEnv->pInput->GetDevice(0, EInputDeviceType::eIDT_Gamepad);

			if (pDevice)
			{
				gEnv->pInput->ForceFeedbackSetDeviceIndex(0);
			}
		}

		if (g_pGameCVars->pl_cameraDefaultMode != 0)
		{
			if (g_pGameCVars->pl_cameraDefaultMode == 1)
			{
				ChangeCameraMode(ECameraMode::Top);
			}

		}

		CheckGameMode();
	}

	Cry::Entity::EventFlags CSnake::GetEventMask() const
	{
		return
			Cry::Entity::EEvent::BecomeLocalPlayer |
			Cry::Entity::EEvent::Update |
			Cry::Entity::EEvent::Reset |
			Cry::Entity::EEvent::AnimationEvent;
	}

	void CSnake::ProcessEvent(const SEntityEvent& event)
	{
		switch (event.event)
		{
		case Cry::Entity::EEvent::Reset:
		{
			m_isAlive = event.nParam[0] != 0;

			if (m_isAlive)
			{
				Revive(ZERO);
			}
			else
			{
				Reset();

				if (gEnv->IsEditor())
				{
					if (GetGameraMode() == ECameraMode::FreeMode)
					{
						ChangeCameraMode(m_lastCameraMode);
					}
				}
			}
		}
		break;
		case Cry::Entity::EEvent::BecomeLocalPlayer:
		{
			InitializeLocalPlayer();
		}
		break;
		case Cry::Entity::EEvent::Update:
		{
			const float frameTime = event.fParam[0];

			if (m_isRequstReviveOnNextUpdate)
			{
				Revive(ZERO);
			}

			if (IsAlive())
			{
				UpdateMovementRequest(frameTime);

				UpdateAnimations(frameTime);

				CheckCollideWithFruits(frameTime);

				CheckCollideWithParts(frameTime);

				CheckCollideWithBorders(frameTime);

				CheckCollideWithBarriers(frameTime);

				UpdatePlayerStatistic(frameTime);

				// Update silhouette
				//{
				//	GetEntity()->GetRenderNode()->m_nHUDSilhouettesParam = m_snakeParts[0].m_nHUDSilhouettesParam;
				//	m_snakeParts[0].m_nHUDSilhouettesParam = 0;
				//}

				// It's need to be exported to normal UI
				{
					CryWatch("Current score : %d", GetStatistic().currentScore);
					CryWatch("Best score : %d", GetStatistic().bestScore);
					CryWatch("Apples : %d", GetStatistic().harvestedApples);
					CryWatch("Deathes : %d", GetStatistic().deathCount);
					CryWatch("Path lenght : %d m.", GetStatistic().pathSize);
				}
			}

			// Used for free camera
			UpdateLookDirectionRequest(frameTime);
		}
		break;
		case  Cry::Entity::EEvent::AnimationEvent:
		{
			//const AnimEventInstance* pAnimEvent = reinterpret_cast<const AnimEventInstance*>(event.nParam[0]);

			//if (pAnimEvent == nullptr)
			//	break;
		}
		break;
		}
	}

	void CSnake::UpdateMovementRequest(float frameTime)
	{
		CRY_PROFILE_FUNCTION(EProfiledSubsystem::PROFILE_GAME);

#ifndef RELEASE
		if (g_pGameCVars->dg_snakeCoordsOnField)
		{
			const Vec3 nextPosCoords = GetField()->GetSpawnPointByCoords(m_snakeParts[0].m_nextPosCoords).worldPos;
			gEnv->pRenderer->GetIRenderAuxGeom()->DrawAABB(AABB(nextPosCoords, 1.f), true, ColorB(255, 255, 255), EBoundingBoxDrawStyle::eBBD_Extremes_Color_Encoded);
		}
#endif

		const float meteresPerStep = g_pGameCVars->g_meters_per_step;
		float defaultSnakeSpeed = g_pGameCVars->pl_default_speed;
		float speedBoost = 0.1f * GetSnakeSize();
		float abilitySpeedBoost = 0.f;

		speedBoost = std::min(speedBoost, defaultSnakeSpeed);

		m_currentSnakeSpeed = defaultSnakeSpeed + speedBoost + abilitySpeedBoost;

		const Vec3 currentHeadPosition = GetEntity()->GetWorldPos();

		m_snakeParts[0].m_currentPosition = currentHeadPosition;
		m_snakeParts[0].m_currentVelocity = m_currentVelocity;
		m_snakeParts[0].m_direction = m_lastDirection;

		if (GetGameraMode() != ECameraMode::FreeMode && !m_isInputBlocked)
		{
			if (m_inputFlags & EInputFlag::MoveLeft)
			{
				if (IsCanChangeDirection(EMovementDirection::ToLeft))
				{
					m_newDirection = EMovementDirection::ToLeft;
				}
			}
			else if (m_inputFlags & EInputFlag::MoveRight)
			{
				if (IsCanChangeDirection(EMovementDirection::ToRight))
				{
					m_newDirection = EMovementDirection::ToRight;
				}
			}
			else if (m_inputFlags & EInputFlag::MoveForward)
			{
				if (IsCanChangeDirection(EMovementDirection::Forward))
				{
					m_newDirection = EMovementDirection::Forward;
				}
			}
			else if (m_inputFlags & EInputFlag::MoveBack)
			{
				if (IsCanChangeDirection(EMovementDirection::Backward))
				{
					m_newDirection = EMovementDirection::Backward;
				}
			}
		}

		// Update head position/rotation/scale
		GetEntity()->SetPosRotScale
		(
			Vec3::CreateLerp(currentHeadPosition, currentHeadPosition + m_currentVelocity * m_currentSnakeSpeed, frameTime),
			UpdatePartRotation(m_snakeParts[0], frameTime),
			Vec3(1.f)
		);

		// Update parts positions
		{
#ifndef RELEASE
			if (g_pGameCVars->dg_snakeParts)
			{
				gEnv->pRenderer->GetIRenderAuxGeom()->DrawCone(currentHeadPosition + Vec3(0.f, 0.f, 1.5f), m_currentVelocity, 0.2f, 1.0f, ColorB(0, 255, 0));
			}
#endif
			for (int i = 1; i < GetSnakeSize(); i++)
			{
				const Vec3 currentPartPos = m_snakeParts[i].m_currentPosition;
				const Vec3 lastPartPos = m_snakeParts[i].m_lastPosition;
				const EMovementDirection currentPartDirection = m_snakeParts[i].m_direction;

				Vec3 partVelocity = GetVelocityFromDirection(currentPartDirection);

				m_snakeParts[i].m_currentVelocity = partVelocity;

#ifndef RELEASE
				if (g_pGameCVars->dg_snakeCoordsOnField)
				{
					const Vec3 partCoords = GetField()->GetSpawnPointByCoords(m_snakeParts[i].m_lastPosCoords).worldPos;
					gEnv->pRenderer->GetIRenderAuxGeom()->DrawAABB(AABB(partCoords, 1.f), true, ColorB(0, 0, 255), EBoundingBoxDrawStyle::eBBD_Extremes_Color_Encoded);
				}

				if (g_pGameCVars->dg_snakeParts)
				{
					const Vec3 debugOffset = Vec3(0.f, 0.f, 1.5f);
					gEnv->pRenderer->GetIRenderAuxGeom()->DrawCone(currentPartPos + debugOffset, partVelocity, 0.2f, 1.0f, ColorB(0, 0, 0));
				}
#endif
				const float distanceToPartGoal = lastPartPos.GetDistance(currentPartPos);

				if (distanceToPartGoal < meteresPerStep)
				{
					const Vec3 newPos = Vec3::CreateLerp(currentPartPos, currentPartPos + partVelocity * m_currentSnakeSpeed, frameTime);
					m_snakeParts[i].m_currentPosition = newPos;
				}

				Quat partRotation = UpdatePartRotation(m_snakeParts[i], frameTime);

				// Move part entity
				if(m_snakeParts[i].m_partEntity != nullptr)
				{
					m_snakeParts[i].m_partEntity->SetPosRotScale(m_snakeParts[i].m_currentPosition, partRotation, Vec3(1.f));
				}
			}
		}

		const float distanceToGoal = m_lastHeadPosition.GetDistance(currentHeadPosition);

		if (distanceToGoal >= meteresPerStep)
		{
			m_isSnakeCanChangeDirection = true;
			m_isInputBlocked = false;
		}

		// Go to the future
		if (m_newDirection != m_lastDirection && distanceToGoal >= (meteresPerStep - g_pGameCVars->pl_maxDistForFastChangeDir))
		{
			m_isSnakeCanChangeDirection = true;
			m_isInputBlocked = false;
		}

		if (m_isSnakeCanChangeDirection)
		{
			if (!IsCanChangeDirection(m_newDirection))
				m_newDirection = m_lastDirection;

			const float newHeadRotation = GetDegreesFromDirection(m_newDirection);

			m_snakeParts[0].m_lastRotation = m_snakeParts[0].m_nextRotation;
			m_snakeParts[0].m_nextRotation = newHeadRotation;
			m_snakeParts[0].m_currentRotation = m_snakeParts[0].m_lastRotation;

			m_snakeParts[0].m_lastPosition = m_nextHeadPosition;
			m_snakeParts[0].m_direction = m_lastDirection;
			m_snakeParts[0].m_lastDirection = m_lastDirection;

			m_lastVelocityGoal = GetVelocityFromDirection(m_newDirection) * meteresPerStep;

			if (m_lastDirection != m_newDirection)
			{
				GetEntity()->SetPos(m_nextHeadPosition);
			}

			m_lastDirection = m_newDirection;
			m_lastHeadPosition = m_nextHeadPosition;
			m_nextHeadPosition = m_nextHeadPosition + m_lastVelocityGoal;

			m_currentVelocity = Vec3(m_nextHeadPosition - m_lastHeadPosition).Normalize();

			if (m_currentVelocity.IsZero())
			{
				LogWarning("Snake velocity equels zero - potential wrong behavior!");
			}

			m_snakeParts[0].m_nextPosition = m_nextHeadPosition;

			if (m_snakeParts[0].m_lastPosCoords.IsValid())
			{
				m_snakeParts[0].m_lastPosCoords = m_snakeParts[0].m_nextPosCoords;
			}
			else
			{
				m_snakeParts[0].m_lastPosCoords = GetField()->GetSpawnPointCoords(m_lastHeadPosition);
			}

			if (m_snakeParts[0].m_nextPosCoords.IsValid())
			{
				m_snakeParts[0].m_nextPosCoords = GetUpdatedCoordsByDirection(m_snakeParts[0].m_nextPosCoords, m_newDirection);
			}
			else
			{
				m_snakeParts[0].m_nextPosCoords = GetField()->GetSpawnPointCoords(m_nextHeadPosition);
			}

			for (int i = GetSnakeSize(); i > 0; --i)
			{
				const Vec3 currentPartPos = m_snakeParts[i].m_currentPosition;
				const EMovementDirection currentPartDirection = m_snakeParts[i].m_direction;
				const EMovementDirection newPartDirection = m_snakeParts[i - 1].m_direction;
				const SCoordsOnField& newPartCoords = m_snakeParts[i - 1].m_nextPosCoords;
				const Vec3 currentNextPos = m_snakeParts[i].m_nextPosition;

				Vec3 goalPartPos = ZERO;

				if (i == 1)
					goalPartPos = m_snakeParts[i - 1].m_lastPosition;
				else
					goalPartPos = m_snakeParts[i - 1].m_nextPosition;

				if (currentPartDirection != newPartDirection || (currentPartPos.GetDistance(currentNextPos) > meteresPerStep * 2.f))
				{
					m_snakeParts[i].m_currentPosition = currentNextPos;
				}

				m_snakeParts[i].m_direction = newPartDirection;
				m_snakeParts[i].m_nextPosition = goalPartPos;
				m_snakeParts[i].m_lastPosition = currentNextPos;
				m_snakeParts[i].m_lastPosCoords = m_snakeParts[i].m_nextPosCoords;
				m_snakeParts[i].m_nextPosCoords = newPartCoords;


				m_snakeParts[i].m_lastRotation = GetDegreesFromDirection(currentPartDirection);
				m_snakeParts[i].m_currentRotation = GetDegreesFromDirection(currentPartDirection);
				m_snakeParts[i].m_nextRotation = GetDegreesFromDirection(newPartDirection);
			}

			if (GetSnakeSize() == 1)
			{
				IncreaseSnake(g_pGameCVars->pl_default_size - 1);

				for (int i = 1; i < g_pGameCVars->pl_default_size; i++)
				{
					m_snakeParts[i].m_direction = m_newDirection;

					Vec3 partVelocity = GetVelocityFromDirection(m_snakeParts[1].m_direction);

					m_snakeParts[i].m_lastPosition = m_snakeParts[i].m_currentPosition = m_lastHeadPosition - (partVelocity * meteresPerStep * (float)i);
					m_snakeParts[i].m_nextPosition = m_lastHeadPosition - (partVelocity * meteresPerStep * (float)(i - 1));
				}

			}

			m_isSnakeCanChangeDirection = false;

			m_statistic.pathSize += (int)meteresPerStep;
		}
	}

	void CSnake::UpdateLookDirectionRequest(float frameTime)
	{
		const float mouseDeltaTreshold = 0.0001f;
		const float rotationSpeed = 0.002f;
		const float rotationLimitsMinPitch = -1.5f;
		const float rotationLimitsMaxPitch = 1.5f;

		if (m_mouseDeltaRotation.IsEquivalent(ZERO, mouseDeltaTreshold))
			return;

		Ang3 ypr = CCamera::CreateAnglesYPR(Matrix33(m_lookOrientation));
		ypr.x += m_mouseDeltaRotation.x * rotationSpeed;
		ypr.y = CLAMP(ypr.y + m_mouseDeltaRotation.y * rotationSpeed, rotationLimitsMinPitch, rotationLimitsMaxPitch);
		ypr.z = 0;

		m_lookOrientation = Quat(CCamera::CreateOrientationYPR(ypr));

		m_mouseDeltaRotation = ZERO;
	}

	void CSnake::UpdatePlayerStatistic(float frameTime)
	{
		CRY_PROFILE_FUNCTION(EProfiledSubsystem::PROFILE_GAME);


		const int score = m_statistic.harvestedApples * 100; // TODO : Different points in different gamemodes/apples ?

		m_statistic.snakeSize = GetSnakeSize();
		m_statistic.currentScore = score;

		if (m_statistic.bestScore < m_statistic.currentScore)
			m_statistic.bestScore = m_statistic.currentScore;

		if (m_statistic.maxSnakeSize < m_statistic.snakeSize)
		{
			m_statistic.maxSnakeSize = m_statistic.snakeSize;
		}
	}

	void CSnake::UpdateAnimations(float frameTime)
	{
		SFruitObject fruit = GetClosestFruit();

		constexpr float triggerDist = 4.f; TODO("Move trigger distance for mounth opening to CVar");
		const float dist = fruit.worldPos.GetDistance(GetEntity()->GetWorldPos());

		if (dist <= triggerDist)
		{
			// Open mouth here
		}
		else
		{
			// Close mouth here
		}
	}

	Quat CSnake::UpdatePartRotation(SSnakePart& part, float frameTime)
	{
		const float currentDeg = part.m_currentRotation;
		const float lastDeg = part.m_lastRotation;

		float nextDeg = part.m_nextRotation;

		if (fabs(nextDeg - lastDeg) > 90.f)
		{
			if (lastDeg == 270.f && nextDeg == 0.f)
			{
				nextDeg = 360.f;
			}
			else if (lastDeg == 0.f && nextDeg == 270.f)
			{
				nextDeg = -90.f;
			}
		}

		/*if (fabs(nextDeg - lastDeg) > 90.f)
		{
#ifndef RELEASE
			LogWarning("WRONG ROTATION. Part <%d>. Last deg <%f>. Next deg <%f>", part.eID, lastDeg, nextDeg);
#endif
		}*/

		const float updatedDeg = Lerp<float, float>(currentDeg, nextDeg, frameTime * m_currentSnakeSpeed * 2.f);

		part.m_currentRotation = updatedDeg;

		return Quat::CreateRotationZ(DEG2RAD(updatedDeg));
	}

	void CSnake::CheckCollideWithBorders(float frameTime)
	{
		CRY_PROFILE_FUNCTION(EProfiledSubsystem::PROFILE_GAME);

		CField* pField = GetField();

		if (pField)
		{
			std::vector<Vec3> cornerPoints;

			if (pField->GetCornerPoints(cornerPoints))
			{
				// We need add vertical offset for points, because snake have additional offset by Z 
				for (auto& it : cornerPoints)
				{
					it += Vec3(0.f, 0.f, g_pGameCVars->g_snakeSpawnOffsetZ / 2.f);
				}

				Vec3 hit = ZERO;
				const AABB hitBbox = AABB(GetEntity()->GetWorldPos(), 0.9f);

				if (Intersect::Ray_AABB(Ray(cornerPoints[0], cornerPoints[1] - cornerPoints[0]), hitBbox, hit)
					|| Intersect::Ray_AABB(Ray(cornerPoints[1], cornerPoints[2] - cornerPoints[1]), hitBbox, hit)
					|| Intersect::Ray_AABB(Ray(cornerPoints[2], cornerPoints[3] - cornerPoints[2]), hitBbox, hit)
					|| Intersect::Ray_AABB(Ray(cornerPoints[3], cornerPoints[0] - cornerPoints[3]), hitBbox, hit))
				{
					KillSnake(ESnakeDeathType::CollideWithBorders);
				}
			}
		}
	}

	void CSnake::CheckCollideWithParts(float frameTime)
	{
		CRY_PROFILE_FUNCTION(EProfiledSubsystem::PROFILE_GAME);

		const Vec3 currentHeadPos = GetEntity()->GetWorldPos();
		const AABB bbox = AABB(currentHeadPos, 0.9f);
		const float partBboxSize = 0.6f;

#ifndef RELEASE
		if (g_pGameCVars->dg_snakeParts)
		{
			gEnv->pRenderer->GetIRenderAuxGeom()->DrawAABB(bbox, false, ColorB(255, 255, 255), EBoundingBoxDrawStyle::eBBD_Faceted);

			// First part
			{
				const AABB partBBox = AABB(m_snakeParts[1].m_currentPosition, partBboxSize);
				gEnv->pRenderer->GetIRenderAuxGeom()->DrawAABB(partBBox, false, ColorB(0, 255, 0), EBoundingBoxDrawStyle::eBBD_Faceted);
			}
		}
#endif

		const float meteresPerStep = g_pGameCVars->g_meters_per_step;

		for (int i = 2; i < GetSnakeSize(); i++)
		{
			const Vec3 currentPartPos = m_snakeParts[i].m_currentPosition;
			const Vec3 lastPartPos = m_snakeParts[i].m_lastPosition;
			const float distanceToGoal = lastPartPos.GetDistance(currentPartPos);
			const AABB partBBox = AABB(currentPartPos, partBboxSize);

#ifndef RELEASE
			if (g_pGameCVars->dg_snakeParts)
			{
				gEnv->pRenderer->GetIRenderAuxGeom()->DrawAABB(partBBox, false, ColorB(0, 255, 0), EBoundingBoxDrawStyle::eBBD_Faceted);
			}
#endif

			if (distanceToGoal > meteresPerStep)
				continue;

			if (bbox.IsIntersectBox(partBBox))
			{
				LogDebug("[Snake] Died from collide with part. Part id <%d>", i);

				KillSnake(ESnakeDeathType::CollideWithParts);
				break;
			}
		}
	}

	void CSnake::CheckCollideWithFruits(float frameTime)
	{
		CRY_PROFILE_FUNCTION(EProfiledSubsystem::PROFILE_GAME);

		// Check collide with fruits
		if (CField* pField = GetField())
		{
			const SCoordsOnField& nextHeadCoords = m_snakeParts[0].m_nextPosCoords;
			const Vec3 currentPos = GetEntity()->GetWorldPos();
			const Vec3 collidingPos = currentPos + m_currentVelocity.Normalize();

			bool isIntersect = false;

			for (const auto& fruit : pField->GetFruits())
			{
				if (CFruit* pFruit = fruit.pFruit)
				{
					const AABB fruitBBox = AABB(fruit.worldPos, 1.f);

					if (nextHeadCoords == fruit.coords && fruitBBox.IsIntersectBox(AABB(collidingPos, 1.f)))
					{
						pFruit->OnCollideWithSnake(this);

						constexpr CryAudio::ControlId trigger = CryAudio::StringToId("Play_action_eating_00");

						PlaySound(trigger);

						gEnv->pEntitySystem->RemoveEntity(fruit.id);
						isIntersect = true;
						break;
					}
				}
			}

			if (isIntersect)
			{
				m_statistic.harvestedApples++;

				IncreaseSnake();
			}
			else // Spawn fruit
			{
				int fruitCount = pField->GetFruitsCount();

				if (fruitCount == 0) // Only one fruit can be spawned
				{
					SFruitSpawnInfo info = pField->SpawnFruit(this);
				}
			}
		}
	}

	void CSnake::CheckCollideWithBarriers(float frameTime)
	{
		CRY_PROFILE_FUNCTION(EProfiledSubsystem::PROFILE_GAME);

		const Vec3 currentHeadPos = GetEntity()->GetWorldPos();
		const AABB bbox = AABB(currentHeadPos, 0.9f);


		if (CField* pField = GetField())
		{
			const std::vector<SBarrierObject>& barriers = pField->GetBarriers();

			for (int i = 0; i < GetSnakeSize(); i++)
			{
				const SSnakePart& part = m_snakeParts[i];

				bool isIntersect = false;

				for (const auto& it : barriers)
				{
					const AABB barrierBbox = it.pBarrier->GetBBox();

					if (i == 0) // Head
					{
						isIntersect = barrierBbox.IsIntersectBox(bbox);

						if (isIntersect)
						{
							it.pBarrier->OnCollideWithSnake(i);

							KillSnake(ESnakeDeathType::CollideWithBarriers);

							break;
						}
					}
					else
					{
						isIntersect = barrierBbox.IsIntersectBox(AABB(part.m_currentPosition, 1.f));

						if (isIntersect)
						{
							it.pBarrier->OnCollideWithSnake(i);
							break;
						}
					}
				}
			}
		}
	}

	void CSnake::SetStateFromSnapshot(const SSnakeSnapshot& snapshot)
	{
		Log("[Snake] Set state from snapshot");

		Reset();

		IncreaseSnake(snapshot.m_snakeSize - 1);

		for (int i = 0; i < GetSnakeSize(); i++)
		{
			m_snakeParts[i].MergeInfo(snapshot.m_snakeParts[i]);

			if (i == 0)
			{
				m_nextHeadPosition = m_snakeParts[i].m_nextPosition;
				m_lastHeadPosition = m_snakeParts[i].m_lastPosition;
				m_lastDirection = m_snakeParts[i].m_direction;
				m_newDirection = m_lastDirection;
				m_currentVelocity = m_snakeParts[i].m_currentVelocity;

				GetEntity()->SetPosRotScale(m_snakeParts[i].m_currentPosition, Quat::CreateRotationZ(DEG2RAD(m_snakeParts[i].m_currentRotation)), Vec3(1.f));
			}
		}

		m_statistic = snapshot.m_statistic;
	}

	void CSnake::SetNextRotationDeg(const float& deg)
	{
		m_snakeParts[0].m_nextRotation = deg;
	}

	void CSnake::SetLastRotationDeg(const float& deg)
	{
		m_snakeParts[0].m_lastRotation = deg;
	}

	void CSnake::OnReadyForGameplayOnServer()
	{
		CRY_ASSERT(gEnv->bServer, "This function should only be called on the server!");
		Revive(ZERO);

		// Notify listeners
		if (IsLocalClient())
		{
			for (const auto& it : m_listeners)
			{
				it->OnLocalPlayerSpawned();
			}
		}
	}

	void CSnake::RegisterListener(ILocalPlayerListener* pListener)
	{
		if (!stl::find(m_listeners, pListener))
			m_listeners.push_back(pListener);
	}

	void CSnake::UnregisterListener(ILocalPlayerListener* pListener)
	{
		stl::find_and_erase(m_listeners, pListener);
	}

	void CSnake::Reset()
	{
		m_currentVelocity = ZERO;
		m_lastVelocityGoal = ZERO;
		m_lastHeadPosition = ZERO;
		m_nextHeadPosition = ZERO;
		m_isSnakeCanChangeDirection = false;
		m_inputFlags.Clear();
		m_currentSnakeSpeed = 0.f;

		for (int i = 1; i < GetSnakeSize(); i++)
		{
			gEnv->pEntitySystem->RemoveEntity(m_snakeParts[i].m_partEID);

			m_snakeParts[i].Clear();
		}

		m_snakeSize = 1;

		m_statistic.Clear();

		if (CField* pField = GetField())
		{
			for (const auto& fruit : pField->GetFruits())
			{
				gEnv->pEntitySystem->RemoveEntity(fruit.id);
			}
		}
	}

	void CSnake::KillSnake(ESnakeDeathType type, bool withEffects, bool byReplay)
	{
#ifndef RELEASE
		m_snapshotBeforeDeath = GetSnapshot();
#endif
		GetEntity()->Hide(true);

		m_isAlive = false;

		m_statistic.deathCount++;

		Reset();

		if (withEffects)
		{
			switch (type)
			{
			case Snake::ESnakeDeathType::CollideWithBorders:
			{
				constexpr CryAudio::ControlId trigger = CryAudio::StringToId("Play_react_death_from_wall");

				PlaySound(trigger);
			}
			break;
			case Snake::ESnakeDeathType::CollideWithParts:
			{
				constexpr CryAudio::ControlId trigger = CryAudio::StringToId("Play_reatc_death_from_punch");

				PlaySound(trigger);
			}
			break;
			case Snake::ESnakeDeathType::CollideWithBarriers:
			{
				constexpr CryAudio::ControlId trigger = CryAudio::StringToId("Play_react_death_from_wall");

				PlaySound(trigger);
			}
			break;
			default:
				break;
			}

			// Gamepad feedback. TODO : More effects by m_events (death from wall/death from collide with part/etc)
			{
				IForceFeedbackSystem* pForceFeedback = gEnv->pGameFramework->GetIForceFeedbackSystem();
				ForceFeedbackFxId fxId = pForceFeedback->GetEffectIdByName("vehicleCollision");
				pForceFeedback->PlayForceFeedbackEffect(fxId, SForceFeedbackRuntimeParams(1.0f, 0.0f));
			}
		}

		bool autoRevive = g_pGameCVars->pl_autoReviveAfterDeath > 0 ? true : false;

		if (autoRevive)
		{
			LogDebug("Auto-revive snake...");
			Revive(ZERO);
		}
	}

	void CSnake::PlaySound(CryAudio::ControlId trigger)
	{
		IEntityAudioComponent* pAudioComponent = GetEntity()->GetOrCreateComponent<IEntityAudioComponent>();
		if (pAudioComponent)
		{
			pAudioComponent->ExecuteTrigger(trigger);
		}
	}

	void CSnake::ChangeCameraMode(ECameraMode mode)
	{
		IEntity* pCameraEntity = gEnv->pEntitySystem->GetEntity(m_cameraEntityID);
		CSnakeCamera* pCamera = pCameraEntity ? pCameraEntity->GetComponent<CSnakeCamera>() : nullptr;

		if (pCamera)
		{
			pCamera->ChangeMode(mode);

			if (mode == ECameraMode::FreeMode)
			{
				m_inputFlags.Clear();
			}
		}
	}

	ECameraMode CSnake::GetGameraMode() const
	{
		IEntity* pCameraEntity = gEnv->pEntitySystem->GetEntity(m_cameraEntityID);
		CSnakeCamera* pCamera = pCameraEntity ? pCameraEntity->GetComponent<CSnakeCamera>() : nullptr;

		if (pCamera)
		{
			return pCamera->GetMode();
		}

		return ECameraMode::Default;
	}

	CField* CSnake::GetField()
	{
		if (m_pField == nullptr)
		{
			auto pEntIT = gEnv->pEntitySystem->GetEntityIterator();
			while (IEntity* pEntity = pEntIT->Next())
			{
				if (pEntity->GetComponent<CField>())
				{
					m_pField = pEntity->GetComponent<CField>();
					break;
				}
			}
		}

		return m_pField;
	}

	EGameRulesType CSnake::GetGameMode()
	{
		if (CGameRules* pGameRules = GetGamerules())
		{
			return pGameRules->GetType();
		}

		return EGameRulesType::Invalid;
	}

	CGameRules* CSnake::GetGamerules()
	{
		if (m_pGameRules == nullptr)
		{
			auto pEntIT = gEnv->pEntitySystem->GetEntityIterator();
			while (IEntity* pEntity = pEntIT->Next())
			{
				if (pEntity->GetComponent<CGameRules>())
				{
					m_pGameRules = pEntity->GetComponent<CGameRules>();
					break;
				}
			}
		}

		return m_pGameRules;
	}

	Quat CSnake::GetRotationFromDirection(EMovementDirection direction)
	{
		Quat rotation = IDENTITY;

		switch (direction)
		{
		case EMovementDirection::Forward:
		{
			rotation.SetRotationZ(DEG2RAD(0));
			break;
		}
		case EMovementDirection::ToRight:
		{
			rotation.SetRotationZ(DEG2RAD(-90));
			break;
		}
		case EMovementDirection::Backward:
		{
			rotation.SetRotationZ(DEG2RAD(180));
			break;
		}
		case EMovementDirection::ToLeft:
		{
			rotation.SetRotationZ(DEG2RAD(90));
			break;
		}
		default:
			break;
		}

		return rotation;
	}

	float CSnake::GetDegreesFromDirection(EMovementDirection direction)
	{
		float degrees = 0.f;

		switch (direction)
		{
		case EMovementDirection::Forward:
		{
			degrees = 0.f;
			break;
		}
		case EMovementDirection::ToLeft:
		{
			degrees = 90.f;
			break;
		}
		case EMovementDirection::Backward:
		{
			degrees = 180.f;
			break;
		}
		case EMovementDirection::ToRight:
		{
			degrees = 270.f;
			break;
		}
		default:
			break;
		}

		return degrees;
	}

	Vec3 CSnake::GetVelocityFromDirection(EMovementDirection direction)
	{
		Vec3 velocityGoal = ZERO;

		switch (direction)
		{
		case EMovementDirection::Forward:
			velocityGoal = Vec3(0.f, 1.f, 0.f);
			break;
		case EMovementDirection::ToRight:
			velocityGoal = Vec3(1.f, 0.f, 0.f);
			break;
		case EMovementDirection::Backward:
			velocityGoal = Vec3(0.f, -1.f, 0.f);
			break;
		case EMovementDirection::ToLeft:
			velocityGoal = Vec3(-1.f, 0, 0.f);
			break;
		default:
			break;
		}

		return velocityGoal;
	}

	SCoordsOnField CSnake::GetUpdatedCoordsByDirection(const SCoordsOnField& currentCoords, EMovementDirection direction)
	{
		SCoordsOnField coords = currentCoords;

		switch (direction)
		{
		case Snake::EMovementDirection::Forward:
			coords.y++;
			break;
		case Snake::EMovementDirection::Backward:
			coords.y--;
			break;
		case Snake::EMovementDirection::ToLeft:
			coords.x--;
			break;
		case Snake::EMovementDirection::ToRight:
			coords.x++;
			break;
		default:
			break;
		}

		return coords;
	}

	SFruitObject CSnake::GetClosestFruit()
	{
		CField* pField = GetField();

		const std::vector<SFruitObject>& fruits = pField->GetFruits();

		SFruitObject fruit;
		float distToFruit = 100.f;

		// Get best fruit
		for (const auto& it : fruits)
		{
			const float dist = it.worldPos.GetDistance(GetEntity()->GetWorldPos());

			if (dist < distToFruit)
			{
				distToFruit = dist;
				fruit = it;
			}
		}

		return fruit;
	}

	void CSnake::CheckGameMode()
	{
		CGameRules* pGameRules = GetGamerules();

		if (pGameRules)
		{
			switch (pGameRules->GetType())
			{
			case EGameRulesType::Default:
				m_isClassicMode = true;
				break;
			default:
				break;
			}
		}
	}

	bool CSnake::IsCanChangeDirection(const EMovementDirection& newDir)
	{
		Vec3 velocity = ZERO;

		switch (newDir)
		{
		case EMovementDirection::ToLeft:
		{
			if (m_lastDirection == EMovementDirection::ToRight)
			{
				return false;
			}

			velocity.x -= g_pGameCVars->g_meters_per_step;
		}
		break;
		case EMovementDirection::ToRight:
		{
			if (m_lastDirection == EMovementDirection::ToLeft)
			{
				return false;
			}

			velocity.x += g_pGameCVars->g_meters_per_step;
			break;
		}
		case EMovementDirection::Backward:
		{
			if (m_lastDirection == EMovementDirection::Forward)
			{
				return false;
			}

			velocity.y -= g_pGameCVars->g_meters_per_step;
			break;
		}
		case EMovementDirection::Forward:
		{
			if (m_lastDirection == EMovementDirection::Backward)
			{
				return false;
			}

			velocity.y += g_pGameCVars->g_meters_per_step;
			break;
		}
		default:
			break;
		}

		const Vec3 nextPos = m_nextHeadPosition + velocity;

		const SSnakePart& nextPart = m_snakeParts[1];

		constexpr float minDistance = 0.2f;

		{
			if (nextPart.m_nextPosition.GetDistance(nextPos) <= minDistance
				|| nextPart.m_lastPosition.GetDistance(nextPos) <= minDistance)
			{
				return false;
			}
		}

		return true;
	}

	void CSnake::IncreaseSnake(int count)
	{
		for (int i = 0; i < count; i++)
		{
			SSnakePart part;
			part.m_currentPosition = m_snakeParts[GetSnakeSize() - 1].m_lastPosition;
			part.m_nextPosition = m_snakeParts[GetSnakeSize() - 1].m_lastPosition;
			part.m_nextRotation = m_snakeParts[GetSnakeSize() - 1].m_lastRotation;
			part.m_lastRotation = part.m_currentRotation = part.m_nextRotation;

			{
				SEntitySpawnParams params;
				params.pClass = gEnv->pEntitySystem->GetClassRegistry()->GetDefaultClass();
				params.vPosition = part.m_currentPosition;

				IEntity* pSnakePart = gEnv->pEntitySystem->SpawnEntity(params);
				pSnakePart->CreateComponent<CSnakePart>();

				part.m_partEntity = pSnakePart;
				part.m_partEID = pSnakePart->GetId();
			}

			m_snakeParts[GetSnakeSize()] = part;
			m_snakeSize++;
		}
	}

	void CSnake::ReduceSnake(int count, ESnakeDeathType type)
	{
		for (int i = 0; i < count; i++)
		{
			SSnakePart& lastPart = m_snakeParts[GetSnakeSize() - 1];

			{
				gEnv->pEntitySystem->RemoveEntity(lastPart.m_partEID);
			}

			{
				lastPart.Clear();
				m_snakeSize--;
			}

			if (GetSnakeSize() < g_pGameCVars->pl_default_size - 1)
			{
				KillSnake(type);
				break;
			}
		}
	}

	SSnakeSnapshot CSnake::GetSnapshot()
	{
		SSnakeSnapshot snapshot;

		for (int i = 0; i < GetSnakeSize(); i++)
		{
			snapshot.m_snakeParts[i] = m_snakeParts[i];
		}

		snapshot.m_snakeSize = GetSnakeSize();
		snapshot.m_statistic = m_statistic;

		return snapshot;
	}

	void CSnake::Revive(const Matrix34& transform)
	{
		Reset();

		m_isRequstReviveOnNextUpdate = false;

		GetEntity()->Hide(false);

		CField* pField = GetField();

		bool executeSpawnEvent = false;

		if (transform.IsZero() || transform.IsIdentity())
		{
			if (pField)
			{
				const SCoordsOnField coords = pField->SpawnSnake(this);

				if (coords.IsValid())
				{
					const SCoordsOnField lastHeadPos = pField->GetSpawnPointCoords(GetLastHeadPosition());
					const SCoordsOnField nextHeadPos = pField->GetSpawnPointCoords(GetNextHeadPosition());

					m_snakeParts[0].m_nextPosCoords = nextHeadPos;
					m_snakeParts[0].m_lastPosCoords = lastHeadPos;

					m_isAlive = true;
					m_isInputBlocked = true;
					executeSpawnEvent = true;
				}
				else
				{
					m_isRequstReviveOnNextUpdate = true;
				}
			}
		}
		else
		{
			m_isAlive = true;
			m_pEntity->SetWorldTM(transform);
			executeSpawnEvent = true;
		}

		// Notify listeners
		if (IsLocalClient() && executeSpawnEvent)
		{
			for (const auto& it : m_listeners)
			{
				it->OnLocalPlayerRevived();
			}
		}
	}

	void CSnake::HandleInputFlagChange(EInputFlag flags, EActionActivationMode activationMode)
	{
		if (GetGameraMode() != ECameraMode::FreeMode)
		{
			if (activationMode == eAAM_OnPress)
			{
				if (!m_inputFlags.Check(flags))
				{
					m_inputFlags.Clear();
					m_inputFlags.Add(flags);
				}
			}
		}
		else
		{
			if (activationMode == eAAM_OnPress)
			{
				m_inputFlags.Add(flags);
			}
			else if (activationMode == eAAM_OnRelease)
			{
				m_inputFlags.Remove(flags);
			}
		}
	}

	namespace
	{
		static void RegisterPlayerComponent(Schematyc::IEnvRegistrar& registrar)
		{
			Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
			{
				scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(CSnake));
			}
		}

		CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterPlayerComponent);
	}
}