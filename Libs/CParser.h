#pragma once
#include <fstream>
#include <unordered_map>

namespace C_Utility
{
	class CParser
	{
	public:
		CParser();
		~CParser();

		// true (success) / false (fail)
		bool LoadFile(const char* fileName);
		void CloseFile();

		bool GetDValue(const char* key, OUT double& dValue) const;
		bool GetIValue(const char* key, OUT int& iValue) const;
		bool GetString(const char* key, OUT std::string& str);
	private:

		std::fstream _fStream;
		std::string _fileName;

		// [key] : [value]
		std::unordered_map<std::string, std::string> _items;
	};

	void Trim(std::string& str);
}
