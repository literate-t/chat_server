#pragma once
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")
#pragma comment(lib, "Winmm.lib")

#include <WS2tcpip.h>
#include <WinSock2.h>
#include <MSWSock.h>
#include <mmsystem.h>
#include <WS2spi.h>
#include <mstcpip.h>
// 위의 헤더와 라이브러리가 없으면 오류 발생

#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <winbase.h>
#include <thread>
#include <vector>
#include <unordered_map>
#include <queue>
#include <concurrent_queue.h>
#include <list>
#include <fstream>
#include <math.h>
#include <iostream>
#include <functional>
using namespace std;

#include "../server_library/IocpServer.h"
#include "../server_library/ILog.h"
#include "Definitions.h"
#include "ErrorCode.h"
#include "Packet.h"
#include "User.h"
#include "UserManager.h"
#include "Lobby.h"
#include "LobbyManager.h"
#include "Room.h"
#include "PacketManager.h"
#include "Main.h"