// Copyright (C) 2021 Space Raccoon Game Studio. All rights reserved. Contacts: <business@space-raccoon.com>
// Created by AfroStalin

//
//                    _oo0oo_
//                   o8888888o
//                   88" . "88
//                   (| -_- |)
//                   0\  =  /0
//                 ___/`---'\___
//               .' \\|     |// '.
//              / \\|||  :  |||// \
//             / _||||| -:- |||||- \
//            |   | \\\  -  /// |   |
//            | \_|  ''\---/''  |_/ |
//            \  .-\__  '-'  ___/-. /
//          ___'. .'  /--.--\  `. .'___
//       ."" '<  `.___\_<|>_/___.' >' "".
//      | | :  `- \`.;`\ _ /`;.`/ - ` : | |
//      \  \ `_.   \_ __\ /__ _/   .-` /  /
//  =====`-.____`.___ \_____/___.-`___.-'=====
//                    `=---='


#include "StdAfx.h"
#include "Field.h"
#include "Fruit.h"
#include "GameRules.h"
#include "Snake.h"

#include <CryMath/Random.h>
#include <CryRenderer/IRenderAuxGeom.h>

namespace Snake
{
	namespace
	{
		static void RegisterFieldComponent(Schematyc::IEnvRegistrar& registrar)
		{
			Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
			{
				scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(CField));
			}
		}

		CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterFieldComponent);
	}

	void CField::Initialize()
	{
	}

	Cry::Entity::EventFlags CField::GetEventMask() const
	{
		if (gEnv->IsEditor())
		{
			return Cry::Entity::EEvent::Update;
		}
		else
		{
#ifdef RELEASE
			return Cry::Entity::EventFlags();
#else
			return Cry::Entity::EEvent::Update;
#endif
		}
	}

	void CField::ProcessEvent(const SEntityEvent& event)
	{
#ifndef RELEASE
		if (event.event == Cry::Entity::EEvent::Update)
		{
			bool isAlive = false;

			IEntity* pLocalPlayerEntity = gEnv->pEntitySystem->GetEntity(LOCAL_PLAYER_ENTITY_ID);
			CSnake* pSnake = pLocalPlayerEntity ? pLocalPlayerEntity->GetComponent<CSnake>() : nullptr;
			if (pLocalPlayerEntity)
			{
				isAlive = pSnake->IsAlive();
			}

			if (!isAlive && gEnv->IsEditor())
			{
				CalculateSpawnPoints();
				CalculateCacheObjects();
			}

			const ColorB _white = ColorB(255, 255, 255);
			const ColorB _red = ColorB(255, 0, 0);
			const ColorB _green = ColorB(0, 255, 0);


			if ((gEnv->IsEditor() && m_isDebugField) || g_pGameCVars->dg_field)
			{
				gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(m_cornerPoints[0], _white, m_cornerPoints[1], _white, 1.f);
				gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(m_cornerPoints[1], _white, m_cornerPoints[2], _white, 1.f);
				gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(m_cornerPoints[2], _white, m_cornerPoints[3], _white, 1.f);
				gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(m_cornerPoints[3], _white, m_cornerPoints[0], _white, 1.f);
			}

			if (g_pGameCVars->dg_snakeDeath && pSnake)
			{
				const SSnakeSnapshot& snapshot = pSnake->GetSnapshotBeforeDeath();

				for (int i = 0; i < snapshot.m_snakeSize; i++)
				{
					const SSnakePart& part = snapshot.m_snakeParts[i];
					
					ColorB _color = _white;

					if (i == 0) // Head
						_color = _green;


					// Draw pos
					gEnv->pRenderer->GetIRenderAuxGeom()->DrawAABB(AABB(part.m_currentPosition, 1.f), false, _color, EBoundingBoxDrawStyle::eBBD_Faceted);

					// Draw velocity
					gEnv->pRenderer->GetIRenderAuxGeom()->DrawCone(part.m_currentPosition + Vec3(0.f, 0.f, 1.5f), part.m_currentVelocity, 0.2f, 1.0f, ColorB(0, 255, 0));
				}
			}

			if ((gEnv->IsEditor() && m_isDebugBorders) || g_pGameCVars->dg_borders)
			{
				std::vector<Vec3> cornerPoints;
				GetCornerPoints(cornerPoints);

				// We need add vertical offset for points, because snake have additional offset by Z 
				for (auto&& it : cornerPoints)
				{
					it += Vec3(0.f, 0.f, g_pGameCVars->g_snakeSpawnOffsetZ / 2.f);
				}

				Vec3 borderHeightOffset = Vec3(0.f, 0.f, 5.f);

				gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(cornerPoints[0], _red, cornerPoints[1], _red);
				gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(cornerPoints[0], _red, cornerPoints[0] + borderHeightOffset, _red);
				gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(cornerPoints[0] + borderHeightOffset, _red, cornerPoints[1] + borderHeightOffset, _red);
				gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(cornerPoints[1] + borderHeightOffset, _red, cornerPoints[1], _red);

				gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(cornerPoints[1], _red, cornerPoints[2], _red);
				gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(cornerPoints[1] + borderHeightOffset, _red, cornerPoints[2] + borderHeightOffset, _red);
				gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(cornerPoints[2] + borderHeightOffset, _red, cornerPoints[2], _red);

				gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(cornerPoints[2], _red, cornerPoints[3], _red);
				gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(cornerPoints[2] + borderHeightOffset, _red, cornerPoints[3] + borderHeightOffset, _red);
				gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(cornerPoints[3] + borderHeightOffset, _red, cornerPoints[3], _red);

				gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(cornerPoints[3], _red, cornerPoints[0], _red);
				gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(cornerPoints[3] + borderHeightOffset, _red, cornerPoints[0] + borderHeightOffset, _red);
			}

			const float snakeSpeed = g_pGameCVars->g_meters_per_step;

			if ((gEnv->IsEditor() && m_isDebugField) || g_pGameCVars->dg_field)
			{
				{
					const float lenght = m_cornerPoints[0].y - m_cornerPoints[1].y;
					int sectors = int(lenght / snakeSpeed);

					if (sectors < 0)
						sectors *= -1;

					float lastYPos = m_cornerPoints[1].y;

					for (int i = 0; i < sectors; i++)
					{
						float pos = lastYPos - snakeSpeed;

						Vec3 start = Vec3(m_cornerPoints[2].x, pos, m_cornerPoints[2].z);

						Vec3 end = Vec3(m_cornerPoints[0].x, pos, m_cornerPoints[0].z);


						gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(start, _white, end, _white, 1.f);

						lastYPos = pos;
					}
				}

				{
					const float lenght = m_cornerPoints[1].x - m_cornerPoints[2].x;
					int sectors = int(lenght / snakeSpeed);

					if (sectors < 0)
						sectors *= -1;

					float lastXPos = m_cornerPoints[1].x;

					for (int i = 0; i < sectors; i++)
					{
						float pos = lastXPos + snakeSpeed;

						Vec3 start = Vec3(pos, m_cornerPoints[1].y, m_cornerPoints[1].z);
						Vec3 end = Vec3(pos, m_cornerPoints[3].y, m_cornerPoints[3].z);

						gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(start, _white, end, _white, 1.f);

						lastXPos = pos;
					}
				}
			}

			if ((gEnv->IsEditor() && m_isDebugSpawnPoints) || g_pGameCVars->dg_spawnPoints || (gEnv->IsEditor() && m_isDebugAvailableSpawnPoints) || g_pGameCVars->dg_availableSpawnPoints)
			{
				for (int i = 0; i < GetFieldWidth(); i++)
				{
					for (int j = 0; j < GetFieldHeight(); j++)
					{
						bool isAvailable = true;

						if(m_isDebugAvailableSpawnPoints || g_pGameCVars->dg_availableSpawnPoints)
						{
							if (CheckIntersectWithBarriers(m_spawnPoints[array_index(i,j)])
								|| CheckIntersectWithSnakeParts(m_spawnPoints[array_index(i,j)], pSnake)
								|| CheckIntersectWithFruits(m_spawnPoints[array_index(i,j)]))
							{
								isAvailable = false;
							}
						}

						if (isAvailable)
						{
							gEnv->pRenderer->GetIRenderAuxGeom()->DrawSphere(m_spawnPoints[array_index(i,j)].worldPos, 0.2f, _white);
						}
						else
						{
							gEnv->pRenderer->GetIRenderAuxGeom()->DrawSphere(m_spawnPoints[array_index(i,j)].worldPos, 0.2f, _red);
							gEnv->pRenderer->GetIRenderAuxGeom()->DrawAABB(AABB(m_spawnPoints[array_index(i,j)].worldPos, 1.f), false, _red, EBoundingBoxDrawStyle::eBBD_Faceted);
						}
					}
				}
			}

			if ((gEnv->IsEditor() && m_isDebugBarriers) || g_pGameCVars->dg_barriers)
			{
				for (const auto& it : GetBarriers())
				{
					if (it.pBarrier)
					{
						const AABB bbox = it.pBarrier->GetBBox();

						gEnv->pRenderer->GetIRenderAuxGeom()->DrawAABB(bbox, false, ColorB(255, 0, 0), EBoundingBoxDrawStyle::eBBD_Faceted);
					}
				}
			}

			if (g_pGameCVars->dg_snakeSpawn)
			{
				gEnv->pRenderer->GetIRenderAuxGeom()->DrawSphere(m_lastSpawnPointForSnake, 0.2f, ColorB(255, 0, 255));
				gEnv->pRenderer->GetIRenderAuxGeom()->DrawSphere(m_lastNextHeadPosForSnake, 0.2f, ColorB(0, 255, 0));

				for (const auto& it : m_lastPointsInDirectionForSnake)
				{
					gEnv->pRenderer->GetIRenderAuxGeom()->DrawSphere(it.worldPos, 0.2f, ColorB(255, 0, 0));
				}
			}
		}
#endif
	}

	void CField::OnFruitRemoved(EntityId id)
	{
		for (auto it = m_fruits.begin(); it != m_fruits.end(); ++it)
		{
			if (it->id == id)
			{
				m_spawnPoints[array_index(it->coords.x, it->coords.y)].isFruit = false;
				m_fruits.erase(it);
				break;
			}
		}
	}

	SCoordsOnField CField::SpawnSnake(CSnake* pSnake)
	{
		CRY_PROFILE_FUNCTION(EProfiledSubsystem::PROFILE_GAME);

		if (!isSpawnPointsCalculated)
		{
			CalculateSpawnPoints();
		}

		CalculateCacheObjects();

		SCoordsOnField coords;
		coords.x = -1;
		coords.y = -1;

		if (pSnake)
		{
			Matrix34 m_tm = IDENTITY;

			const int x = cry_random<int>(0, GetFieldWidth() - 1);
			const int y = cry_random<int>(0, GetFieldHeight() - 1);

			if (x < 0 || y < 0)
			{
				LogError("Can't spawn snake - field broken!");
				return coords;
			}

			const SSpawnPoint& point = m_spawnPoints[array_index(x, y)];
			const Vec3 spawnOffset = GetSnakeSpawnOffset();

			const float meteresPerStep = g_pGameCVars->g_meters_per_step;

			const EMovementDirection& lastDirection = pSnake->GetLastDirection();

			int randomDirection = 0;

			if (lastDirection == EMovementDirection::Forward || lastDirection == EMovementDirection::Backward)
			{
				randomDirection = cry_random<int>(static_cast<int>(EMovementDirection::ToLeft), static_cast<int>(EMovementDirection::ToRight));
			}
			else if (lastDirection == EMovementDirection::ToLeft || lastDirection == EMovementDirection::ToRight)
			{
				randomDirection = cry_random<int>(static_cast<int>(EMovementDirection::Forward), static_cast<int>(EMovementDirection::Backward));
			}

			const EMovementDirection m_newDirection = EMovementDirection(randomDirection);

			Vec3 velocityGoal = ZERO;

			switch (m_newDirection)
			{
			case EMovementDirection::Forward:
				velocityGoal.y += meteresPerStep;
				break;
			case EMovementDirection::Backward:
				velocityGoal.y -= meteresPerStep;
				break;
			case EMovementDirection::ToLeft:
				velocityGoal.x -= meteresPerStep;
				break;
			case EMovementDirection::ToRight:
				velocityGoal.x += meteresPerStep;
				break;
			default:
				break;
			}

			m_lastSpawnPointForSnake = m_tm.GetTranslation();
			m_lastNextHeadPosForSnake = m_lastSpawnPointForSnake + velocityGoal;

			Vec3 snakeDirection = m_lastSpawnPointForSnake - m_lastNextHeadPosForSnake;
			snakeDirection.Normalize();

			constexpr int pointsInDirectionCount = 5;

			m_lastPointsInDirectionForSnake = GetSpawnPointsInDirection(point.coords, snakeDirection, pointsInDirectionCount);

			if (m_lastPointsInDirectionForSnake.size() < pointsInDirectionCount)
			{
				return coords;
			}

			m_lastPointsInInvertedDirectionForSnake = GetSpawnPointsInDirection(point.coords, snakeDirection * -1.f, g_pGameCVars->pl_default_size - 1);

			if (m_lastPointsInInvertedDirectionForSnake.size() < g_pGameCVars->pl_default_size - 1)
			{
				return coords;
			}

			// Check intersect with barriers
			for (const auto& dir_point : m_lastPointsInDirectionForSnake)
			{
				if (CheckIntersectWithBarriers(dir_point))
					return coords;
			}

			if (m_tm.IsValid())
			{
				const Vec3 newHeadPosition = point.worldPos + spawnOffset + velocityGoal;

				m_tm.SetTranslation(point.worldPos + spawnOffset - velocityGoal);

				const float rotation = pSnake->GetDegreesFromDirection(m_newDirection);

				pSnake->GetEntity()->SetWorldTM(m_tm);
				pSnake->SetLastHeadPosition(newHeadPosition - velocityGoal);
				pSnake->SetNextHeadPosition(newHeadPosition);
				pSnake->SetNextRotationDeg(rotation);
				pSnake->SetLastRotationDeg(rotation);
				pSnake->SetNewDirection(m_newDirection);

				return point.coords;
			}
			else
			{
				LogError("[Field] Can't spawn snake on field - check spawn points TM!");
				return coords;
			}
		}

		return coords;
	}

	SFruitSpawnInfo CField::SpawnFruit(CSnake* pSnake)
	{
		CRY_PROFILE_FUNCTION(EProfiledSubsystem::PROFILE_GAME);

		SFruitSpawnInfo info;
		info.coords = SCoordsOnField(-1);
		info.id = INVALID_ENTITYID;

		if (!isSpawnPointsCalculated)
		{
			CalculateSpawnPoints();
		}
		
		if (!pSnake)
			return info;

		float distanceToSpawn = g_pGameCVars->g_fruitSpawnDistance;

		std::vector<SSpawnPoint> spawnsForFruit;

		const Vec3 snakeHeadPos = pSnake->GetNextHeadPosition();

		for (int i = 0; i < GetFieldWidth(); i++)
		{
			for (int j = 0; j < GetFieldHeight(); j++)
			{
				const SSpawnPoint& spawnPoint = m_spawnPoints[array_index(i,j)];
				const Vec2 spawnPointPos2D = Vec2(spawnPoint.worldPos.x, spawnPoint.worldPos.y);

				if (spawnPointPos2D.GetDistance(Vec2(snakeHeadPos.x, snakeHeadPos.y)) <= distanceToSpawn)
					spawnsForFruit.push_back(m_spawnPoints[array_index(i,j)]);
			}
		}

		for (auto it = spawnsForFruit.begin(); it != spawnsForFruit.end();)
		{
			bool found = false;

			// Check intersect with snake parts		
			if (CheckIntersectWithSnakeParts(*it, pSnake))
			{
				found = true;
			}

			// Check intersect with barriers
			if (CheckIntersectWithBarriers(*it))
			{
				found = true;
			}

			// Check intersect with other fruits
			if (CheckIntersectWithFruits(*it))
			{
				found = true;
			}

			if (found)
				it = spawnsForFruit.erase(it);
			else
				it++;
		}

		const int index = cry_random<int>(0, spawnsForFruit.size() - 1);

		if (!spawnsForFruit.empty() && index < spawnsForFruit.size())
		{
			SEntitySpawnParams params;
			params.pClass = gEnv->pEntitySystem->GetClassRegistry()->GetDefaultClass();
			params.vPosition = spawnsForFruit[index].worldPos + GetFruitSpawnOffset();

			IEntity* pEntity = gEnv->pEntitySystem->SpawnEntity(params);
			CFruit* pFruit = pEntity->CreateComponent<CFruit>();

			info.id = pEntity->GetId();
			info.coords = spawnsForFruit[index].coords;

			SFruitObject object;
			object.coords = info.coords;
			object.id = info.id;
			object.worldPos = params.vPosition;
			object.pFruit = pFruit;

			AddFruit(object);
		}

		return info;
	}

	bool CField::GetCornerPoints(std::vector<Vec3>& result)
	{
		result.clear();

		for (int i = 0; i < CRY_ARRAY_COUNT(m_cornerPoints); i++)
		{
			result.push_back(m_cornerPoints[i]);
		}

		return result.size() == CRY_ARRAY_COUNT(m_cornerPoints);
	}

	std::vector<SSpawnPoint> CField::GetSpawnPointsInDirection(const SCoordsOnField& coords, const Vec3& direction, int count)
	{
		std::vector<SSpawnPoint> result;

		if (!coords.IsValid())
			return result;

		int offsetX = 0;
		int offsetY = 0;

		if (direction.x > 0.f)
			offsetX = -1;
		else if (direction.x < 0.f)
			offsetX = 1;
		else if (direction.y > 0.f)
			offsetY = -1;
		else if (direction.y < 0.f)
			offsetY = 1;


		if (offsetX != 0)
		{
			int currentXIndex = coords.x;

			for (int i = 0; i < count; i++)
			{
				int newIndex = currentXIndex += offsetX;

				if (newIndex >= 0 && newIndex < m_fieldWidth)
				{
					result.push_back(GetSpawnPointByCoords(SCoordsOnField(newIndex, coords.y)));
				}
			}
		}
		else if (offsetY != 0)
		{
			int currentYIndex = coords.y;

			for (int i = 0; i < count; i++)
			{
				int newIndex = currentYIndex += offsetY;

				if (newIndex >= 0 && newIndex < m_fieldHeight)
				{
					result.push_back(GetSpawnPointByCoords(SCoordsOnField(coords.x, newIndex)));
				}
			}
		}

		return result;
	}

	std::vector<SSpawnPoint> CField::GetSpawnPointsInRadius(const SCoordsOnField& coords, const int& radius)
	{
		std::vector<SSpawnPoint> result;
		
		if (radius < 1 || !coords.IsValid())
			return result;

		for (int i = 0; i < radius; i++)
		{
			// x +
			{
				SCoordsOnField temp = coords;
				temp.x += i + 1;

				if (temp.IsValid())
				{
					result.push_back(GetSpawnPointByCoords(temp));
				}

				// y -
				{
					for (int j = 0; j < radius; j++)
					{
						SCoordsOnField temp_temp = temp;
						temp_temp.y -= j + 1;

						if (temp_temp.IsValid())
						{
							result.push_back(GetSpawnPointByCoords(temp_temp));
						}
					}
				}
			}

			// x-
			{
				SCoordsOnField temp = coords;
				temp.x -= i + 1;

				if (temp.IsValid())
				{
					result.push_back(GetSpawnPointByCoords(temp));
				}

				// y +
				{
					for (int j = 0; j < radius; j++)
					{
						SCoordsOnField temp_temp = temp;
						temp_temp.y += j + 1;

						if (temp_temp.IsValid())
						{
							result.push_back(GetSpawnPointByCoords(temp_temp));
						}
					}
				}
			}

			// y +
			{
				SCoordsOnField temp = coords;
				temp.y += i + 1;

				if (temp.IsValid())
				{
					result.push_back(GetSpawnPointByCoords(temp));
				}

				// x +
				{
					for (int j = 0; j < radius; j++)
					{
						SCoordsOnField temp_temp = temp;
						temp_temp.x += j + 1;

						if (temp_temp.IsValid())
						{
							result.push_back(GetSpawnPointByCoords(temp_temp));
						}
					}
				}
			}

			// y -
			{
				SCoordsOnField temp = coords;
				temp.y -= i + 1;

				if (temp.IsValid())
				{
					result.push_back(GetSpawnPointByCoords(temp));
				}

				// x -
				{
					for (int j = 0; j < radius; j++)
					{
						SCoordsOnField temp_temp = temp;
						temp_temp.x -= j + 1;

						if (temp_temp.IsValid())
						{
							result.push_back(GetSpawnPointByCoords(temp_temp));
						}
					}
				}
			}
		}


		return result;
	}

	SSpawnPoint CField::GetSpawnPointByCoords(const SCoordsOnField& coords) const
	{
		CRY_PROFILE_FUNCTION(EProfiledSubsystem::PROFILE_GAME);

		if ((coords.x < 0 || coords.x > GetFieldWidth()) || (coords.y < 0 || coords.y > GetFieldHeight()))
			return SSpawnPoint();

		return m_spawnPoints[array_index(coords.x, coords.y)];
	}

	SCoordsOnField CField::GetSpawnPointCoords(const Vec3& point)
	{
		CRY_PROFILE_FUNCTION(EProfiledSubsystem::PROFILE_GAME);

		constexpr float minDistance = 0.2f;
		const Vec2 point2D = Vec2(point.x, point.y);

		for (int i = 0; i < GetFieldWidth(); i++)
		{
			for (int j = 0; j < GetFieldHeight(); j++)
			{
				const SSpawnPoint& spawnPoint = m_spawnPoints[array_index(i,j)];
				const Vec2 spawnPointPos2D = Vec2(spawnPoint.worldPos.x, spawnPoint.worldPos.y);
		
				if (spawnPointPos2D.IsEqual(point2D) || point2D.GetDistance(spawnPointPos2D) <= minDistance)
					return spawnPoint.coords;
			}
		}

		return SCoordsOnField(-1);
	}

	bool CField::IsPointOnField(const Vec3& point)
	{
		constexpr float minDistance = 0.2f;

		for (int i = 0; i < GetFieldWidth(); i++)
		{
			for (int j = 0; j < GetFieldHeight(); j++)
			{
				const Vec2 spawnPointPos2D = Vec2(m_spawnPoints[array_index(i,j)].worldPos.x, m_spawnPoints[array_index(i,j)].worldPos.y);

				if (spawnPointPos2D == Vec2(point.x, point.y) || spawnPointPos2D.GetDistance(Vec2(point.x, point.y)) <= minDistance)
					return true;
			}
		}

		return false;
	}

	std::vector<SPointWeight> CField::GetFieldWeights(CSnake* pSnake)
	{
		CRY_PROFILE_FUNCTION(EProfiledSubsystem::PROFILE_GAME);

		std::vector<SPointWeight> result;

		for (int x = 0; x < GetFieldWidth(); x++)
		{
			for (int y = 0; y < GetFieldHeight(); y++)
			{
				const SCoordsOnField coord = SCoordsOnField(x, y);
				const SSpawnPoint& spawnPoint = m_spawnPoints[array_index(x,y)];

				int weight = 1;
				bool alredyIntersect = false;

				if (spawnPoint.isBarrier)
				{
					weight = 9;
					alredyIntersect = true;
				}

				if (!alredyIntersect && CheckIntersectWithSnakeParts(spawnPoint, pSnake))
				{
					weight = 9;
					alredyIntersect = true;
				}

				result.push_back(SPointWeight(coord, weight, spawnPoint.worldPos));
			}
		}

		return result;
	}

	Vec3 CField::GetClosestPoint(const Vec3& point, const float& distance)
	{
		Vec3 closestPoint = ZERO;
		float minDistance = distance;

		for (int i = 0; i < GetFieldWidth(); i++)
		{
			for (int j = 0; j < GetFieldHeight(); j++)
			{
				const Vec2 spawnPointPos2D = Vec2(m_spawnPoints[array_index(i,j)].worldPos.x, m_spawnPoints[array_index(i,j)].worldPos.y);

				const float dist = spawnPointPos2D.GetDistance(Vec2(point));

				if (dist <= minDistance)
				{
					closestPoint = m_spawnPoints[array_index(i,j)].worldPos;
					minDistance = distance;
				}
			}
		}

		return closestPoint;
	}

	Vec3 CField::GetSnakeSpawnOffset() const
	{
		return Vec3(0.f, 0.f, g_pGameCVars->g_snakeSpawnOffsetZ);
	}

	Vec3 CField::GetFruitSpawnOffset() const
	{
		return Vec3(0.f, 0.f, 0.7f);
	}

	std::vector<SFruitState> CField::GetFruitsStates()
	{
		std::vector<SFruitState> state;

		for (const auto& it : GetFruits())
		{
			if (it.pFruit)
			{
				state.push_back(it.pFruit->GetState());
			}
		}

		return state;
	}

	int CField::GetFruitsCount()
	{
		return GetFruits().size();
	}

	void CField::CalculateSpawnPoints()
	{
		for (IEntityLink* pLink = GetEntity()->GetEntityLinks(); pLink; pLink = pLink->next)
		{
			if (IEntity * pTarget = gEnv->pEntitySystem->GetEntity(pLink->entityId))
			{
				if (string(pTarget->GetName()) == "Point_A")
					m_cornerPoints[0] = pTarget->GetWorldPos();
				else if (string(pTarget->GetName()) == "Point_B")
					m_cornerPoints[1] = pTarget->GetWorldPos();
				else if (string(pTarget->GetName()) == "Point_C")
					m_cornerPoints[2] = pTarget->GetWorldPos();
				else if (string(pTarget->GetName()) == "Point_D")
					m_cornerPoints[3] = pTarget->GetWorldPos();
			}
		}

		const float metersPerStep = g_pGameCVars->g_meters_per_step;

		m_fieldHeight = 0;
		m_fieldWidth = 0;

		{
			const float lenght = m_cornerPoints[0].y - m_cornerPoints[1].y;
			int sectors = int(lenght / metersPerStep);

			if (sectors < 0)
				sectors *= -1;

			m_fieldHeight = sectors;
		}

		{
			const float lenght = m_cornerPoints[1].x - m_cornerPoints[2].x;
			int sectors = int(lenght / metersPerStep);

			if (sectors < 0)
				sectors *= -1;

			m_fieldWidth = sectors;
		}

		if (m_fieldWidth <= 0 || m_fieldHeight <= 0)
		{
			LogError("Can't calculate spawn points - field broken!");
			isSpawnPointsCalculated = false;
			return;
		}

		// Clear
		{
			for (int i = 0; i < GetFieldWidth(); i++)
			{
				for (int j = 0; j < GetFieldHeight(); j++)
				{
					m_spawnPoints[array_index(i,j)] = SSpawnPoint();
				}
			}

			isSpawnPointsCalculated = false;
		}

		{
			for (int i = 0; i < m_fieldWidth; i++)
			{
				const float offsetX = float(i * metersPerStep);

				for (int j = 0; j < m_fieldHeight; j++)
				{
					const float offestY = float(j * metersPerStep);

					SSpawnPoint spawnPoint;
					spawnPoint.coords = SCoordsOnField(i, j);
					spawnPoint.worldPos = Vec3(m_cornerPoints[0].x + (offsetX + (metersPerStep * 0.5f)), m_cornerPoints[0].y + offestY + (metersPerStep * 0.5f), m_cornerPoints[0].z);

					m_spawnPoints[array_index(i,j)] = spawnPoint;
				}
			}
		}

		isSpawnPointsCalculated = true;
	}

	void CField::CalculateCacheObjects()
	{
		m_barriers.clear();
		m_fruits.clear();

		auto pEntIT = gEnv->pEntitySystem->GetEntityIterator();
		while (IEntity* pEntity = pEntIT->Next())
		{
			const Vec3 objectPos = pEntity->GetWorldPos();

			if (CBarrier* pBarrier = pEntity->GetComponent<CBarrier>())
			{
				SBarrierObject object;
				object.coords = GetSpawnPointCoords(objectPos);
				object.id = pEntity->GetId();
				object.worldPos = objectPos;
				object.pBarrier = pBarrier;

				if (object.coords.IsValid())
				{
					m_spawnPoints[array_index(object.coords.x, object.coords.y)].isBarrier = true;
				}

				m_barriers.push_back(object);
			}
			else if (CFruit* pFruit = pEntity->GetComponent<CFruit>())
			{
				SFruitObject object;
				object.coords = GetSpawnPointCoords(objectPos);
				object.id = pEntity->GetId();
				object.worldPos = objectPos;
				object.pFruit = pFruit;

				if (object.coords.IsValid())
				{
					m_spawnPoints[array_index(object.coords.x,object.coords.y)].isFruit = true;
				}

				m_fruits.push_back(object);
			}
		}
	}

	void CField::AddFruit(const SFruitObject& fruit)
	{
		if (fruit.coords.IsValid())
		{
			m_spawnPoints[array_index(fruit.coords.x, fruit.coords.y)].isFruit = true;
		}

		m_fruits.push_back(fruit);
	}

	void CField::SetFruitsStates(const std::vector<SFruitState>& fruits)
	{
		for (const auto& it : fruits)
		{
			SEntitySpawnParams params;
			params.pClass = gEnv->pEntitySystem->GetClassRegistry()->GetDefaultClass();
			params.vPosition = it.m_currentPos;

			IEntity* pEntity = gEnv->pEntitySystem->SpawnEntity(params);
			CFruit* pFruit = pEntity->CreateComponent<CFruit>();

			pFruit->SetTimer(it.m_timer);

			SFruitObject object;
			object.coords = GetSpawnPointCoords(params.vPosition);
			object.id = pEntity->GetId();
			object.worldPos = params.vPosition;
			object.pFruit = pFruit;

			AddFruit(object);
		}
	}

	bool CField::CheckIntersectWithBarriers(const SSpawnPoint& point)
	{
		constexpr float minDistance = 0.2f;

		for (const auto& barrier : GetBarriers())
		{
			const Vec2 barrierPos2D = Vec2(barrier.worldPos.x, barrier.worldPos.y);

			if ((barrier.coords == point.coords) || (barrierPos2D.GetDistance(Vec2(point.worldPos.x, point.worldPos.y)) <= minDistance))
			{
				return true;
			}
		}

		return false;
	}

	bool CField::CheckIntersectWithSnakeParts(const SSpawnPoint& point, CSnake* pSnake)
	{
		if (!pSnake)
			return false;

		constexpr float minDistance = 0.2f;

		const SSnakePart* pParts = pSnake->GetSnakeParts();

		for (int i = 0; i < pSnake->GetSnakeSize(); i++)
		{
			const SSnakePart& part = pParts[i];
			const Vec2 partNextPos2D = Vec2(part.m_nextPosition.x, part.m_nextPosition.y);
			const Vec2 partLastPos2D = Vec2(part.m_lastPosition.x, part.m_lastPosition.y);

			if (partNextPos2D.GetDistance(Vec2(point.worldPos.x, point.worldPos.y)) <= minDistance
				|| partLastPos2D.GetDistance(point.worldPos) <= minDistance)
				return true;
		}

		return false;
	}

	bool CField::CheckIntersectWithFruits(const SSpawnPoint& point)
	{
		constexpr float minDistance = 0.2f;

		for (const auto& fruit : GetFruits())
		{
			const Vec2 fruitPos2D = Vec2(fruit.worldPos.x, fruit.worldPos.y);

			if ((fruit.coords == point.coords) || (fruitPos2D.GetDistance(Vec2(point.worldPos.x, point.worldPos.y)) <= minDistance))
			{
				return true;
			}
		}

		return false;
	}

	CGameRules* CField::GetGameRules()
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

	CSnake* CField::GetSnake()
	{
		IEntity* pSnakeEntity = gEnv->pEntitySystem->GetEntity(LOCAL_PLAYER_ENTITY_ID);
		CSnake* pSnake = pSnakeEntity ? pSnakeEntity->GetComponent<CSnake>() : nullptr;
		return pSnake;
	}
}