#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

namespace Coli
{
	namespace Detail
	{
		template <class _StreamTy, class... _ValTys>
		concept OutputValue = requires (_StreamTy _stream, _ValTys&&... _vals)
		{
			(_stream << ... << _vals);
		};

		template <class _ToTy>
		_ToTy char_cast(char from) {
			static_assert(false, "invalid types");
		}

		template <>
		_NODISCARD char char_cast<char>(char from) noexcept {
			return from;
		}

		template <>
		_NODISCARD wchar_t char_cast<wchar_t>(char from)
		{
			wchar_t	   result;
			auto const err = std::mbstowcs(std::addressof(result), 
										   std::addressof(from), 
										   1);

			if (err == static_cast<size_t>(-1))
				throw std::runtime_error("Conversion failed");

			return result;
		}

		template <>
		_NODISCARD char8_t char_cast<char8_t>(char from) noexcept {
			return static_cast<char8_t>(from);
		}

		template <>
		_NODISCARD char16_t char_cast<char16_t>(char from)
		{
			std::mbstate_t state = {};
			char16_t       result;

			auto const err = std::mbrtoc16(std::addressof(result),
										   std::addressof(from),
										   1,
										   std::addressof(state));

			if (err == static_cast<size_t>(-1))
				throw std::runtime_error("Conversion failed");

			return result;
		}

		template <>
		_NODISCARD char32_t char_cast<char32_t>(char from)
		{
			std::mbstate_t state = {};
			char32_t       result;

			auto const err = std::mbrtoc32(std::addressof(result),
										   std::addressof(from),
										   1,
										   std::addressof(state));

			if (err == static_cast<size_t>(-1))
				throw std::runtime_error("Conversion failed");

			return result;
		}

		template <class _ToTy>
		std::basic_string<_ToTy> string_cast(std::string_view from) {
			static_assert(false, "invalid types");
		}

		template <>
		_NODISCARD std::basic_string<char> string_cast<char>(std::string_view from) {
			return std::basic_string<char>{ from };
		}

		template <>
		_NODISCARD std::basic_string<wchar_t> string_cast<wchar_t>(std::string_view from)
		{
			std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

			return converter.from_bytes(from.data(), 
										from.data() + from.size());
		}

		template <>
		_NODISCARD std::basic_string<char8_t> string_cast<char8_t>(std::string_view from) {
			return { from.begin(), from.end() };
		}

		template <>
		_NODISCARD std::basic_string<char16_t> string_cast<char16_t>(std::string_view from)
		{
			std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;

			return converter.from_bytes(from.data(), 
										from.data() + from.size());
		}

		template <>
		_NODISCARD std::basic_string<char32_t> string_cast<char32_t>(std::string_view from)
		{
			std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;

			return converter.from_bytes(from.data(), 
										from.data() + from.size());
		}
	}

	namespace Debug
	{
		enum class Severity {
			info,
			warn,
			error
		};

		template <class _CharTy>
		class Logger final
		{
			_NODISCARD std::basic_string<_CharTy> make_timestamp()
			{
				auto const currentTime = time(nullptr);
				tm		   localTime;

				localtime_s(std::addressof(localTime), 
							std::addressof(currentTime));

				myTimestampHelper.str({});
				myTimestampHelper.clear();

				myTimestampHelper
					<< leftBracket
					<< std::setw(2) << std::setfill(zero) << localTime.tm_hour
					<< colon
					<< std::setw(2) << std::setfill(zero) << localTime.tm_min
					<< colon
					<< std::setw(2) << std::setfill(zero) << localTime.tm_sec
					<< rightBracket;

				return myTimestampHelper.str();
			}

			_NODISCARD std::basic_string_view<_CharTy> make_severity(Severity val)
			{
				static const auto info    = Detail::string_cast<_CharTy>("[INFO]");
				static const auto warn    = Detail::string_cast<_CharTy>("[WARN]");
				static const auto error   = Detail::string_cast<_CharTy>("[ERROR]");
				static const auto unknown = Detail::string_cast<_CharTy>("[UNKNOWN SEVERITY]");

				switch (val)
				{
				case Severity::info:
					return info;

				case Severity::warn:
					return warn;

				case Severity::error:
					return error;

				default:
					return unknown;
				}
			}

			template <class _Ty, class... _RestTys>
			void print (std::basic_ostream<_CharTy>& stream, _Ty&& val, _RestTys&&... rest) {
				stream << std::forward<_Ty>(val) << space;
				print(stream, std::forward<_RestTys>(rest)...);
			}

			template <class _Ty>
			void print (std::basic_ostream<_CharTy>& stream, _Ty&& val) {
				stream << std::forward<_Ty>(val) << std::endl;
			}

		public:
			template <std::derived_from<std::basic_ostream<_CharTy>> _Ty, class... _ArgTys>
			void make_output_stream(_ArgTys&&... args) {
				auto ptr = std::make_unique<_Ty>(std::forward<_ArgTys>(args)...);
				myStreams.emplace_back(std::move(ptr));
			}

			template <std::derived_from<std::basic_ostream<_CharTy>> _Ty>
			void add_output_stream(_Ty& stream) {
				myStreams.emplace_back(static_cast<std::basic_ostream<_CharTy>*>(std::addressof(stream)));
			}

			template <class... _ValTys> requires (
				sizeof...(_ValTys) > 0 &&
				Detail::OutputValue<std::basic_ostream<_CharTy>, _ValTys...>
			)
			void log (Severity sev, _ValTys&&... vals)
			{
				if (!myStreams.empty())
				{
					auto const timestamp = make_timestamp();
					auto const severity  = make_severity(sev);

					for (auto& stream : myStreams)
					{
						using as_owning_type = std::unique_ptr<std::basic_ostream<_CharTy>>;
						using as_ref_type    = std::basic_ostream<_CharTy>*;

						as_ref_type ptr;

						if (std::holds_alternative<as_owning_type>(stream))
							ptr = std::get<as_owning_type>(stream).get();

						else if (std::holds_alternative<std::basic_ostream<_CharTy>*>(stream))
							ptr = std::get<as_ref_type>(stream);

						else continue;

						print(*ptr, timestamp, severity, std::forward<_ValTys>(vals)...);
					}
				}
			}

		private:
			static const inline auto leftBracket  = Detail::char_cast<_CharTy>('[');
			static const inline auto rightBracket = Detail::char_cast<_CharTy>(']');
			static const inline auto colon		  = Detail::char_cast<_CharTy>(':');
			static const inline auto space		  = Detail::char_cast<_CharTy>(' ');
			static const inline auto zero	      = Detail::char_cast<_CharTy>('0');

			std::basic_stringstream<_CharTy> myTimestampHelper;

			std::vector <std::variant <std::unique_ptr<std::basic_ostream<_CharTy>>,
									   std::basic_ostream<_CharTy> *>>
			myStreams;
		};
	}
}