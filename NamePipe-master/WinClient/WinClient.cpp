// WinClient.cpp: определ€ет точку входа дл€ консольного приложени€.
//

#include "stdafx.h"


int main(int argc, char* args[])
{
	HANDLE hPipe = NULL;//дискриптор на канал
	DWORD dwMode = 0;// режим
	BOOL isSuccess = FALSE;//корректно ли все завершилось
	DWORD iCountIO = 0;// кол-во записанных или прочитанных элементов

	LPWSTR str = new WCHAR[260];// создаем строку 
	memset(str, 0, 259 * 2);// заполн€ем нул€ми она нужна дл€ буфера 

	std::wstring server;// путь к каналу 
	std::wcout << L"Print name of server, or . for localhost" << std::endl;
	std::getline(std::wcin, server);

	std::wstring pipe;
	std::wcout << L"Print name of pipe:" << std::endl;
	std::getline(std::wcin, pipe);

	std::wstring namePipe(L"\\\\");
	namePipe.append(server);
	namePipe.append(L"\\pipe\\");
	namePipe.append(pipe);
	std::wcout << L"Path to pipe: " << namePipe << std::endl;

	std::wstring message;
	std::wcout << L"Write message: " << std::endl;
	std::getline(std::wcin, message);


		hPipe = CreateFile(// создаем файл 
			namePipe.c_str(),// им€ канала 
			GENERIC_ALL,// доступ
			0, 
			NULL,
			OPEN_EXISTING,
			0,
			NULL);
		if (hPipe == INVALID_HANDLE_VALUE)// выходим если ошибка
			return EXIT_FAILURE;

		WaitNamedPipe(namePipe.c_str(), INFINITE);// дожидаем€ пока канал будет готов к работе

		dwMode = PIPE_READMODE_MESSAGE;//режим трубы чтоб работать с сообщени€ми 

		isSuccess = SetNamedPipeHandleState(//указываем значение PIPE_READMODE_MESSAGE
			hPipe,
			&dwMode,
			NULL,
			NULL);

		if(!isSuccess)
		{ 
			std::wcout << L"Can't edit mode of pipe!";
			return EXIT_FAILURE;
			
		}

		std::wcout << L"Sending message to server" << std::endl;

		isSuccess = WriteFile // отправ€лем сообщение на сервер
		(
			hPipe,
			message.c_str(),
			(message.size() + 1) * sizeof(wchar_t),
			&iCountIO,
			NULL);

		if (iCountIO != (message.size() + 1) * sizeof(wchar_t) || !isSuccess)// провер€ем было ли все корректно 
		{
			std::wcout << L"Error of push message to server!" << std::endl;
			return EXIT_FAILURE;
			
		}

		std::wcout << "Message pushed to server" << std::endl;

		do
		{
			isSuccess = ReadFile(//передаем строку котора€ €вл€етс€ буфером. ќна принимает значени€ отправленные сервером
				hPipe,
				str,// вот эта строка 
				259 * sizeof(WCHAR),
				&iCountIO,
				NULL);

			if (iCountIO > 0 && isSuccess)
				std::wcout << L"Message from server: " << str << std::endl;
			
		}
		while(!isSuccess);
	

	CloseHandle(hPipe);
	delete[] str;
	system("pause");
    return EXIT_SUCCESS;
}

