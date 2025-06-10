#pragma once

#include "../Common.hxx"
#include "../Utility.hxx"

namespace Coli
{
	namespace Detail
	{
		class FileLoaderBase
		{
		protected:
			static void x_failed_open() {
				throw std::runtime_error("Failed to open the file");
			}

			FileLoaderBase (std::string_view rootPath, std::string_view fileFormat) :
				myRootPath   (rootPath),
				myFileFormat (fileFormat)
			{}

		public:
			FileLoaderBase(FileLoaderBase&&)	  = delete;
			FileLoaderBase(FileLoaderBase const&) = delete;

			FileLoaderBase& operator=(FileLoaderBase&&)		 = delete;
			FileLoaderBase& operator=(FileLoaderBase const&) = delete;

		protected:
			_NODISCARD std::string make_path (std::string_view assetName) const
			{
				using namespace std::literals;

				auto totalLength = myRootPath.size() + assetName.size() + myFileFormat.size();

				std::string result;
				result.reserve(totalLength);

				result += myRootPath;
				result += assetName;
				result += myFileFormat;

				return result;
			}

		private:
			std::string myRootPath;
			std::string myFileFormat;
		};
	}
}