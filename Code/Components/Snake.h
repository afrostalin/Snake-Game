// Copyright (C) 2021 Space Raccoon Game Studio. All rights reserved. Contacts: <business@space-raccoon.com>
// Created by AfroStalin

#pragma once

#include <CryEntitySystem/IEntityComponent.h>
#include <CryMath/Cry_Camera.h>
#include <CrySchematyc/Utils/EnumFlags.h>

#include <DefaultComponents/Physics/CharacterControllerComponent.h>
#include <DefaultComponents/Geometry/AdvancedAnimationComponent.h>
#include <DefaultComponents/Geometry/StaticMeshComponent.h>
#include <DefaultComponents/Input/InputComponent.h>

#include "Camera.h"
#include "Field.h"
#include "GameRules.h"

namespace Snake
{
	constexpr int maxSnakePartsCount = 2000;

	enum class EMovementDirection : uint8
	{
		Forward = 0,
		Backward,
		ToLeft,
		ToRight,
		Invalid
	};

	enum class ESnakeDeathType : uint8
	{
		Invalid = 0,
		CollideWithBorders,
		CollideWithParts,
		CollideWithBarriers
	};

	struct SSnakePart
	{
	public:
		SSnakePart() {}

		IEntity*    m_partEntity = nullptr;
		EntityId    m_partEID = INVALID_ENTITYID;

		Vec3        m_nextPosition = ZERO;
		Vec3        m_lastPosition = ZERO;
		Vec3        m_currentPosition = ZERO;
		Vec3        m_currentVelocity = ZERO;
		EMovementDirection m_direction = EMovementDirection::Invalid;
		EMovementDirection m_lastDirection = EMovementDirection::Invalid;
		float       m_scale = 1.0f;
		uint32      m_nHUDSilhouettesParam = 0;

		SCoordsOnField m_nextPosCoords = SCoordsOnField();
		SCoordsOnField m_lastPosCoords = SCoordsOnField();

		float       m_lastRotation = 0.f;
		float       m_nextRotation = 0.f;
		float       m_currentRotation = 0.f;

#ifndef RELEASE
		bool m_isDrawBbox = false;
		bool m_isDrawDirection = false;	
		bool m_isDrawNextPos = false;
		bool m_isDrawLastPos = false;
#endif
	public:
		void Clear()
		{
			m_partEntity = nullptr;
			m_partEID = INVALID_ENTITYID;

			m_nextPosition = ZERO;
			m_lastPosition = ZERO;
			m_currentPosition = ZERO;
			m_currentVelocity = ZERO;
			m_direction = EMovementDirection::Invalid;
			m_lastDirection = EMovementDirection::Invalid;
			m_scale = 1.f;
			m_nHUDSilhouettesParam = 0;

			m_nextPosCoords = SCoordsOnField();
			m_lastPosCoords = SCoordsOnField();

			m_lastRotation = 0.f;
			m_nextRotation = 0.f;
			m_currentRotation = 0.f;

#ifndef RELEASE
			m_isDrawBbox = false;
			m_isDrawDirection = false;
			m_isDrawNextPos = false;
			m_isDrawLastPos = false;
#endif
		}

		void MergeInfo(const SSnakePart& info)
		{
			m_nextPosition = info.m_nextPosition;
			m_lastPosition = info.m_lastPosition;
			m_currentPosition = info.m_currentPosition;
			m_currentVelocity = info.m_currentVelocity;
			m_direction = info.m_direction;
			m_lastDirection = info.m_lastDirection;
			m_scale = info.m_scale;

			m_nextPosCoords = info.m_nextPosCoords;
			m_lastPosCoords = info.m_lastPosCoords;

			m_lastRotation = info.m_lastRotation;
			m_nextRotation = info.m_nextRotation;
			m_currentRotation = info.m_currentRotation;
		}
	};

	struct SPlayerStatistic
	{
	public:
		int currentScore = 0;
		int bestScore = 0;
		int snakeSize = 0;
		int maxSnakeSize = 0;
		int harvestedApples = 0;
		int deathCount = 0;
		int pathSize = 0;
	public:
		void Clear()
		{
			currentScore = 0;
			snakeSize = 0;
			harvestedApples = 0;
		}
	};

