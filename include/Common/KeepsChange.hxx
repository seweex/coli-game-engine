#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

namespace Coli
{
	namespace Detail
	{
		template <class _Ty> 
			requires (Hashable<_Ty>)
		class KeepsChangeBase
		{
		protected:
			KeepsChangeBase() noexcept = default;

		public:
			KeepsChangeBase(KeepsChangeBase&&)	    = delete;
			KeepsChangeBase(KeepsChangeBase const&) = delete;

			KeepsChangeBase& operator=(KeepsChangeBase&&)	   = delete;
			KeepsChangeBase& operator=(KeepsChangeBase const&) = delete;

			_NODISCARD bool has_changed() const noexcept {
				return myPreviousHash != myCurrentHash;
			}

		protected:
			void commit(_Ty const& thisPtr) noexcept {
				myPreviousHash = myCurrentHash;
				myCurrentHash  = std::hash<_Ty>{}(thisPtr);
			}

		private:
			size_t myCurrentHash  = 0;
			size_t myPreviousHash = 0;
		};
	}
}