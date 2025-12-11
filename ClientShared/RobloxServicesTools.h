#pragma once
#include <string>
std::string trim_trailing_slashes(const std::string &path);
std::string GetSettingsUrl(const std::string &baseUrl, const std::string &group, const std::string &key);
std::string GetClientVersionUploadUrl(const std::string &baseUrl, const std::string &key);
std::string GetDmpUrl(const std::string &, bool changeToDataDomain = true);
std::string ReplaceTopSubdomain(const std::string& url, const char* newTopSubDoman);