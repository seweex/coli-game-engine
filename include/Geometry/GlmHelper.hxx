#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

namespace std
{
	template <glm::length_t _L, class _T, glm::qualifier _Q>
	struct hash<glm::vec<_L, _T, _Q>>
	{
		_NODISCARD size_t operator()(glm::vec<_L, _T, _Q> const& val) const noexcept requires (_L > 0)
		{
			size_t hash = Coli::Detail::FNV::start_value;

			for (glm::length_t i = 0; i < _L; ++i)
				hash = Coli::Detail::FNV::mix_hash(std::hash<_T>{}(val[i]), hash);

			return hash;
		}
	};

	template <glm::length_t _C, glm::length_t _R, class _T, glm::qualifier _Q>
	struct hash<glm::mat<_C, _R, _T, _Q>>
	{
		_NODISCARD size_t operator()(glm::mat<_C, _R, _T, _Q> const& val) const noexcept requires(_C > 0 && _R > 0)
		{
			size_t hash = Coli::Detail::FNV::start_value;

			for (glm::length_t i = 0; i < _C; ++i)
			for (glm::length_t j = 0; j < _R; ++j)
				hash = Coli::Detail::FNV::mix_hash(std::hash<_T>{}(val[i][j]), hash);

			return hash;
		}
	};

	template <class _T, glm::qualifier _Q>
	struct hash<glm::qua<_T, _Q>> 
	{
		_NODISCARD size_t operator()(glm::qua<_T, _Q> const& val) const noexcept
		{
			size_t hash = Coli::Detail::FNV::start_value;
			
			for (glm::length_t i = 0; i < 4; ++i)
				hash = Coli::Detail::FNV::mix_hash(std::hash<_T>{}(val[i]), hash);

			return hash;
		}
	};
}

namespace nlohmann 
{
	template <glm::length_t _L, class _T, glm::qualifier _Q>
	struct adl_serializer <glm::vec<_L, _T, _Q>> 
	{
		static void to_json(json& j, const glm::vec<_L, _T, _Q>& val)
		{
			for (glm::length_t i = 0; i < _L; ++i)
				j.push_back(val[i]);
		}

		static void from_json(const json& j, glm::vec<_L, _T, _Q>& val) 
		{
			if (j.is_array() && j.size() == _L)
				for (glm::length_t i = 0; i < _L; ++i)
					val[i] = j[i].get<_T>();
			else
				throw std::invalid_argument("invalid json object");
		}
	};

	template <glm::length_t _C, glm::length_t _R, class _T, glm::qualifier _Q>
	struct adl_serializer <glm::mat<_C, _R, _T, _Q>>
	{
		static void to_json(json& j, const glm::mat<_C, _R, _T, _Q>& val)
		{
			json column;

			for (glm::length_t i = 0; i < _C; ++i) {
				adl_serializer<typename glm::mat<_C, _R, _T, _Q>::col_type>::to_json(column, val[i]);

				j.push_back(column);
				column.clear();
			}
		}

		static void from_json(const json& j, glm::mat<_C, _R, _T, _Q>& val)
		{
			if (j.is_array() && j.size() == _C) 
				for (glm::length_t i = 0; i < _C; ++i)
					adl_serializer<typename glm::mat<_C, _R, _T, _Q>::col_type>::from_json(j[i], val[i]);
			else
				throw std::invalid_argument("invalid json object");
		}
	};

	template <class _T, glm::qualifier _Q>
	struct adl_serializer <glm::qua<_T, _Q>> 
	{
		static void to_json(json& j, const glm::qua<_T, _Q>& val)
		{
			for (glm::length_t i = 0; i < glm::qua<_T, _Q>::length(); ++i)
				j.push_back(val[i]);
		}

		static void from_json(const json& j, glm::qua<_T, _Q>& val)
		{
			constexpr auto length = glm::qua<_T, _Q>::length();

			if (j.is_array() && j.size() == length)
				for (glm::length_t i = 0; i < length; ++i)
					val[i] = j[i].get<_T>();
			else
				throw std::invalid_argument("invalid json object");
		}
	};
}