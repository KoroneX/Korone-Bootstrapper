#pragma once

#include <boost/enable_shared_from_this.hpp>
#include <functional>

class ShutdownRobloxApp : public boost::enable_shared_from_this<ShutdownRobloxApp>
{
	int timeoutInSeconds;
	typedef std::function<bool(int, int)> Callback;
	Callback callback;
	HINSTANCE hInstance;
	std::function<HWND()> parent;
	bool result;
	std::wstring appExeName;
public:
	ShutdownRobloxApp(HINSTANCE hInstance, std::wstring name, std::function<HWND()> parent, int timeoutInSeconds, Callback callback):hInstance(hInstance), appExeName(name), parent(parent),timeoutInSeconds(timeoutInSeconds),callback(callback) {}
	~ShutdownRobloxApp(void);
	bool run();
private:
	void terminateApp(DWORD pid);
};
