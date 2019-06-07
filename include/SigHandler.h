#pragma once

#include <functional>
#include <optional>
#include <string>
#include <unordered_map>


class SigHandler
{
public:
	using SigFunc_t = std::function<std::uintptr_t(const char*)>;

	SigHandler();
	SigHandler(const SigHandler&) = default;
	SigHandler(SigHandler&&) = default;
	~SigHandler() = default;

	SigHandler& operator=(const SigHandler&) = default;
	SigHandler& operator=(SigHandler&&) = default;

	std::optional<std::reference_wrapper<SigFunc_t>> operator()(const std::string& a_type);

private:
	std::unordered_map<std::string, SigFunc_t> _sigFuncMap;
};
