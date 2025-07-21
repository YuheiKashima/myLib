/**

	@file      Log.h
	@brief
	@details   ~
	@author    Yuhei Kashima
	@date      3.09.2024
	@copyright © Yuhei Kashima, 2024. All right reserved.

**/

#ifndef _LOG_
#define _LOG_

#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <source_location>
#include <format>
#include <mutex>
#include <mutex>

#include <TimeStamp/TimeStamp.h>
#pragma comment(lib,"TimeStamp.lib")

namespace myLib {
	/**

		@class   Log
		@brief
		@details ~ログ出力クラス

	**/
	class Logger {
	public:
		Logger() {}
		~Logger() {
			ms_LogStream.close();
			ms_LogOutputFunc = nullptr;
		}

		enum class ELoggingLevel {
			LOGLV_NONE = 0x00,//使用しない
			LOGLV_TRACE = 0x01,
			LOGLV_DEBUG = 0x02,
			LOGLV_INFO = 0x04,
			LOGLV_WARN = 0x08,
			LOGLV_ERROR = 0x10,
			LOGLV_FATAL = 0x20,
			LOBLV_ALL = LOGLV_TRACE | LOGLV_DEBUG | LOGLV_INFO | LOGLV_WARN | LOGLV_ERROR | LOGLV_FATAL
		};

		/**
			@fn    Open
			@brief ログファイルを作成、記録を開始する
			@param _logFileName -Log_作成ログファイル名_作成日時.log
		**/
		static void Open(const std::string _logFileName);

		/**
			@fn     Logging
			@brief
			@tparam Args       - template parameter pack type
			@param  _log       -
			@param  _logFormat -
			@retval            -
		**/
		template <typename... Args>
		static constexpr std::string Logging(const std::format_string<Args...> _log, Args&&... _logFormat) {
			if (!ms_LogStream)return "";

			ms_LogMutex.lock();

			std::stringstream strstr;

			strstr << std::format("{}: {}", TimeStamp::GetTime_str(), std::format(_log, std::forward<Args>(_logFormat)...)) << std::endl;

			ms_LogStream << strstr.str();

			if (ms_LogOutputFunc)
				ms_LogOutputFunc(strstr.str());

			if (!ms_LogColorString.empty())
				std::cout << ms_LogColorString;

			std::cout << strstr.str();

			if (!ms_LogColorString.empty()) {
				std::cout << "\x1b[m";
				ms_LogColorString.clear();
			}

			std::string dest = strstr.str();
			ms_LogMutex.unlock();

			return dest;
		}

		/**
			@fn     Logging
			@brief
			@tparam Args       - template parameter pack type
			@param  _level     -
			@param  _log       -
			@param  _logFormat -
			@retval            -
		**/
		template <typename... Args>
		static constexpr std::string Logging(const ELoggingLevel _level, const std::format_string<Args...> _log, Args&&... _logFormat) {
			if (!ms_LogStream)return "";

			ms_LogMutex.lock();

			std::stringstream strstr;
			switch (_level) {
			case ELoggingLevel::LOGLV_TRACE:
				ms_LogColorString = "\x1b[38;2;255;255;255m";
				strstr << "[Trace]";
				break;
			case ELoggingLevel::LOGLV_DEBUG:
				ms_LogColorString = "\x1b[38;2;0;255;0m";
				strstr << "[Debug]";
				break;
			case ELoggingLevel::LOGLV_INFO:
				ms_LogColorString = "\x1b[38;2;0;0;255m";
				strstr << "[Info]";
				break;
			case ELoggingLevel::LOGLV_WARN:
				ms_LogColorString = "\x1b[38;2;255;254;59m";
				strstr << "[Warning]";
				break;
			case ELoggingLevel::LOGLV_ERROR:
				ms_LogColorString = "\x1b[38;2;255;0;0m";
				strstr << "[Error]";
				break;
			case ELoggingLevel::LOGLV_FATAL:
				ms_LogColorString = "\x1b[48;2;255;0;0m";
				strstr << "[Fatal]";
				break;
			default:
				throw std::runtime_error("An ELoggingLevel outside the defined range is used.");
			}

			std::string dest = Logging("{}{}", strstr.str(), std::format(_log, std::forward<Args>(_logFormat)...));

			ms_LogMutex.unlock();

			return dest;
		}

		/**
			@fn     Logging
			@brief
			@tparam Args            - template parameter pack type
			@param  _level          -
			@param  _sourceLocation -
			@param  _log            -
			@param  _logFormat      -
			@retval                 -
		**/
		template <typename... Args>
		static constexpr std::string Logging(const ELoggingLevel _level, const std::source_location _sourceLocation, const std::format_string<Args...> _log, Args&&... _logFormat) {
			if (!ms_LogStream)return "";

			ms_LogMutex.lock();

			std::stringstream strstr;
			const std::string fileDir = std::format("{}", _sourceLocation.file_name());
			const int32_t pos = fileDir.find_last_of("\\");
			strstr << std::format(" {} [Locate {}:Line {}] ", fileDir.substr(pos + 1), _sourceLocation.function_name(), _sourceLocation.line());

			std::string dest = Logging(_level, "{}{}", strstr.str(), std::format(_log, std::forward<Args>(_logFormat)...));

			ms_LogMutex.unlock();
			return dest;
		}
		/**
			@fn     is_Open
			@brief
			@retval  -
		**/
		static bool is_Open() { return ms_LogStream.is_open(); }

		/**
			@fn		SetLogOutputCallback
			@brief	ログテキストをライブラリ外部に送る為のコールバック関数を設定する
			@param	_logCallbackFunc -ログコールバック関数 -void (const std::string)
		**/
		static void SetLogOutputCallback(std::function<void(const std::string)> _logCallbackFunc) { ms_LogOutputFunc = _logCallbackFunc; }
	private:
		static std::ofstream ms_LogStream;
		static std::string ms_LogColorString;
		static std::function<void(const std::string)> ms_LogOutputFunc;
		static std::recursive_mutex ms_LogMutex;
	};
}

#endif