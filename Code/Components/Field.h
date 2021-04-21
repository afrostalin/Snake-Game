// Copyright (C) 2021 Space Raccoon Game Studio. All rights reserved. Contacts: <business@space-raccoon.com>
// Created by AfroStalin

#pragma once

#include <CryEntitySystem/IEntityComponent.h>
#include <CryEntitySystem/IEntity.h>

#include "Fruit.h"

namespace Snake
{
	constexpr int k_maxFieldWidth = 100;
	constexpr int k_maxFieldHeight = 100;
	constexpr int k_maxFieldSize = k_maxFieldWidth * k_maxFieldHeight;

	class CSnake;
	class CGameRules;
	class CBarrier;

	struct SCoordsOnField
	{
		SCoordsOnField() = default;
		SCoordsOnField(int8 _x, int8 _y) : x(_x), y(_y) {}
		explicit SCoordsOnField(int8 xy) : x(xy), y(xy) {}

		int8 x = -1;
		int8 y = -1;

		bool operator ==(const SCoordsOnField& rhs) const
		{
			return x == rhs.x && y == rhs.y;
		}

		bool operator !=(const SCoordsOnField& rhs) const
		{
			return !(*this == rhs);
		}

		bool IsValid() const
		{
			return x >= 0 && y >= 0;
		}
	};

	struct SSpawnPoint
	{
		SCoordsOnField coords = SCoordsOnField();
		Vec3           worldPos = ZERO;
		bool           isBarrier = false;
		bool           isFruit = false;
		bool           isSnakePart = false;
	};

	struct SPointWeight
	{
		SPointWeight() {}
		SPointWeight(const SCoordsOnField& _coord, const int & _weight, const Vec3 & _pos) : coord(_coord), weight(_weight), pos(_pos) {}

		SCoordsOnField coord = SCoordsOnField();
		int weight = 9;
		Vec3 pos = ZERO;
	};

	struct SFruitSpawnInfo
	{
		EntityId id = INVALID_ENTITYID;
		SCoordsOnField coords = SCoordsOnField();
	};

	struct SObjectOnField
	{
		SCoordsOnField coords = SCoordsOnField();
		Vec3           worldPos = ZERO;
		EntityId       id = INVALID_ENTITYID;
	};

	struct SBarrierObject : public SObjectOnField
	{
		CBarrier* pBarrier = nullptr;
	};

	struct SFruitObject : public SObjectOnField
	{
		CFruit* pFruit = nullptr;

#ifndef RELEASE
		bool m_isDrawBBox = false;
#endif
	};

	class CField final : public IEntityComponent
	{
	public:
		CField() = default;
		virtual ~CField() {}

		static void ReflectType(Schematyc::CTypeDesc<CField>& desc)
		{
			desc.SetGUID("{461670AF-FAA9-4158-A0F4-CD952A5424DE}"_cry_guid);
			desc.SetEditorCategory("Game");
			desc.SetLabel("Field");
			desc.SetDescription("");
			desc.SetComponentFlags({ IEntityComponent::EFlags::Transform, IEntityComponent::EFlags::Socket, IEntityComponent::EFlags::Attach, IEntityComponent::EFlags::NetNotReplicate });

			desc.AddMember(&CField::m_isDebugSpawnPoints, 'dbgs', "DebugSpawnPoints", "Debug spawn points", "Debug spawn points", false);
			desc.AddMember(&CField::m_isDebugField, 'dbgf', "DebugFieldLines", "Debug field lines", "Debug field lines", false);
			desc.AddMember(&CField::m_isDebugBarriers, 'dbgb', "DebugBarriers", "Debug barriers", "Debug barriers", false);
			desc.AddMember(&CField::m_isDebugBorders, 'dbbb', "DebugBorders", "Debug borders", "Debug borders", false);
			desc.AddMember(&CField::m_isDebugAvailableSpawnPoints, 'dbas', "DebugAvailableSpawns", "Debug available spawn points", "Debug available spawn points", false);
		}

		// IEntityComponent
		virtual void   Initialize() override;
		virtual Cry::Entity::EventFlags GetEventMask() const override;
		virtual void   ProcessEvent(const SEntityEvent& event) override;
		// ~IEntityComponent
	public:
		void OnFruitRemoved(EntityId id);
	public:
		SCoordsOnField                     SpawnSnake(CSnake* pSnake);
		SFruitSpawnInfo                    SpawnFruit(CSnake* pSnake);

		const SSpawnPoint*                 GetSpawnPoints() const { return m_spawnPoints.data(); }
		const std::vector<SBarrierObject>& GetBarriers() const { return m_barriers; }
		const std::vector<SFruitObject>&   GetFruits() const { return m_fruits; }
		std::vector<SFruitObject>&         GetFruitsAccess() { return m_fruits;}
		bool                               GetCornerPoints(std::vector<Vec3>& result);
		std::vector<SSpawnPoint>           GetSpawnPointsInDirection(const SCoordsOnField& coords, const Vec3& direction, int count);
		std::vector<SSpawnPoint>           GetSpawnPointsInRadius(const SCoordsOnField& coords, const int& radius);
		SSpawnPoint                        GetSpawnPointByCoords(const SCoordsOnField& coords) const;
		SCoordsOnField                     GetSpawnPointCoords(const Vec3& point);
		int                                GetFieldHeight() const { return m_fieldHeight; }
		int                                GetFieldWidth() const { return m_fieldWidth; }
		std::vector<SPointWeight>          GetFieldWeights(CSnake* pSnake);
		Vec3                               GetClosestPoint(const Vec3& point, const float& distance);
		Vec3                               GetSnakeSpawnOffset() const;
		Vec3                               GetFruitSpawnOffset() const;
		std::vector<SFruitState>           GetFruitsStates();
		int                                GetFruitsCount();

		bool                               IsPointOnField(const Vec3& point);
		bool                               IsSpawnPointsCalculated() const { return isSpawnPointsCalculated; }

		void                               CalculateSpawnPoints();
		void                               CalculateCacheObjects();

		void                               AddFruit(const SFruitObject& fruit);
		void                               SetFruitsStates(const std::vector<SFruitState>& fruits);

		static size_t                      array_index(int x, int y) { return x + k_maxFieldWidth * y; }
	private:
		bool                               CheckIntersectWithBarriers(const SSpawnPoint& point);
		bool                               CheckIntersectWithSnakeParts(const SSpawnPoint& point, CSnake* pSnake);
		bool                               CheckIntersectWithFruits(const SSpawnPoint& point);

		CGameRules*                        GetGameRules();
		CSnake*                            GetSnake();
	private:
		CGameRules*                 m_pGameRules = nullptr;
		Vec3                        m_cornerPoints[4] = { ZERO };
		std::array<SSpawnPoint, k_maxFieldSize> m_spawnPoints;

		Vec3                        m_lastSpawnPointForSnake = ZERO;
		Vec3                        m_lastNextHeadPosForSnake = ZERO;

		std::vector<SSpawnPoint>    m_lastPointsInDirectionForSnake;
		std::vector<SSpawnPoint>    m_lastPointsInInvertedDirectionForSnake;
								    
		int                         m_fieldHeight = 0;
		int                         m_fieldWidth = 0;
								    
		bool                        isSpawnPointsCalculated = false;

		std::vector<SBarrierObject> m_barriers;
		std::vector<SFruitObject>   m_fruits;
	protected:
		bool m_isDebugSpawnPoints = false;
		bool m_isDebugField = false;
		bool m_isDebugBarriers = false;
		bool m_isDebugBorders = false;
		bool m_isDebugAvailableSpawnPoints = false;
	};
}