#include "../ChatServerLibrary/Main.h"

int main()
{
	ChatServerLibrary::Main main;
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