	struct SSnakeSnapshot
	{
	public:
		SSnakePart       m_snakeParts[maxSnakePartsCount];
		int              m_snakeSize = 0;
		SPlayerStatistic m_statistic;
	public:
		void Clear()
		{
			m_statistic = SPlayerStatistic();

			for (int i = 0; i < m_snakeSize; i++)
			{
				m_snakeParts[i] = SSnakePart();
			}

			m_snakeSize = 0;
		}
	};

	struct ILocalPlayerListener
	{
		virtual void OnLocalPlayerSpawned() = 0;
		virtual void OnLocalPlayerRevived() = 0;
	};

	class CSnake final : public IEntityComponent
	{
	public:
		enum class EInputFlag : uint8
		{
			MoveLeft = 1 << 0,
			MoveRight = 1 << 1,
			MoveForward = 1 << 2,
			MoveBack = 1 << 3
		};
	public:
		CSnake() = default;
		virtual ~CSnake() = default;

		// IEntityComponent
		virtual void Initialize() override;
		virtual void OnShutDown() override;
		virtual Cry::Entity::EventFlags GetEventMask() const override;
		virtual void ProcessEvent(const SEntityEvent& event) override;
		// ~IEntityComponent

		static void ReflectType(Schematyc::CTypeDesc<CSnake>& desc)
		{
			desc.SetGUID("{63F4C0C6-32AF-4ACB-8FB0-57D45DD14725}"_cry_guid);
		}
	public:
		void                    OnReadyForGameplayOnServer();
		void                    InitializeLocalPlayer();
	public:
		void                    RegisterListener(ILocalPlayerListener* pListener);
		void                    UnregisterListener(ILocalPlayerListener* pListener);

		void                    SetNewDirection(EMovementDirection dir) { m_newDirection = dir; }
		void                    SetLastDirection(EMovementDirection dir) { m_lastDirection = dir; }
		void                    SetLastVelocityGoal(const Vec3& velocity) { m_lastVelocityGoal = ZERO; }
		void                    SetCanChangeDirection(bool can) { m_isSnakeCanChangeDirection = true; }
		void                    SetLastHeadPosition(const Vec3& pos) { m_lastHeadPosition = pos; }
		void                    SetNextHeadPosition(const Vec3& pos) { m_nextHeadPosition = pos; }
		void                    SetStateFromSnapshot(const SSnakeSnapshot& snapshot);
		void                    SetIsLoadedSnake(bool loaded) { m_isLoadedSnake = loaded; }
		void                    SetNextRotationDeg(const float& deg);
		void                    SetLastRotationDeg(const float& deg);
						       
		const EMovementDirection& GetLastDirection() const { return m_lastDirection; }
		const EMovementDirection& GetNewDirection() const { return m_newDirection; }
		const Vec3&             GetNextHeadPosition() const { return m_nextHeadPosition; }
		const Vec3&             GetLastHeadPosition() const { return m_lastHeadPosition;}
		const int&              GetSnakeSize() const { return m_snakeSize; }
		const SSnakePart*       GetSnakeParts() const { return m_snakeParts.data(); }
		std::array<SSnakePart, maxSnakePartsCount>& GetSnakePartsAccess() { return m_snakeParts; }
		const SPlayerStatistic& GetStatistic() const { return m_statistic; }
		const float&            GetStarvingTimer() const { return m_starvingTimer; }
		const Quat&             GetLookOrientation() const { return m_lookOrientation; }
		const CEnumFlags<EInputFlag>& GetInputFlags() const { return m_inputFlags; }
		SSnakeSnapshot          GetSnapshot();
		const SSnakeSnapshot&   GetSnapshotBeforeDeath() const { return m_snapshotBeforeDeath; }
#if REPLAY_SYSTEM
		const uint32&           GetReplayCounter() const { return m_replayCounter; }
#endif
		CField*                 GetField();
		EGameRulesType          GetGameMode();
		Quat                    GetRotationFromDirection(EMovementDirection direction);
		float                   GetDegreesFromDirection(EMovementDirection direction);

