#include "SigHandler.h"

#include "Relocation.h"


SigHandler::SigHandler() :
	_sigFuncMap()
{
	_sigFuncMap.insert({ "DirectSig", [](const char* a_sig) -> std::uintptr_t
	{
		DirectSig sig(a_sig);
		return sig.GetOffset();
	} });

	_sigFuncMap.insert({ "IndirectSig", [](const char* a_sig) -> std::uintptr_t
	{
		IndirectSig sig(a_sig);
		return sig.GetOffset();
	} });
}


auto SigHandler::operator()(const std::string& a_type)
-> std::optional<std::reference_wrapper<SigFunc_t>>
{
	auto it = _sigFuncMap.find(a_type);
	if (it != _sigFuncMap.end()) {
		return it->second;
	} else {
		return std::nullopt;
	}
}
