#include "stdafx.h"
#include "ComModule.h"

#include "SharedHelpers.h"

ComModule::PFNREGISTERTYPELIB ComModule::registerTypeLibFunc = NULL;
bool ComModule::regPerUser = false;

ComModule::ComModule()
{
}

bool ComModule::Init(bool perUser)
{
	//NOT thread safe, but this code is single threaded anyway
	if (registerTypeLibFunc == NULL)
	{
		if (perUser)
		{
			HMODULE hmodOleAut=::GetModuleHandleW(L"OLEAUT32.DLL");
			if (hmodOleAut)
			{
				registerTypeLibFunc = reinterpret_cast<PFNREGISTERTYPELIB>(::GetProcAddress(hmodOleAut, "RegisterTypeLibForUser"));
			}
			if (!registerTypeLibFunc)
			{
				perUser = false;
			}
		}
		
		if (!perUser)
		{
			registerTypeLibFunc = &RegisterTypeLib;
		}

		regPerUser = perUser;
	}

	return perUser;
}


ComModule::Interface::Interface(const char* name, const GUID& guid, bool isConnectionPoint):name(name),isConnectionPoint(isConnectionPoint)
{
	OLECHAR s[256];
	::StringFromGUID2(guid, s, 256);
	this->guid = convert_w2s(CString(s).GetString());
	if (this->guid.empty())
		throw std::runtime_error(format_string("Bad guid for %s", name));
}

