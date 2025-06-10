#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

#include "../Graphics/Window.hxx"

namespace Coli
{
	namespace Generic
	{
		struct Configuration {
			Graphics::Window::Configuration windowConfig = {};
		};
	}
}

namespace nlohmann
{
	template <>
	struct adl_serializer <Coli::Generic::Configuration>
	{
	private:
		struct Keys {
			static constexpr std::string_view window = "window";
		};

	public:
		static void to_json(json& j, Coli::Generic::Configuration const& val) {
			j [Keys::window] = val.windowConfig;
		}

		static void from_json(const json& j, Coli::Generic::Configuration& val)
		{
			using Coli::Detail::Json::try_fill;

			decltype (val.windowConfig) tempWindowConfig;
			try_fill (j, tempWindowConfig, Keys::window);

			val.windowConfig = tempWindowConfig;
		}
	};
}