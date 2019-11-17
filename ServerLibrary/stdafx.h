#pragma once

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "mswsock.lib")  
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <Mswsock.h>

#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <ws2spi.h>
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
using namespace std;

#include "Definitions.h"
#include "Lock.h"
#include "ILog.h"
#include "RingBuffer.h"
#include "Connection.h"
#include "MessagePool.h"
#include "Performance.h"
#include "IocpServer.h"