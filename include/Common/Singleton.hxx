#pragma once

#include "../Common.hxx"

namespace Coli
{
	namespace Detail
	{
		template <class _Ty, bool _Available = false>
		class SingletonBase;

		template <class _Ty>
		class SingletonBase <_Ty, false>
		{
			static void x_another_instance() {
				throw std::logic_error("Another instance of this singleton type already exists");
			}

			static void x_no_instance() {
				throw std::logic_error("No instance of this singleton type already exists");
			}

		protected:
			SingletonBase() {
				if (exist())
					x_another_instance();
				else
					instance = static_cast<_Ty*>(this);
			}

		public:
			SingletonBase(SingletonBase&&)		= delete;
			SingletonBase(SingletonBase const&) = delete;

			SingletonBase& operator=(SingletonBase&&)	   = delete;
			SingletonBase& operator=(SingletonBase const&) = delete;

			_NODISCARD static bool exist() noexcept {
				return instance != nullptr;
			}
			
			friend class SingletonBase <_Ty, true>;

		private:
			static inline _Ty* instance = nullptr;
		};

		template <class _Ty>
		class SingletonBase <_Ty, true> :
			public SingletonBase <_Ty, false>
		{
		public:
			_NODISCARD static _Ty& get_existing()  {
				if (this->exist())
					return * SingletonBase<_Ty, false>::instance;
				else
					SingletonBase<_Ty, false>::x_no_instance();
			}
		};
	}
}