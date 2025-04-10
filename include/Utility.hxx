#pragma once

#include "Common.hxx"

namespace Coli
{
	namespace Detail
	{
		template <class _Ty>
		concept LinearContainer = requires (_Ty _val)
		{
			{ _val.size() } -> std::convertible_to <size_t>;
			{ _val.data() } -> std::convertible_to <void const*>;

			typename _Ty::value_type;
		};

		template <class _Ty, class _ValueTy>
		concept LinearContainerOf =
			LinearContainer <_Ty> &&
			std::same_as<std::remove_cv_t <typename _Ty::value_type>, 
						 std::remove_cv_t <_ValueTy>>;

		class TransparentHash final
		{
		public:
			using is_transparent = std::true_type;

			template <class _Ty> requires (requires{ std::hash<_Ty>{}(_STD declval<const _Ty&>()); })
			_NODISCARD size_t operator()(_Ty const& val) const noexcept {
				return std::hash<_Ty>{}(val);
			}
		};

		class IdentifiableBase
		{
			_NODISCARD static size_t take_id() noexcept {
				return previous_id = std::hash<size_t>{}(previous_id);
			}

		protected:
			IdentifiableBase() noexcept :
				myID (take_id())
			{}

		public:
			_NODISCARD size_t get_id() const noexcept {
				return myID;
			}

		private:
			static inline size_t previous_id = 0;

			size_t const myID;
		};

		class TypeIdentifiableBase
		{
		public:
			_NODISCARD virtual size_t get_type_id() const noexcept = 0;
		};

		template <class> 
		struct IsOptional : public std::false_type {};

		template <class _Ty>
		struct IsOptional<std::optional<_Ty>> : public std::true_type {};

		namespace FNV 
		{
			constexpr size_t prime_value = 0x01000193;
			constexpr size_t start_value = 0x811c9dc5;

			_NODISCARD constexpr size_t mix_hash(size_t next, size_t current = start_value) noexcept {
				return (current * prime_value) ^ next;
			}
		}

		namespace Json
		{
			void try_fill(nlohmann::json const& obj, auto& storage, auto const& key)
			{
				using storage_type = std::remove_reference_t<decltype(storage)>;

				static_assert(std::is_lvalue_reference_v<decltype(storage)>,
							  "'storage' must be a lvalue reference");

				static_assert(!std::is_const_v<storage_type>, 
							  "'storage' must be of a non-const type");
				try {
					if constexpr (IsOptional<storage_type>::value)
					{
						auto& atKey = obj.at(key);

						if (atKey.is_null())
							storage.reset();
						else
							storage.emplace(atKey.get<typename storage_type::value_type>());
					}
					else if constexpr (std::same_as<storage_type, nlohmann::json>)
						storage = obj.at(key);
					else
						storage = obj.at(key).get<std::remove_reference_t<decltype(storage)>>();
				}
				catch (nlohmann::json::exception const& exc) {
					using namespace std::string_literals;
					throw std::invalid_argument("invalid json object: "s + exc.what());
				}
			}
		}

		template <std::floating_point _FloatTy>
		_NODISCARD constexpr _FloatTy clamp_0_1(_FloatTy val) noexcept 
		{
			constexpr auto min = std::numeric_limits<_FloatTy>::epsilon();
			constexpr auto max = 1 - std::numeric_limits<_FloatTy>::epsilon();

			return glm::clamp<_FloatTy>(val, min, max);
		}
	}
}

namespace nlohmann
{
	template <class _Ty>
	struct adl_serializer <std::optional<_Ty>>
	{
		static void to_json(json& j, std::optional<_Ty> const& val) {
			if (val)
				j = *val;
			else
				j = nullptr;
		}

		static void from_json(json const& j, std::optional<_Ty>& val) {
			if (j.is_null())
				val.reset();
			else
				val.emplace(j.get<_Ty>());
		}
	};
}