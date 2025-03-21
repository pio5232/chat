#include "LibsPch.h"
#include "CParser.h"
#include "CParser.h"
#include <sstream>
#include <string>

C_Utility::CParser::CParser() : _fileName()
{
}

C_Utility::CParser::~CParser()
{
	CloseFile();
}

bool C_Utility::CParser::LoadFile(const char* fileName)
{
	CloseFile();

	_fileName = fileName;
	if (_fileName.find(".txt") == std::string::npos)
	{
		_fileName.append(".txt");
	}

	_fStream.open(_fileName, std::ios::in | std::ios::out);

	bool isOpen = _fStream.is_open();

	if (isOpen == false)
	{
		std::cout << "FileName [" << fileName << "] is not opened" << std::endl;
		return false;
	}

	std::cout << "FileName " << _fileName << " open" << std::endl;


	// eof or failed => false
	while (_fStream)
	{
		std::string line;
		std::getline(_fStream, line);

		if (line.empty())
			continue;

		std::istringstream lineStream(line);
		//       
		// [ key ] : [ value ]
		std::string key, value;
		std::getline(lineStream, key, ':');
		std::getline(lineStream, value);

		Trim(key);
		Trim(value);

		_items.insert(std::make_pair(key, value));

		std::cout << '[' << key << "] ==> [" << value << "]\n";

	}

	return isOpen;
}

void C_Utility::CParser::CloseFile()
{
	if (_fStream.is_open())
	{
		_fStream.close();

		std::cout << "File Close... [" << _fileName << "] \n";

		_fileName = "";

		_items.clear();

	}
}

bool C_Utility::CParser::GetDValue(const char* key, OUT double& dValue) const
{
	std::string strKey(key);

	auto findItem = _items.find(key);

	if (findItem == _items.end())
		return false;

	try
	{
		dValue = std::stod(findItem->second);
	}
	catch (std::invalid_argument invalidException)
	{
		std::cout << "stod Data Type is Not Matched (Double)\n";
	}
	return true;
}


bool C_Utility::CParser::GetIValue(const char* key, OUT int& iValue) const
{
	std::string strKey(key);

	auto findItem = _items.find(key);

	if (findItem == _items.end())
		return false;

	try
	{
		iValue = std::stoi(findItem->second);
	}
	catch (std::invalid_argument invalidException)
	{
		std::cout << "stoi Data Type is Not Matched (int)\n";
	}
	return true;
}

bool C_Utility::CParser::GetString(const char* key, OUT std::string& str)
{
	std::string strKey(key);

	auto findItem = _items.find(key);

	if (findItem == _items.end())
		return false;

	str = findItem->second;

	return true;
}

void C_Utility::Trim(std::string& str)
{
	int startIdx = str.find_first_not_of(" ");
	str.erase(0, startIdx);

	int lastIdx = str.find_last_not_of(" ") + 1;

	str.erase(lastIdx);
}
