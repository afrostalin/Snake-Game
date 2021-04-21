// Copyright (C) 2020 Space Raccoon Game Studio. All rights reserved. Contacts: <business@space-raccoon.com>
// Created by AfroStalin

#pragma once

#include <CryEntitySystem/IEntityComponent.h>
#include <CryEntitySystem/IEntity.h>

namespace Snake
{
	struct SFruitState
	{
		float m_timer = 0.f;
		Vec3 m_currentPos = ZERO;
	};

	class CFruit final : public IEntityComponent
	{
	public:
		CFruit() = default;
		virtual ~CFruit() {}

		static void ReflectType(Schematyc::CTypeDesc<CFruit>& desc)
		{
			desc.SetGUID("{7387170E-7B6F-4023-A641-C71B909A1353}"_cry_guid);
			desc.SetEditorCategory("Game");
			desc.SetLabel("Fruit");
			desc.SetDescription("Fruit");
			desc.SetComponentFlags({ IEntityComponent::EFlags::Transform, IEntityComponent::EFlags::Socket, IEntityComponent::EFlags::Attach, IEntityComponent::EFlags::NetNotReplicate });
		}

		// IEntityComponent
		virtual void   Initialize() override;
		virtual Cry::Entity::EventFlags GetEventMask() const override;
		virtual void   ProcessEvent(const SEntityEvent& event) override;
		virtual void   OnShutDown() override;
		// ~IEntityComponent
	public:
		void OnCollideWithSnake(class CSnake* pSnake);
		bool IsSpecialFruit() const { return m_isSpecialFruit; }
		SFruitState  GetState();

		void SetTimer(float timer) { m_timer = timer; }
#ifndef RELEASE
		const float& GetTimer() const { return m_timer; }
		const float& GetDestructionTime() const { return m_destructionTime; }
#endif
	private:
		class CField* GetField();
	private:
		bool m_isSpecialFruit = false;
		float m_destructionTime = 0.f;
	private:
		bool up = true;
		float current_z = 0.0f;
		int rotationDegrees = 0;
		float m_timer = 0.f;
	};
}