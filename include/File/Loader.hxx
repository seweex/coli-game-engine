#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

namespace Coli
{
	namespace Detail
	{
		struct Settings
		{
			int windowWidth = 1366;
			int windowHeight = 768;
		};
	}
}

namespace nlohmann
{
	template <>
	struct adl_serializer <Coli::Detail::Settings>
	{
	private:
		static constexpr std::string_view name_window_width = "windowWidth";
		static constexpr std::string_view name_window_height = "windowHeight";

	public:
		static void to_json(json& j, Coli::Detail::Settings const& val)
		{
			j[name_window_width]  = val.windowWidth;
			j[name_window_height] = val.windowHeight;
		}

		static void from_json(json const& j, Coli::Detail::Settings& val)
		{
			using Coli::Detail::Json::try_fill;

			int tempWindowWidth;
			try_fill(j, tempWindowWidth, name_window_width);

			int tempWindowHeight;
			try_fill(j, tempWindowHeight, name_window_height);

			val.windowWidth  = tempWindowWidth;
			val.windowHeight = tempWindowHeight;
		}
	};
}

namespace Coli
{
	namespace File
	{
		class Loader final
		{
		public:
			Loader(std::string_view rootPath = "./") :
				myRootPath (rootPath)
			{}

			_NODISCARD Detail::Settings load_settings(std::string_view name)
			{
				try {
					return load_serialized(name);
				}
				catch (std::runtime_error const&) {
					return {};
				}
			}

			_NODISCARD nlohmann::json load_serialized(std::string_view name)
			{
				myStream.open(myRootPath + name.data() + ".json", std::ios::in);
				nlohmann::json obj;

				if (myStream.is_open())
					try {
						obj = nlohmann::json::parse(myStream);
					}
					catch (nlohmann::json::parse_error const&) {
						myStream.close();
						throw;
					}
				else {
					myStream.close();
					throw std::runtime_error("failed to open file for read");
				}

				myStream.close();
				return obj;
			}

			void save_settings(std::string_view name, Detail::Settings const& settings) {
				save_serialized(name, settings);
			}

			void save_serialized(std::string_view name, nlohmann::json const& obj)
			{
				myStream.open(myRootPath + name.data() + ".json", std::ios::trunc | std::ios::out);

				if (myStream.is_open())
					myStream << std::setw(4) << obj << std::endl;

				else {
					myStream.close();
					throw std::runtime_error("failed to open file for write");
				}

				myStream.close();
			}

		private:
			std::string const myRootPath;

			std::fstream myStream;
		};
	}
}