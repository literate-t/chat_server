#include "../chat_server_library/Main.h"

int main()
{	
	chat_server_library::Main main;
	main.Init();
	thread chat_thread([&main](){main.Run(); });

	printf("Press any key to terminate the server\n");
	auto result = getchar();
	main.Stop();
	if (chat_thread.joinable())
	{
		chat_thread.join();
	}
	system("pause");
	return 0;
}