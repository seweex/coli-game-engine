#pragma once

#include "../Common.hxx"
#include "../Common/Identifiable.hxx"

#include "Behavioral.hxx"

namespace Coli
{
	namespace Detail
	{
		class AssetBase :
			public virtual IdentifiableBase,
			public virtual StatefulBase			
		{
		protected:
			static void x_cannot_restore() {
				throw std::invalid_argument("This asset cannot be restored from this JSON object");
			}

		private:
			_NODISCARD bool is_correct(nlohmann::json const& obj) const 
			{
				return obj.is_object() &&
					obj.contains (Keys::id) && obj[Keys::id] == this->get_id() &&
					obj.contains (Keys::enabling) &&
					obj.contains (Keys::visibility);
			}

		protected:
			AssetBase() noexcept = default;

		public:
			AssetBase(AssetBase&&)      = delete;
			AssetBase(AssetBase const&) = delete;

			AssetBase& operator=(AssetBase&&)	   = delete;
			AssetBase& operator=(AssetBase const&) = delete;

			_NODISCARD virtual nlohmann::json on_save () const = 0;
			virtual void on_restore (nlohmann::json const&) = 0;

			_NODISCARD nlohmann::json save() const
			{
				auto object = this->on_save();
				
				object [Keys::id]	      = this->get_id();
				object [Keys::enabling]   = this->is_active();
				object [Keys::visibility] = this->is_visible();

				return object;
			}

			void restore (nlohmann::json const& obj) 
			{
				if (is_correct(obj))
					this->on_restore(obj);
				else
					x_cannot_restore();
			}

		protected:
			struct Keys {
				static constexpr std::string_view id	     = "ID";
				static constexpr std::string_view enabling   = "IsEnabled";
				static constexpr std::string_view visibility = "IsVisible";
			};
		};
	}
}