		bool                    IsLocalClient() const { return (m_pEntity->GetFlags() & ENTITY_FLAG_LOCAL_PLAYER) != 0; }
		bool                    IsAlive() const { return m_isAlive; }
		bool                    IsClassicMode() const { return m_isClassicMode; }
		bool                    IsCanChangeDirection(const EMovementDirection& newDir);
		bool                    IsSavedGame() const { return m_isLoadedSnake; }

		void                    IncreaseSnake(int count = 1);
		void                    ReduceSnake(int count = 1, ESnakeDeathType type = ESnakeDeathType::Invalid);
		void                    Revive(const Matrix34& transform);
		void                    Reset();
		void                    KillSnake(ESnakeDeathType type, bool withEffects = true, bool byReplay = false);
		ECameraMode             GetGameraMode() const;
		void                    ChangeCameraMode(ECameraMode mode);
	protected:
		void                    UpdateMovementRequest(float frameTime);
		void                    UpdateLookDirectionRequest(float frameTime);
		void                    UpdatePlayerStatistic(float frameTime);
		void                    UpdateAnimations(float frameTime);
		Quat                    UpdatePartRotation(SSnakePart& part, float frameTime);
			                    
		void                    CheckCollideWithBorders(float frameTime);
		void                    CheckCollideWithParts(float frameTime);
		void                    CheckCollideWithFruits(float frameTime);
		void                    CheckCollideWithBarriers(float frameTime);
		

		void                    HandleInputFlagChange(EInputFlag flags, EActionActivationMode activationMode);
		
		void                    PlaySound(CryAudio::ControlId trigger);		
	protected:
		CGameRules*             GetGamerules();
		Vec3                    GetVelocityFromDirection(EMovementDirection direction);
		SCoordsOnField          GetUpdatedCoordsByDirection(const SCoordsOnField& currentCoords, EMovementDirection direction);
		SFruitObject            GetClosestFruit();
		void                    CheckGameMode();
	protected:
		EMovementDirection      m_lastDirection = EMovementDirection::Forward;
		EMovementDirection      m_newDirection = EMovementDirection::Forward;
						        
		Vec3                    m_lastHeadPosition = ZERO;
		Vec3                    m_nextHeadPosition = ZERO;
		Vec3                    m_lastVelocityGoal = ZERO;
		Vec3                    m_currentVelocity = ZERO;
		Vec3                    m_currentPortalOut = ZERO;
		Vec3                    m_currentPortalDir = ZERO;

		Vec2                    m_mouseDeltaRotation = ZERO;

		Quat                    m_lookOrientation = IDENTITY;
		Quat                    m_eyesRotation = IDENTITY;

		ECameraMode             m_lastCameraMode = ECameraMode::Default;
						        
		bool                    m_isSnakeCanChangeDirection = false;		
		bool                    m_isRequstReviveOnNextUpdate = false;
		bool                    m_isInputBlocked = false;
		bool                    m_isAlive = false;
		bool                    m_isClassicMode = false;
		bool                    m_isLoadedSnake = false;


		Cry::DefaultComponents::CInputComponent*           m_pInputComponent = nullptr;
		Cry::Audio::DefaultComponents::CListenerComponent* m_pAudioListener = nullptr;
		CField*                                            m_pField = nullptr;	
		CGameRules*                                        m_pGameRules = nullptr;
		CEnumFlags<EInputFlag>                             m_inputFlags;
		std::vector<ILocalPlayerListener*>                 m_listeners;

		float                  m_currentSnakeSpeed = 0.f;
		int                    m_snakeSize = 1;

		SPlayerStatistic       m_statistic;
		std::array<SSnakePart, maxSnakePartsCount> m_snakeParts;
		EntityId               m_cameraEntityID = INVALID_ENTITYID;

		float                  m_lastOnlineStatUpdateTime = 0.f;
		float                  m_starvingTimer = 0.f;	

#if REPLAY_SYSTEM
		uint32                 m_replayCounter = 0;
#endif
		uint32                 m_leftEyeID = 0;
		uint32                 m_rightEyeID = 0;
		float                  m_closeMouthDelay = 0.f;
	private:
		SSnakeSnapshot         m_snapshotBeforeDeath;
	};		                   
}