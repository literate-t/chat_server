#include "stdafx.h"

int main()
{
	char packet[1024];
	int num1 = 0;
	short num2 = 0;
	char ch = 0;
	char buf[512];

	VBuffer::GetInstance()->Init();
	VBuffer::GetInstance()->SetShort(1234);
	VBuffer::GetInstance()->SetInteger(1234567);
	VBuffer::GetInstance()->SetChar('H');
	VBuffer::GetInstance()->SetString("HIHI I AM SO HI");

	VBuffer::GetInstance()->CopyBuffer(packet);
	VBuffer::GetInstance()->SetBuffer(packet);
	
	VBuffer::GetInstance()->GetShort(num2);
	VBuffer::GetInstance()->GetInteger(num1);
	VBuffer::GetInstance()->GetChar(ch);
	VBuffer::GetInstance()->GetString(buf);

	printf("%d\n", num2);
	printf("%d\n", num1);
	printf("%c\n", ch);
	printf("%s\n", buf);

	return 0;
}