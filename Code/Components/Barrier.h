// Copyright (C) 2021 Space Raccoon Game Studio. All rights reserved. Contacts: <business@space-raccoon.com>
// Created by AfroStalin

#pragma once

#include <CryEntitySystem/IEntityComponent.h>
#include <CryEntitySystem/IEntity.h>

namespace Snake
{
	class CBarrier final : public IEntityComponent
	{
	public:
		CBarrier() = default;
		virtual ~CBarrier() {}

		static void ReflectType(Schematyc::CTypeDesc<CBarrier>& desc)
		{
			desc.SetGUID("{370FF550-29CD-4992-8B79-72FE031A7CCB}"_cry_guid);
			desc.SetEditorCategory("Game");
			desc.SetLabel("Barrier");
			desc.SetDescription("Barrier");
			desc.SetComponentFlags({ IEntityComponent::EFlags::Transform, IEntityComponent::EFlags::Socket, IEntityComponent::EFlags::Attach, IEntityComponent::EFlags::NetNotReplicate });

			desc.AddMember(&CBarrier::m_alignButton, 'btn1', "align", "Align", "Align to closest field point", Serialization::FunctorActionButton<std::function<void()>>());
			desc.AddMember(&CBarrier::m_alignedWorldPos, 'wrld', "Aligned", "Aligned", "Aligned world pos", ZERO);
			desc.AddMember(&CBarrier::bboxSize, 'bbox', "BBoxSize", "BBoxSize", "BBoxSize", 1.f);
		}

		// IEntityComponent
		virtual void   Initialize() override;
		virtual Cry::Entity::EventFlags GetEventMask() const override;
		// ~IEntityComponent
	public:
		struct SListener
		{
			virtual void OnCollideWithSnake(const int16& partIndex) = 0;
		};

		void RegisterListener(CBarrier::SListener* pListener);
		void UnregisterListener(CBarrier::SListener* pListener);
	protected:
		Serialization::FunctorActionButton<std::function<void()>> m_alignButton;
	public:
		void OnCollideWithSnake(const int16& partIndex);
		void AlignToField();
		void SetSize(const float& size) { bboxSize = size; }
		AABB GetBBox();
	private:
		Vec3 m_alignedWorldPos = ZERO;
		float bboxSize = 1.f;
		std::vector<CBarrier::SListener*> m_listeners;
	};
}