#pragma once

#include "../Common.hxx"

namespace Coli
{
	namespace Detail
	{
		class IdentifiableBase
		{
		protected:
			IdentifiableBase() noexcept :
				myID (distributor(generator))
			{}

		public:
			IdentifiableBase(IdentifiableBase&&)	  = delete;
			IdentifiableBase(IdentifiableBase const&) = delete;

			IdentifiableBase& operator=(IdentifiableBase&&)	     = delete;
			IdentifiableBase& operator=(IdentifiableBase const&) = delete;

			_NODISCARD size_t get_id() const noexcept {
				return myID;
			}

		private:
			static constexpr size_t		     seed = 0xf70e98230;
			static inline	 std::mt19937_64 generator { seed };

			static inline std::uniform_int_distribution <size_t> distributor { 0 };

			size_t myID;
		};
	}
}