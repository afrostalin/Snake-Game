// Copyright (C) 2021 Space Raccoon Game Studio. All rights reserved. Contacts: <business@space-raccoon.com>
// Created by AfroStalin

#pragma once

#include <CryEntitySystem/IEntityComponent.h>
#include <CryEntitySystem/IEntity.h>
#include <CryMath/Cry_Camera.h>
#include <DefaultComponents/Cameras/CameraComponent.h>
#include <DefaultComponents/Audio/ListenerComponent.h>

namespace Snake
{
	enum class ECameraMode : uint8
	{
		Default,
		Top,
		FreeMode
	};

	class CSnakeCamera final : public IEntityComponent
	{
	public:
		CSnakeCamera() = default;
		virtual ~CSnakeCamera() {}

		static void ReflectType(Schematyc::CTypeDesc<CSnakeCamera>& desc)
		{
			desc.SetGUID("{E30E2AF0-3D61-4091-B22C-CA37B3034C51}"_cry_guid);
		}

		// IEntityComponent
		virtual void   Initialize() override;
		virtual Cry::Entity::EventFlags GetEventMask() const override;
		virtual void   ProcessEvent(const SEntityEvent& event) override;
		// ~IEntityComponent
	public:
		void        ChangeMode(ECameraMode newMode) { m_mode = newMode; }
		ECameraMode GetMode() const { return m_mode; }
	private:
		void        Update(float frameTime);
	private:
		ECameraMode                               m_mode = ECameraMode::Default;
		Cry::DefaultComponents::CCameraComponent* m_pCameraComponent = nullptr;
		Matrix34                                  m_lastWorldTM = IDENTITY;
	};
}