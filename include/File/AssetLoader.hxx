#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

#include "../Game/Asset.hxx"

#include "Loader.hxx"

namespace Coli
{
	namespace File
	{
		class AssetLoader final :
			public Detail::FileLoaderBase
		{
		public:
			AssetLoader(std::string_view rootPath) :
				FileLoaderBase (rootPath, ".json")
			{}

			AssetLoader(AssetLoader&&)      = delete;
			AssetLoader(AssetLoader const&) = delete;

			AssetLoader& operator=(AssetLoader&&)      = delete;
			AssetLoader& operator=(AssetLoader const&) = delete;

			_NODISCARD nlohmann::json load (std::string_view assetName)
			{
				auto path = this->make_path(assetName);

				std::ifstream  stream { path };
				nlohmann::json obj;
				
				if (stream)
					obj = nlohmann::json::parse(stream);
				else 
					x_failed_open();

				return obj;
			}

			void save (std::string_view assetName, nlohmann::json const& obj)
			{
				auto path = this->make_path(assetName);
				std::ofstream stream { path, std::ios::trunc };

				if (stream)
					stream << std::setw(4) << obj << std::endl;
				else 
					x_failed_open();
			}
		};
	}
}