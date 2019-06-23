#include "skse64_common/skse_version.h"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

#include <ShlObj.h>

#include "SigHandler.h"
#include "version.h"

#include "SKSE/API.h"


namespace
{
	constexpr char EXE_VERSION[] = "// 1_5_73";


	void UpdateOffset(std::string& a_line, std::uintptr_t a_newOffset)
	{
		auto beg = a_line.find_first_of('=');
		if (beg == std::string::npos) {
			_ERROR("[ERROR] Could not find beginning of offset to update");
			return;
		}

		auto end = a_line.find_first_of(';');
		if (end == std::string::npos) {
			_ERROR("[ERROR] Could not find end of offset to update");
			return;
		}

		char subStr[] = "= 0x________";
		std::snprintf(subStr, sizeof(subStr), "= 0x%08tX", a_newOffset);
		a_line.replace(beg, end - beg, subStr);

		beg = a_line.find("//", end + 1);
		if (beg == std::string::npos) {
			_ERROR("[ERROR] Could not find beginning of comment to update");
			return;
		}
		end = a_line.size() - 1;
		a_line.replace(beg, end - beg + 1, EXE_VERSION);
	}


	void ScanForSigs()
	{
		SigHandler sigHandler;

		std::ifstream inFile("input.txt");
		if (!inFile.is_open()) {
			_ERROR("[ERROR] Failed to open input file!\n");
			return;
		}

		std::ofstream outFile("output.txt");
		if (!outFile.is_open()) {
			_ERROR("[ERROR] Failed to open output file!\n");
			return;
		}

		std::string line;
		while (std::getline(inFile, line)) {
			outFile << line << '\n';

			auto beg = line.find("//");
			if (beg == std::string::npos) {
				continue;
			}

			beg += 2;
			while (line[beg] == ' ') {
				++beg;
			}
			auto end = line.find_first_of(':', beg + 1);
			if (end == std::string::npos) {
				continue;
			}

			std::string type(line, beg, end - beg);
			auto sigFunc = sigHandler(type);
			if (!sigFunc) {
				// no func for given type
				continue;
			}

			beg = end;
			do {
				++beg;
			} while (line[beg] == ' ');
			end = line.size() - 1;

			std::string sig(line, beg, end - beg + 1);
			auto offset = (*sigFunc)(sig.c_str());
			if (offset != 0xDEADBEEF) {
				std::string nextLine;
				if (std::getline(inFile, nextLine)) {
					UpdateOffset(nextLine, offset);
					outFile << nextLine << '\n';
				}
			}
		}
	}


	void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
	{
		switch (a_msg->type) {
		case SKSE::MessagingInterface::kInputLoaded:
			ScanForSigs();
			break;
		}
	}
}


extern "C" {
	bool SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
	{
		SKSE::Logger::OpenRelative(FOLDERID_Documents, L"\\My Games\\Skyrim Special Edition\\SKSE\\OffsetUpdater.log");
		SKSE::Logger::SetPrintLevel(SKSE::Logger::Level::kDebugMessage);
		SKSE::Logger::SetFlushLevel(SKSE::Logger::Level::kDebugMessage);

		_MESSAGE("OffsetUpdater v%s", OFST_VERSION_VERSTRING);

		a_info->infoVersion = SKSE::PluginInfo::kVersion;
		a_info->name = "OffsetUpdater";
		a_info->version = OFST_VERSION_MAJOR;

		if (a_skse->IsEditor()) {
			_FATALERROR("[FATAL ERROR] Loaded in editor, marking as incompatible!\n");
			return false;
		} else if (a_skse->RuntimeVersion() != RUNTIME_VERSION_1_5_73) {
			_FATALERROR("[FATAL ERROR] Unsupported runtime version %08X!\n", a_skse->RuntimeVersion());
			return false;
		}

		return true;
	}


	bool SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
	{
		_MESSAGE("[MESSAGE] OffsetUpdater loaded");

		if (!SKSE::Init(a_skse)) {
			return false;
		}

		auto messaging = SKSE::GetMessagingInterface();
		if (messaging->RegisterListener("SKSE", MessageHandler)) {
			_MESSAGE("[MESSAGE] Registered SKSE listener");
		} else {
			_FATALERROR("[FATAL ERROR] Failed to register SKSE listener!\n");
			return false;
		}

		return true;
	}
};