void ComModule::registerModule(CRegKey &classesKey, std::wstring versionDir, simple_logger<wchar_t> &logger)
{
	// First unregister the module in hopes of invalidating any cached info in DCOM
	unregisterModule(classesKey, regPerUser, logger);

	std::wstring modulePath = versionDir + moduleName;

	LOG_ENTRY1("registerModule %s", appName.c_str());

	{
		auto keyAppID = CreateKey(classesKey, _T("AppID"), NULL, is64Bits);
		CreateKey(keyAppID->m_hKey, appID.c_str(), convert_s2w(appName).c_str(), is64Bits);
		{
			auto keyAppIDFileName = CreateKey(keyAppID->m_hKey, fileName.c_str(), NULL, is64Bits);
			throwHRESULT(keyAppIDFileName->SetStringValue(_T("AppID"), appID.c_str()), format_string("failed to set AppID\\%S:AppID string value", appID.c_str()));
		}
	}

	{
		auto keyRobloxAppApp1 = CreateKey(classesKey, convert_s2w(progID).c_str(), name.c_str(), is64Bits);
		CreateKey(keyRobloxAppApp1->m_hKey, _T("CLSID"), clsid.c_str(), is64Bits);
	}

	{
		auto keyRobloxAppApp = CreateKey(classesKey, convert_s2w(versionIndependentProgID).c_str(), name.c_str(), is64Bits);
		CreateKey(keyRobloxAppApp->m_hKey, _T("CLSID"), clsid.c_str(), is64Bits);
		CreateKey(keyRobloxAppApp->m_hKey, _T("CurVer"), convert_s2w(progID).c_str(), is64Bits);
	}
	{
		auto clsidKey = CreateKey(classesKey, _T("CLSID"), NULL, is64Bits);

		auto appKey = CreateKey(clsidKey->m_hKey, clsid.c_str(), name.c_str(), is64Bits);

		CreateKey(appKey->m_hKey, _T("TypeLib"), convert_s2w(typeLib).c_str(), is64Bits);
		CreateKey(appKey->m_hKey, _T("ProgID"), convert_s2w(progID).c_str(), is64Bits);
		CreateKey(appKey->m_hKey, _T("VersionIndependentProgID"), convert_s2w(versionIndependentProgID).c_str(), is64Bits);
		CreateKey(appKey->m_hKey, _T("Programmable"), NULL, is64Bits);
		if (isDll)
		{
			auto keyInprocServer32 = CreateKey(appKey->m_hKey, _T("InprocServer32"), modulePath.c_str(), is64Bits);
			throwHRESULT(keyInprocServer32->SetStringValue(_T("ThreadingModel"), _T("Apartment")), "failed to set ThreadingModel string value");
		}
		else
			CreateKey(appKey->m_hKey, _T("LocalServer32"), modulePath.c_str(), is64Bits);
	}

	if (false)
	{
		LOG_ENTRY1("LoadTypeLib %S", modulePath.c_str());

		// See https://msdn.microsoft.com/en-us/library/bb756926.aspx regarding admin-level running
		CComPtr<ITypeLib> typeLib;
		CComBSTR s(modulePath.c_str());
		throwHRESULT(::LoadTypeLib(s, &typeLib), format_string("LoadTypeLib failed for %S", modulePath.c_str()).c_str());

		LOG_ENTRY1("registerTypeLibFunc %S", modulePath.c_str());

		throwHRESULT(registerTypeLibFunc(typeLib, s, NULL), format_string("registerTypeLibFunc failed for %S", modulePath.c_str()));
	}
	else
	{
		{
			auto key = CreateKey(classesKey, _T("TypeLib"), NULL, is64Bits);
			auto typeLibKey = CreateKey(key->m_hKey, convert_s2w(typeLib).c_str(), NULL, is64Bits);
			std::string typeLibName = appName + " " + typeLibVersion + " Type Library";
			auto typeLibVKey = CreateKey(typeLibKey->m_hKey, convert_s2w(typeLibVersion).c_str(), convert_s2w(typeLibName).c_str(), is64Bits);
			// ngl no idea what this is
			auto temp = CreateKey(typeLibVKey->m_hKey, _T("0"), NULL, is64Bits);
			CreateKey(temp->m_hKey, _T("win32"), modulePath.c_str(), is64Bits);
			CreateKey(typeLibVKey->m_hKey, _T("FLAGS"), _T("0"), is64Bits);
			CreateKey(typeLibVKey->m_hKey, _T("HELPDIR"), versionDir.c_str(), is64Bits);
		}
		{
			auto interfaceKey = CreateKey(classesKey, _T("Interface"), NULL, is64Bits);
			for (ComModule::Interfaces::const_iterator iter = interfaces.begin(); iter!=interfaces.end(); ++iter)
			{
				auto key = CreateKey(interfaceKey->m_hKey, convert_s2w(iter->guid).c_str(), convert_s2w(iter->name).c_str(), is64Bits);
				CreateKey(key->m_hKey, _T("ProxyStubClsid"), iter->isConnectionPoint ? _T("{00020420-0000-0000-C000-000000000046}") : _T("{00020424-0000-0000-C000-000000000046}"), is64Bits);
				CreateKey(key->m_hKey, _T("ProxyStubClsid32"), iter->isConnectionPoint ? _T("{00020420-0000-0000-C000-000000000046}") : _T("{00020424-0000-0000-C000-000000000046}"), is64Bits);
				auto typelibKey = CreateKey(key->m_hKey, _T("TypeLib"), convert_s2w(typeLib).c_str(), is64Bits);
				throwHRESULT(typelibKey->SetStringValue(_T("Version"), convert_s2w(typeLibVersion).c_str()), format_string("failed to set TypeLib\\%s:Version string value", typeLib.c_str()));
			}
		}
	}

	LOG_ENTRY1("registerModule %s done", appName.c_str());
}

void ComModule::unregisterModule(HKEY ckey, bool isPerUser, simple_logger<wchar_t> &logger)
{
	LOG_ENTRY2("unregisterModule %s%s", (isPerUser ? "HKCU " : "HKLM "), appName.c_str());

	DeleteKey(logger, ckey, convert_s2w(progID).c_str());
	DeleteKey(logger, ckey, convert_s2w(versionIndependentProgID).c_str());

	// https://blogs.msdn.com/larryosterman/archive/2006/01/09/510856.aspx
	DeleteSubKey(logger, ckey, _T("Typelib"), convert_s2w(typeLib).c_str());
	DeleteSubKey(logger, ckey, _T("CLSID"), clsid.c_str());
	DeleteSubKey(logger, ckey, _T("CLSID"), convert_s2w(versionIndependentProgID).c_str());
	DeleteSubKey(logger, ckey, _T("AppID"), appID.c_str());
	DeleteSubKey(logger, ckey, _T("AppID"), fileName.c_str());
	for (ComModule::Interfaces::const_iterator iter = interfaces.begin(); iter!=interfaces.end(); ++iter)
		DeleteSubKey(logger, ckey, _T("Interface"), convert_s2w(iter->guid).c_str());

}

