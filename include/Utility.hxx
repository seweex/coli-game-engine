#pragma once

#include "Common.hxx"

namespace Coli
{
	namespace Detail
	{
		template <class _Ty>
		concept Hashable = requires (_Ty _val) {
			{ std::hash<_Ty>{}(_val) } -> std::convertible_to<size_t>;
		};

		template <class _Ty>
		concept LinearContainer = requires (_Ty _val)
		{
			typename _Ty::value_type;

			{ _val.size() } -> std::convertible_to <size_t>;
			{ _val.data() } -> std::convertible_to <typename _Ty::value_type const*>;
		};

		template <class _Ty, class _ValueTy>
		concept LinearContainerOf =
			LinearContainer <_Ty> &&
			std::same_as <std::remove_cv_t <typename _Ty::value_type>, 
						  std::remove_cv_t <_ValueTy>>;

		template <class _Ty>
		concept IterableContainer = requires (_Ty _val)
		{
			typename _Ty::value_type;

			{ _val.size() }  -> std::convertible_to <size_t>;
			{ _val.begin() } -> std::input_or_output_iterator;
			{ _val.end() }   -> std::input_or_output_iterator;
		};

		template <class _Ty, class _ValueTy>
		concept IterableContainerOf =
			IterableContainer <_Ty> &&
			std::same_as <std::remove_cv_t <typename _Ty::value_type>,
						  std::remove_cv_t <_ValueTy>>;

		class TransparentHash final
		{
		public:
			using is_transparent = std::true_type;

			template <Hashable _Ty>
			_NODISCARD size_t operator()(_Ty const& val) const noexcept {
				return std::hash<_Ty>{}(val);
			}
		};

		class HashMixer final
		{
			static constexpr size_t prime_value = 0x01000193;

		public:
			static constexpr size_t start_value = 0x811c9dc5;

			_NODISCARD size_t operator()(size_t next, size_t current = start_value) const noexcept {
				return (current * prime_value) ^ next;
			}
		};

		struct DataProxy {
			void const* const data;
			size_t const	  size;
		};

		template <class> 
		struct IsOptional : public std::false_type {};

		template <class _Ty>
		struct IsOptional <std::optional <_Ty>> : public std::true_type {};

		namespace Json
		{
			template <class _ValTy> requires (
				std::is_object_v <_ValTy> &&
				!std::is_const_v <_ValTy>
			)
			void try_fill (nlohmann::json const& obj, _ValTy& storage, auto const& key)
			{
				try {
					if constexpr (IsOptional<_ValTy>::value)
					{
						auto& atKey = obj.at(key);

						if (atKey.is_null())
							storage.reset();
						else
							storage.emplace(atKey.get<typename _ValTy::value_type>());
					}
					else if constexpr (std::same_as<_ValTy, nlohmann::json>)
						storage = obj.at(key);
					else
						storage = obj.at(key).get<_ValTy>();
				}
				catch (nlohmann::json::exception const& exc) 
				{
					using namespace std::string_literals;
					throw std::invalid_argument("Invalid json object: "s + exc.what());
				}
			}
		}
	}
}

namespace nlohmann
{
	template <class _Ty>
	struct adl_serializer <std::optional <_Ty>>
	{
		static void to_json (json& j, std::optional<_Ty> const& val) {
			if (val.has_value())
				j = *val;
			else
				j = nullptr;
		}

		static void from_json (json const& j, std::optional<_Ty>& val) {
			if (j.is_null())
				val.reset();
			else
				val.emplace(j.get<_Ty>());
		}
	};
}