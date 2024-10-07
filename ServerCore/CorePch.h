#pragma once
//PreCompiled Header(Pch) 미리 컴파일 된 헤더

#include "Types.h" 
#include "CoreGlobal.h"
#include "CoreTLS.h"
#include "CoreMacro.h"
#include "Container.h"
#include "SendBuffer.h"

#include <WinSock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib,"ws2_32.lib")

#include <iostream>


#include "Lock.h"
#include "ObjectPool.h"
#include "Memory.h"

using namespace std;