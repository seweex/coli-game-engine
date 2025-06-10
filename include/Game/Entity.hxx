#pragma once

#include "../Common.hxx"
#include "../Common/Identifiable.hxx"

#include "Behavioral.hxx"
#include "Asset.hxx"

namespace Coli
{
	namespace Detail
	{
		class EntityBase :
			public virtual IdentifiableBase,
			public virtual BehavioralBase			
		{
		protected:
			EntityBase() noexcept = default;

		public:
			EntityBase(EntityBase&&)      = delete;
			EntityBase(EntityBase const&) = delete;

			EntityBase& operator=(EntityBase&&)		 = delete;
			EntityBase& operator=(EntityBase const&) = delete;
		};
	}
}