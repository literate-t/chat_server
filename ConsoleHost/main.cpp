#include "../ChatServerLibrary/Main.h"

int main()
{
	ChatServerLibrary::Main main;
	main.Init();
	thread ChatThread([&main](){main.Run(); });

	printf("Press any key to terminate the server\n");
	auto result = getchar();
	main.Stop();
	if (ChatThread.joinable())
	{
		ChatThread.join();
	}
	system("pause");
	return 0;
}