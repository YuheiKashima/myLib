#include "Logger.h"

using namespace myLib;
using namespace std;

ofstream Logger::ms_LogStream;
string Logger::ms_LogColorString = {};
function<void(const string)> Logger::ms_LogOutputFunc;

/**
	@fn    Open
	@brief ログファイルを作成、記録を開始する
	@param _logFileName -Log_作成ログファイル名_作成日時.log
**/
void Logger::Open(const string _logFileName) {
	ms_LogStream.close();

	filesystem::create_directory("Log");
	string fileName = format("Log/Log_{}_{}.log", _logFileName, TimeStamp::GetTime_str());
	ofstream ofFile(fileName, ios::out);
	if (!ofFile)throw runtime_error("Log file could not be created.");

	ms_LogStream = move(ofFile);
}