#pragma once

#include <memory>
#include <vector>
#include <string>
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include "miniz/miniz.h"
#include "AxmlParser/AxmlParser.h"

namespace utils
{
	inline std::shared_ptr<char> read_file(const std::string& file_path, size_t& file_size)
	{
		const auto in_file = fopen(file_path.c_str(), "rb");
		if (in_file == nullptr)
		{
			printf("failed to open file: %s\n", file_path.c_str());
			return {};
		}
		fseek(in_file, 0, SEEK_END);
		const auto in_size = ftell(in_file);
		file_size = in_size;
		fseek(in_file, 0, SEEK_SET);

		std::unique_ptr<char> in_buff(new char[in_size]);
		fread(in_buff.get(), 1, in_size, in_file);
		fclose(in_file);

		return in_buff;
	}

	inline bool write_file(const std::string& file_path, const char* content, const size_t content_size)
	{
		const auto out_file = fopen(file_path.c_str(), "wb");
		if (out_file == nullptr)
		{
			return false;
		}

		const auto write_number = content_size / sizeof(char);
		const auto out_size = fwrite(content, sizeof(char), write_number, out_file);
		fclose(out_file);

		return out_size == write_number;
	}

	inline std::pair<std::string, std::string> split(const std::string& full_string, const char deliminator)
	{
		const auto found = full_string.find_first_of(deliminator);
		if (found == std::string::npos)
		{
			return std::make_pair(full_string, "");
		}

		const auto first_part = full_string.substr(0, found);
		const auto second_part = full_string.substr(found + 1);

		return std::make_pair(first_part, second_part);
	}

	static bool ends_with(const std::string& value, const std::string& ending)
	{
		if (ending.size() > value.size())
		{
			return false;
		}
		return std::equal(ending.rbegin(), ending.rend(), value.rbegin(),
		                  [](const char a, const char b)
		                  {
			                  return tolower(a) == tolower(b);
		                  });
	}

	static bool starts_with(const std::string& value, const std::string& start)
	{
		if (start.size() > value.size())
		{
			return false;
		}

		return std::equal(start.begin(), start.end(), value.begin(),
		                  [](const char a, const char b)
		                  {
			                  return a == b;
		                  });
	}

	inline std::string unzip_file(const fs::path& file_path, bool full_unpack = true)
	{
		mz_zip_archive zip_archive;
		memset(&zip_archive, 0, sizeof(zip_archive));

		const auto status = mz_zip_reader_init_file(&zip_archive, file_path.string().c_str(), 0);
		if (!status)
			return {};
		const auto file_count = int(mz_zip_reader_get_num_files(&zip_archive));
		if (file_count == 0)
		{
			mz_zip_reader_end(&zip_archive);
			return {};
		}
		// printf("number_of_files: %zu\n", file_count);

		const auto dest_dir = file_path.parent_path().string() + '/' + file_path.filename().string() + "_unpacked";
		if (fs::exists(dest_dir))
		{
			fs::remove_all(dest_dir);
		}
		fs::create_directory(dest_dir);

		for (auto i = 0; i < file_count; i++)
		{
			mz_zip_archive_file_stat file_stat;
			if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat))
			{
				printf("failed to get file stat. index: %d\n", i);
				continue;
			}
			const std::string file_name{file_stat.m_filename};
			const auto dest_file = dest_dir + '/' + file_name;
			// const auto is_dir = mz_zip_reader_is_file_a_directory(&zip_archive, i); // always returns "false"
			const auto under_dir = file_name.find('/') != std::string::npos;
			if (under_dir)
			{
				const auto is_cert_dir = file_name.find("META-INF") != std::string::npos;
				if (!full_unpack && !is_cert_dir)
				{
					continue;
				}

				const fs::path under_dir_path{dest_file};
				const std::string under_dir_full = under_dir_path.parent_path();
				if (!fs::exists(under_dir_full))
				{
					fs::create_directories(under_dir_full);
				}
			}

			const auto is_okay = mz_zip_reader_extract_to_file(&zip_archive, i, dest_file.c_str(), 0);

			// printf("%s\n", file_stat.m_filename);
		}

		mz_zip_reader_end(&zip_archive);
		return dest_dir;
	}

	inline void clrscr()
	{
		printf("\033[2J\033[1;1H");
	}
} // namespace utils
