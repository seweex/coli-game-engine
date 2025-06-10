#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

#include "Configuration.hxx"

#include "../File/AssetLoader.hxx"
#include "../File/MeshLoader.hxx"

namespace Coli
{
	namespace Generic
	{
		class FileSystem final
		{
			static void gen_pathes (std::string const& root)
			{
				auto const path = std::filesystem::path(root);

				for (auto const& subdir : Subdirectories::all) 
				{
					auto subpath = path;
					subpath.append(subdir);

					std::filesystem::create_directories(subpath);
				}
			}

		public:
			FileSystem (std::string const& rootPath) /* there is no operator+ for string_view and string */ :
				myMeshLoader  (rootPath + Subdirectories::mesh),
				myAssetLoader (rootPath + Subdirectories::assets)
			{
				gen_pathes(rootPath);
			}

			FileSystem(FileSystem&&)	  = delete;
			FileSystem(FileSystem const&) = delete;

			FileSystem& operator=(FileSystem&&)		 = delete;
			FileSystem& operator=(FileSystem const&) = delete;

			template <std::derived_from <Detail::AssetBase> _Ty>
			void save_asset (std::string_view name, _Ty const& obj) 
			{
				auto saved = obj.save();

				myAssetLoader.save(name, saved);
			}

			template <std::derived_from <Detail::AssetBase> _Ty>
			_Ty& load_asset (std::string_view name, _Ty& obj)
			{
				auto object = myAssetLoader.load(name);

				obj.restore(object);
				return obj;
			}

			template <Detail::Vertex _VertexTy>
			Geometry::Mesh <_VertexTy> load_mesh (std::string_view name) {
				return myMeshLoader.load<_VertexTy>(name);
			}

			_NODISCARD Configuration load_config () 
			{
				Configuration cfg = {};

				try {
					cfg = myAssetLoader.load(AssetNames::config);
				}
				catch (std::runtime_error const&) {}

				return cfg;
			}

			void save_config(Configuration const& cfg) {
				myAssetLoader.save(AssetNames::config, cfg);
			}

		private:
			struct AssetNames {
				static constexpr std::string_view config = "config";
			};

			struct Subdirectories {
				static inline const std::string mesh   = "mesh/";
				static inline const std::string assets = "assets/";

				static inline const std::unordered_set <std::string_view> all = {
					mesh, assets
				};
			};

			File::MeshLoader  myMeshLoader;
			File::AssetLoader myAssetLoader;
		};
	}
}