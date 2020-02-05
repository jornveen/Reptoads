#include "resource_loading/TtfHeaderParser.h"
#include "core/Assertion.h"
#include <fstream>
#include <cstdint>
#include "memory/string.h"

typedef uint32_t DWORD;   // DWORD = unsigned 32 bit value
typedef uint16_t WORD;    // WORD = unsigned 16 bit value
typedef uint8_t BYTE;     // BYTE = unsigned 8 bit value

namespace tbsg
{
	static bool EndsWith(const ptl::string& a_str, const ptl::string a_ext)
	{
		return a_str.size() >= a_ext.size() &&
			a_str.compare(a_str.size() - a_ext.size(), a_ext.size(), a_ext) == 0;
	}

	static int FileExists(const ptl::string& fname)
	{
		std::ifstream infile(fname.c_str());
		return infile.good();
	}

	typedef uint32_t       DWORD;
	typedef int                 BOOL;
	typedef unsigned char       BYTE;
	typedef unsigned short      WORD;
	typedef float               FLOAT;
	typedef FLOAT               *PFLOAT;
	typedef BOOL            *PBOOL;
	typedef BOOL             *LPBOOL;
	typedef BYTE            *PBYTE;
	typedef BYTE             *LPBYTE;
	typedef int             *PINT;
	typedef int              *LPINT;
	typedef WORD            *PWORD;
	typedef WORD             *LPWORD;
	typedef long             *LPLONG;
	typedef DWORD           *PDWORD;
	typedef DWORD            *LPDWORD;
	typedef void             *LPVOID;
	typedef const void       *LPCVOID;

	typedef int                 INT;
	typedef unsigned int        UINT;
	typedef unsigned int        *PUINT;

	using USHORT = unsigned short;
	using ULONG = uint32_t;
	using LONG = long;

	typedef int64_t LONG_PTR, *PLONG_PTR;

	typedef uint64_t ULONG_PTR, *PULONG_PTR;
	typedef ULONG_PTR DWORD_PTR;

	//
	// The following types are guaranteed to be signed and 64 bits wide.
	//

	typedef int64_t LONG64, *PLONG64;


	//
	// The following types are guaranteed to be unsigned and 64 bits wide.
	//

	typedef uint64_t ULONG64, *PULONG64;
	typedef uint64_t DWORD64, *PDWORD64;

	typedef struct _tagTT_OFFSET_TABLE {
		USHORT	uMajorVersion;
		USHORT	uMinorVersion;
		USHORT	uNumOfTables;
		USHORT	uSearchRange;
		USHORT	uEntrySelector;
		USHORT	uRangeShift;
	}TT_OFFSET_TABLE;

	static_assert(sizeof(_tagTT_OFFSET_TABLE) == (2 * 6), "");


	//__attribute__((packed))
	struct TT_TABLE_DIRECTORY {
		char	szTag[4];			//table name
		ULONG	uCheckSum;			//Check sum
		ULONG	uOffset;			//Offset from beginning of file
		ULONG	uLength;			//length of the table in bytes
	}//__attribute__((packed))
	;

	//static_assert(sizeof(TT_TABLE_DIRECTORY) == (1 * 4 + 3 * 8), "_tagTT_TABLE_DIRECTORY is not a correct size");


	typedef struct _tagTT_NAME_TABLE_HEADER {
		USHORT	uFSelector;			//format selector. Always 0
		USHORT	uNRCount;			//Name Records count
		USHORT	uStorageOffset;		//Offset for strings storage, from start of the table
	}TT_NAME_TABLE_HEADER;

	static_assert(sizeof(_tagTT_NAME_TABLE_HEADER) == (3*2), "");


	typedef struct _tagTT_NAME_RECORD {
		USHORT	uPlatformID;
		USHORT	uEncodingID;
		USHORT	uLanguageID;
		USHORT	uNameID;
		USHORT	uStringLength;
		USHORT	uStringOffset;	//from start of storage area
	}TT_NAME_RECORD;

	static_assert(sizeof(_tagTT_NAME_RECORD) == (2 * 6), "");


#define MAKEWORD(a, b)      ((WORD)(((BYTE)(((DWORD_PTR)(a)) & 0xff)) | ((WORD)((BYTE)(((DWORD_PTR)(b)) & 0xff))) << 8))
#define MAKELONG(a, b)      ((LONG)(((WORD)(((DWORD_PTR)(a)) & 0xffff)) | ((DWORD)((WORD)(((DWORD_PTR)(b)) & 0xffff))) << 16))
#define LOWORD(l)           ((WORD)(((DWORD_PTR)(l)) & 0xffff))
#define HIWORD(l)           ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))
#define LOBYTE(w)           ((BYTE)(((DWORD_PTR)(w)) & 0xff))
#define HIBYTE(w)           ((BYTE)((((DWORD_PTR)(w)) >> 8) & 0xff))

#define SWAPWORD(x)		MAKEWORD(HIBYTE(x), LOBYTE(x))
#define SWAPLONG(x)		MAKELONG(SWAPWORD(HIWORD(x)), SWAPWORD(LOWORD(x)))

