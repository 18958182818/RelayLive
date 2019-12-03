#include "uv.h"
#include "util.h"
#include "tmcp_interface_sdk.h"

#pragma comment(lib,"tmcp_interface_sdk.lib")

using namespace Platform ;

namespace HikPlat {

    struct cameraInfo {
        std::string id;
        std::string name;
    };
    static vector<cameraInfo> _cameras;

	static int _loginHandle = 0;
    static uv_mutex_t _mutex;

    static void GetDevices() {
        int ret = Plat_QueryResource(CAMERA, _loginHandle);
        if(ret < 0) {
            Log::debug("query camera failed");
            return;
        }

        uv_mutex_lock(&_mutex);
        do {
            cameraInfo ca;
            ca.id   = Plat_GetValueStr(Camera::device_id, _loginHandle);
            ca.name = Plat_GetValueStr(Camera::device_name, _loginHandle);
            _cameras.push_back(ca);
        } while (Plat_MoveNext(_loginHandle)==0);
        uv_mutex_unlock(&_mutex);

        Log::debug("camera num %d", _cameras.size());
    }

	bool Init() {
		int ret = Plat_Init();
		if(ret < 0) {
			Log::error("SDK init failed");
			return false;
		}

        string strIP = Settings::getValue("HikSDK", "IP");
        string strUsr = Settings::getValue("HikSDK", "User");
        string strPwd = Settings::getValue("HikSDK", "Pwd");
        string strPort= Settings::getValue("HikSDK", "Port");
		_loginHandle = Plat_LoginCMS(strIP.c_str(), strUsr.c_str(), strPwd.c_str(), strPort.c_str());
		if(_loginHandle <= 0) {
			Log::error("login failed %d", Plat_GetLastError());
			return false;
		}

        uv_mutex_init(&_mutex);
        
		return true;
	}

	void Cleanup() {
		if(_loginHandle > 0) {
			Plat_LogoutCMS(_loginHandle);
			_loginHandle = 0;
		}

		Plat_Free();

        uv_mutex_destroy(&_mutex);
	}

    string GetDevicesJson() {
        string strResJson = "{\"root\":[";
        bool bfirst = true;
        for (auto dev:_cameras) {
            if(!bfirst) {
                strResJson += ",";
            }
            strResJson += "{\"DeviceID\":\"";
            strResJson += dev.id;
            strResJson += "\",\"Name\":\"";
            strResJson += EncodeConvert::AtoUTF8(dev.name);
            strResJson += "\"}";
        }
        strResJson += "]}";

        return strResJson;
    }

    void DeviceControl(string strDev, int nInOut = 0, int nUpDown = 0, int nLeftRight = 0) {

    }
}