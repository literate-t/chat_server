#include "stdafx.h"

int main()
{
	library::Queue<int> queue(100);
	for (int i = 0; i < 100; ++i)
	{
		queue.Push(i);
	}
	while (!queue.IsEmpty())
	{		
		printf("%d\n", queue.Front());
		queue.Pop();
	}
	queue.Pop();
	system("pause");
	return 0;
}