	//Code from: https://www.codeproject.com/Articles/2293/Retrieving-Font-Name-from-TTF-File
	ptl::string GetFontNameFromTtf(const ptl::string& pathToTtf)
	{
		std::cout << "sizeee: " << sizeof(uint32_t) << std::endl;

		bool endsWith = EndsWith(pathToTtf, ".ttf");
		bool exists = FileExists(pathToTtf);
		ASSERT_MSG(endsWith, "Given path does not give a .ttf file!");
		ASSERT_MSG(exists, "Cannot find file specified!");
		if(!endsWith || !exists) {
			std::cerr << "ERROR Cannot open .ttf file specified! '" << pathToTtf << "'" << std::endl;
			return "";
		}

		std::ifstream f;
		//CFile f;
		ptl::string csRetVal;

		f.open(pathToTtf.c_str(), static_cast<int>(std::ifstream::in));
		{
			TT_OFFSET_TABLE ttOffsetTable;
			f.read(reinterpret_cast<char*>(&ttOffsetTable), sizeof(TT_OFFSET_TABLE));
			ttOffsetTable.uNumOfTables = SWAPWORD(ttOffsetTable.uNumOfTables);
			ttOffsetTable.uMajorVersion = SWAPWORD(ttOffsetTable.uMajorVersion);
			ttOffsetTable.uMinorVersion = SWAPWORD(ttOffsetTable.uMinorVersion);

			//check is this is a true type font and the version is 1.0
			if (ttOffsetTable.uMajorVersion != 1 || ttOffsetTable.uMinorVersion != 0)
				return csRetVal;

			TT_TABLE_DIRECTORY tblDir;
			BOOL bFound = false;
			ptl::string csTemp;

			for (int i = 0; i < ttOffsetTable.uNumOfTables; i++) {
				f.read(reinterpret_cast<char*>(&tblDir), sizeof(TT_TABLE_DIRECTORY));
				csTemp.resize(5);
				strncpy(&csTemp[0], tblDir.szTag, 4);
				csTemp.resize(4);
				if (csTemp == "name") {
					bFound = true;
					tblDir.uLength = SWAPLONG(tblDir.uLength);
					tblDir.uOffset = SWAPLONG(tblDir.uOffset);
					break;
				}
			}

			if (bFound) {
				f.seekg(tblDir.uOffset, std::ifstream::beg);
				TT_NAME_TABLE_HEADER ttNTHeader;
				f.read(reinterpret_cast<char*>(&ttNTHeader), sizeof(TT_NAME_TABLE_HEADER));
				ttNTHeader.uNRCount = SWAPWORD(ttNTHeader.uNRCount);
				ttNTHeader.uStorageOffset = SWAPWORD(ttNTHeader.uStorageOffset);
				TT_NAME_RECORD ttRecord;
				bFound = false;

				for (int i = 0; i < ttNTHeader.uNRCount; i++) {
					f.read(reinterpret_cast<char*>(&ttRecord), sizeof(TT_NAME_RECORD));
					ttRecord.uNameID = SWAPWORD(ttRecord.uNameID);
					if (ttRecord.uNameID == 1) {
						ttRecord.uStringLength = SWAPWORD(ttRecord.uStringLength);
						ttRecord.uStringOffset = SWAPWORD(ttRecord.uStringOffset);
						int nPos = f.tellg();
						f.seekg(tblDir.uOffset + ttRecord.uStringOffset + ttNTHeader.uStorageOffset, std::ifstream::beg);

						//bug fix: see the post by SimonSays to read more about it
						csTemp.resize(ttRecord.uStringLength + 1);
						//TCHAR lpszNameBuf = csTemp.GetBuffer(ttRecord.uStringLength + 1);
						//ZeroMemory(&csTemp[0], ttRecord.uStringLength + 1);
						f.read(&csTemp[0], ttRecord.uStringLength);
						//std::cout << i << " @ " << ": '" << csTemp << "'\n";

						//csTemp.ReleaseBuffer();


						//if(/*strlen(csTemp.data())*/csTemp.size() > 0){
						csTemp.erase(std::remove(csTemp.begin(), csTemp.end(), '\0'), csTemp.end());

						csRetVal = csTemp;
						break;
						//}
						f.seekg(nPos, std::ifstream::beg);
					}
				}
			}
			f.close();
		}
		return csRetVal;



		//std::ifstream ttfStream{ pathToTtf.c_str(), std::ifstream::binary };


		//TT_OFFSET_TABLE ttOffsetTable;

		//ttfStream.read(reinterpret_cast<char*>(&ttOffsetTable), sizeof(ttOffsetTable));
		//ttOffsetTable.uNumOfTables = SWAPWORD(ttOffsetTable.uNumOfTables);
		//ttOffsetTable.uMajorVersion = SWAPWORD(ttOffsetTable.uMajorVersion);
		//ttOffsetTable.uMinorVersion = SWAPWORD(ttOffsetTable.uMinorVersion);

		//ASSERT(ttOffsetTable.uMajorVersion == 1 && ttOffsetTable.uMinorVersion == 0);
		//if (ttOffsetTable.uMajorVersion != 1 || ttOffsetTable.uMinorVersion != 0) {
		//	std::cerr << "ERROR ttf major version isn't 1 or ttf minor version is not 0'" << pathToTtf << "'" << std::endl;
		//	return "";
		//}

		//TT_TABLE_DIRECTORY tblDir;
		//bool bFound = false;
		//ptl::string csTemp;
		//csTemp.resize(4);

		//for (int i = 0; i < ttOffsetTable.uNumOfTables; i++) {
		//	ttfStream.read(reinterpret_cast<char*>(&tblDir), sizeof(TT_TABLE_DIRECTORY));
		//	strncpy(&csTemp.at(0), tblDir.szTag, 4);
		//	if (csTemp == "name") {
		//		bFound = true;
		//		tblDir.uLength = SWAPLONG(tblDir.uLength);
		//		tblDir.uOffset = SWAPLONG(tblDir.uOffset);
		//		break;
		//	}
		//}
		//if (!bFound) {
		//	ASSERT(false);
		//	std::cerr << "ERROR could not find a name offset table in .tff font file!!! '" << pathToTtf << "'" << std::endl;
		//	return "";
		//}

		//ttfStream.seekg(tblDir.uOffset, std::ifstream::beg);

		//TT_NAME_TABLE_HEADER ttNTHeader;
		//ttfStream.read(reinterpret_cast<char*>(&ttNTHeader), sizeof(TT_NAME_TABLE_HEADER));
		//ttNTHeader.uNRCount = SWAPWORD(ttNTHeader.uNRCount);
		//ttNTHeader.uStorageOffset = SWAPWORD(ttNTHeader.uStorageOffset);
		//TT_NAME_RECORD ttRecord;

		//for (int i = 0; i < ttNTHeader.uNRCount; i++) {
		//	ttfStream.read(reinterpret_cast<char*>(&ttRecord), sizeof(TT_NAME_RECORD));
		//	ttRecord.uNameID = SWAPWORD(ttRecord.uNameID);
		//	if (ttRecord.uNameID != 0)
		//		int i = 3;
		//		
		//	ttRecord.uStringLength = SWAPWORD(ttRecord.uStringLength);
		//	ttRecord.uStringOffset = SWAPWORD(ttRecord.uStringOffset);
		//	int nPos = ttfStream.tellg();
		//	auto offset = tblDir.uOffset + ttRecord.uStringOffset + ttNTHeader.uStorageOffset;
		//	ttfStream.seekg(offset, std::ifstream::beg);

		//	//bug fix: see the post by SimonSays to read more about it
		//	ptl::string testString;
		//	testString.resize(ttRecord.uStringLength + 1, '\0');
		//	//csTemp.clear();
		//	//csTemp.resize(ttRecord.uStringLength + 1, '\0');
		//	//TCHAR lpszNameBuf = csTemp.GetBuffer(ttRecord.uStringLength + 1);
		//	//ZeroMemory(lpszNameBuf, ttRecord.uStringLength + 1);
		//	ttfStream.read(&testString[0], ttRecord.uStringLength);
		//	std::cout << i << " @ " << offset << ": " << testString << "\n";

		//		
		//	if (ttRecord.uNameID == 1) {
		//		auto countedLen = strlen(csTemp.c_str());
		//		if (countedLen > 0) {
		//			csTemp.resize(countedLen + 1);
		//			return csTemp;
		//		}

		//		ttfStream.seekg(nPos, std::ifstream::beg);
		//	}
		//}

		////ASSERT_MSG(false, "ERROR: Could not find name!!!");
		//std::cerr << "ERROR could not find a nameId in .tff font file!!! '" << pathToTtf << "'" << std::endl;
		//return "";
	}
}
