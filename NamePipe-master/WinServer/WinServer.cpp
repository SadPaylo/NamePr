// WinServer.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"

#define BASE_PATH_PIPE L"\\\\.\\pipe\\" // базовый путь pipe 
#define SIZE_BUFFER 2048 

#define MESSAGE L"Message" //возвращаемое сообщение
#define SIZE_MSG_BYTES 16 // размер сообщения 

DWORD WINAPI InstanceThread(_In_ LPVOID hPipe); //обьявление функции для работы с экземпляром потока который будет принимать дискриптор(hPipe) на канал

int main(int argc, char* params[])
{
	BOOL isConnected = FALSE;// было ли соеденение 
	DWORD dwThreadId = 0; // для того чтобы давать каждому потоку ID 
	HANDLE hPipe = NULL; // дискриптор на канал
	std::wstring namePipe(BASE_PATH_PIPE);//путь к каналу
	std::wstring tmp;// название канала
	std::list<HANDLE> threads;//перечень потоков 

	std::cout << "Write name of pipe: " << std::endl;
	std::wcin >> tmp;                                      // вводим название канала
	namePipe.append(tmp);

	while (TRUE)
	{
		std::wcout << L"Main thread awake creating pipe with path: " 
			<< namePipe << std::endl;
		std::wcout << L"Wait for client" << std::endl;
		hPipe = CreateNamedPipeW( //создание канала
			namePipe.c_str(),// назавание
			PIPE_ACCESS_DUPLEX,//режим канала - чтение запись
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,// особенности канала он может работать с сообщениями читать сообщения и быть в режиме ожидания
			PIPE_UNLIMITED_INSTANCES,//кол-во экземпляров соеденений можно создать
			SIZE_BUFFER,// размер буферов чтения и записи
			SIZE_BUFFER,
			INFINITE,// ожидание присоеденения клиента
			NULL); // указатель на структуру безопасности

		if (hPipe == INVALID_HANDLE_VALUE)// ошибка создания канала
		{
			std::wcout << L"Error of creating pipe! Process will be terminated!" << std::endl;
			return EXIT_FAILURE;
		}

		isConnected = ConnectNamedPipe(hPipe, NULL);// соеденение к каналу
		if (isConnected)
		{
			std::wcout << L"Client connected. Creating thread for user" << std::endl;
			HANDLE hThread = CreateThread(//создаем поток для работы с пользователем
				NULL,
				NULL,
				InstanceThread,
				(LPVOID) hPipe,
				NULL,
				&dwThreadId);//инкрементируем id потока
			if (hThread == INVALID_HANDLE_VALUE)
				std::wcout << L"Error of making thread!" << std::endl;
			else
			{
				++dwThreadId;// добовляем поток в перечень
				threads.push_back(hThread);// закрыть в окончании сессии 
			}
		}
		else//не смогли присоедениться
		{
			std::wcout << L"Error of connection user" << std::endl;
			CloseHandle(hPipe);
		}
	}

	std::for_each(threads.cbegin(), threads.cend(),// проходимся от начала до конца перечня 
		[](HANDLE h)
	{
		CloseHandle(h);// закрыть дескриптор потока
	});
	CloseHandle(hPipe);// закрыть дескриптор канала

    return EXIT_SUCCESS;
}

DWORD WINAPI InstanceThread(_In_ LPVOID hPipe)
{
	LPWSTR strRequest = new WCHAR[SIZE_BUFFER + 1];// буфер куда будем записывать или читать данные 
	memset(strRequest, 0, (SIZE_BUFFER + 1) * sizeof(WCHAR));
	DWORD cntBytesRead = 0;// кол- байт прочитали 
	DWORD cntBytesWrited = 0;// кол-во байт записали
	BOOL isSucsses = FALSE;

	HANDLE pipe = (HANDLE)hPipe;
	
	std::wcout << L"Instance created, and wait for messages" << std::endl;

	while (TRUE)
	{
		isSucsses = ReadFile(// в переменную записывается результат выполнения функции ReadFile
			pipe,// дискриптор трубы
			strRequest,// куда читать
			SIZE_BUFFER * sizeof(WCHAR),//размер буфера
			&cntBytesRead,// сколько прочитали
			NULL);

		if (!isSucsses)
		{
			std::wcout << "Error of read user message or user disconnect" << std::endl;
			break;
		}
		else
			std::wcout << strRequest << std::endl;//выводим то что написал пользователь

		isSucsses = WriteFile(// записать сообщение
			pipe,
			MESSAGE,
			SIZE_MSG_BYTES,
			&cntBytesWrited,
			NULL
		);

		if (!isSucsses || cntBytesWrited != SIZE_MSG_BYTES)// если произошла ошибка или кол-во байт не равно тому размеру который обьявлен в константе
		{
			std::wcout << L"Error of reply!" << std::endl;
			break;
		}
		else
			std::wcout << L"Message will be replied" << std::endl;// если все нормально то выводим это 

	}

	if(strRequest)// проверка является ли strRequest равной 0
		delete[] strRequest;// если не равна можем очистить массив 
	CloseHandle(pipe);
	ExitThread(0);// выйити с потока
}