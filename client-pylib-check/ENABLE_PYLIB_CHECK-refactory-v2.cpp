#ifdef ENABLE_PYLIB_CHECK
#include <iostream>
#include <string>
#include <cstring>
#include <algorithm>
#include <unordered_map>
#include <fmt/format.h> // Assuming you have fmt library installed

constexpr bool PyLibCheckForce = false;
constexpr int PrintLevel = 0;
constexpr auto PyLibFolder = "lib";

template <class... Args>
void PrintMe(int level, const Args&... Arguments)
{
	if (PrintLevel >= level)
		TraceError(Arguments...);
}

typedef struct PyLibFiles_s
{
	size_t stSize;
	DWORD dwCRC32;
} PyLibFiles_t;

std::unordered_map<std::string, PyLibFiles_t> PyLibFilesTable = {
	{ "lib/abc.pyc", { 6187, 3834771731 } },
	{ "lib/bisect.pyc", { 3236, 3116899751 } },
	{ "lib/codecs.pyc", { 36978, 2928014693 } },
	{ "lib/collections.pyc", { 26172, 385366131 } },
	{ "lib/copy.pyc", { 13208, 1091298715 } },
	{ "lib/copy_reg.pyc", { 5157, 536292604 } },
	{ "lib/fnmatch.pyc", { 3732, 4270526278 } },
	{ "lib/functools.pyc", { 6193, 3257285433 } },
	{ "lib/genericpath.pyc", { 3303, 1652596334 } },
	{ "lib/hashlib.pyc", { 6864, 249833099 } },
	{ "lib/heapq.pyc", { 13896, 2948659214 } },
	{ "lib/keyword.pyc", { 2169, 2178546341 } },
	{ "lib/linecache.pyc", { 3235, 4048207604 } },
	{ "lib/locale.pyc", { 49841, 4114662314 } },
	{ "lib/ntpath.pyc", { 11961, 2765879465 } },
	{ "lib/os.pyc", { 25769, 911432770 } },
	{ "lib/pyexpat.pyd", { 127488, 2778492911 } },
	{ "lib/pyexpat_d.pyd", { 194560, 2589182738 } },
	{ "lib/random.pyc", { 25491, 4021547204 } },
	{ "lib/re.pyc", { 13178, 1671609387 } },
	{ "lib/shutil.pyc", { 19273, 1873281015 } },
	{ "lib/site.pyc", { 20019, 3897044925 } },
	{ "lib/sre_compile.pyc", { 11107, 1620746411 } },
	{ "lib/sre_constants.pyc", { 6108, 3900811275 } },
	{ "lib/sre_parse.pyc", { 19244, 1459430047 } },
	{ "lib/stat.pyc", { 2791, 1375966108 } },
	{ "lib/string.pyc", { 19656, 1066063587 } },
	{ "lib/struct.pyc", { 234, 3060853334 } },
	{ "lib/sysconfig.pyc", { 17571, 1529083148 } },
	{ "lib/traceback.pyc", { 11703, 3768933732 } },
	{ "lib/types.pyc", { 2530, 920695307 } },
	{ "lib/UserDict.pyc", { 9000, 1431875928 } },
	{ "lib/warnings.pyc", { 13232, 3752454002 } },
	{ "lib/weakref.pyc", { 16037, 2124701469 } },
	{ "lib/_abcoll.pyc", { 22339, 2365844594 } },
	{ "lib/_locale.pyc", { 49841, 4114662314 } },
	{ "lib/_weakrefset.pyc", { 10490, 1576811346 } },
	{ "lib/__future__.pyc", { 4431, 2857792867 } },
};

bool checkPyLibDir(const std::string_view& szDirName)
{
	bool HasHack = false;

	char szDirNamePath[MAX_PATH];
	sprintf(szDirNamePath, "%s\\*", szDirName.data());

	WIN32_FIND_DATA f;
	const HANDLE h = FindFirstFile(szDirNamePath, &f);

	if (h == INVALID_HANDLE_VALUE)
		return HasHack;

	do
	{
		if (HasHack)
			break;
		const char* name = f.cFileName;

		if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
			continue;

		if (f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			const auto filePath = fmt::format("{}\\{}{}", szDirName.data(), "\\", name);
			PrintMe(1, "sub %s", filePath.c_str());
			checkPyLibDir(filePath);
		}
		else
		{
			// start processing file
			PrintMe(1, "starting %s", name);
			std::string sName(name);
			std::string sPathName = fmt::format("{}/{}", szDirName, name);
			// change \\ to /
			std::replace(sPathName.begin(), sPathName.end(), '\\', '/');
			PrintMe(1, "path %s", sPathName.c_str());
			// lower file name
			std::transform(sName.begin(), sName.end(), sName.begin(), ::tolower);
			{
				PrintMe(1, "verify %s", sName.c_str());
				bool isPyLibFound = false;
				auto it = PyLibFilesTable.find(sPathName);
				if (it != PyLibFilesTable.end())
				{
					PrintMe(1, "found %s==%s", it->first.c_str(), sName.c_str());
					DWORD dwCrc32 = GetFileCRC32(sPathName.c_str());
					// assert(dwCrc32);
					DWORD dwFileSize = f.nFileSizeLow;
					if (it->second.stSize != dwFileSize)
					{
						PrintMe(1, "wrong size %zu==%zu", it->second.stSize, dwFileSize);
						HasHack = true;
						PrintMe(0, "wrong size %zu for %s", dwFileSize, sPathName.c_str());
						return HasHack;
					}
					else if (it->second.dwCRC32 != dwCrc32)
					{
						PrintMe(1, "wrong crc32 %u==%u", it->second.dwCRC32, dwCrc32);
						HasHack = true;
						PrintMe(0, "wrong crc32 %u for %s", dwCrc32, sPathName.c_str());
						return HasHack;
					}
					PrintMe(1, "right size %zu==%zu", it->second.stSize, dwFileSize);
					PrintMe(1, "right crc32 %u==%u", it->second.dwCRC32, dwCrc32);
					PrintMe(2, "{{ \"%s\", %zu, %u}},", sPathName.c_str(), dwFileSize, dwCrc32);
					isPyLibFound = true;
				}
				// block ambiguous pyc/d files
				if (!isPyLibFound)
				{
					PrintMe(1, "not found %s", sName.c_str());
					if constexpr (PyLibCheckForce)
					{
						HasHack = true;
						PrintMe(0, "ambiguous file for %s", sPathName.c_str());
						return HasHack;
					}
				}
				PrintMe(1, "skipping file(%s) hack(%d) found(%d)", sName.c_str(), HasHack, isPyLibFound);
			}
		}

	} while (FindNextFile(h, &f));
	FindClose(h);
	return HasHack;
}

bool __CheckPyLibFiles()
{
	PrintMe(1, "__CheckPyLibFiles processing %s", PyLibFolder);
	if (checkPyLibDir(PyLibFolder))
		return false;
	return true;
}
#endif