#include "NamedPipeServer.h"

namespace NamedPipe
{

NamedPipeServer::NamedPipeServer( std::wstring pipeName )
	: 
	m_hPipeHandle( INVALID_HANDLE_VALUE ),
	m_pipeName( L"\\\\.\\pipe\\" + pipeName ), 
	m_inboundBufferSize( DEFAULT_BUFFER_SIZE ), 
	m_outboundBufferSize( DEFAULT_BUFFER_SIZE )
{
	if ( !Initialize() )
		throw NamedPipeServerExceptionFailedToCreate();
}

NamedPipeServer::~NamedPipeServer()
{
}

bool NamedPipeServer::Initialize()
{
	bool result = false;

	result = CreateNamedPipe();

	return result;
}

bool NamedPipeServer::CreateNamedPipe()
{
	bool result = false;
	HANDLE hEvent = INVALID_HANDLE_VALUE;

	result = ( m_hPipeHandle = ::CreateNamedPipeW(
		m_pipeName.c_str(),								// pipe name 
		PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,      // read/write access 
		PIPE_TYPE_MESSAGE |								// message type pipe 
		PIPE_READMODE_MESSAGE |							// message-read mode 
		PIPE_WAIT,										// blocking mode 
		PIPE_UNLIMITED_INSTANCES,						// max. instances  
		m_outboundBufferSize,							// output buffer size 
		m_inboundBufferSize,							// input buffer size 
		0,												// client time-out,  0 falls back to 50ms 
		NULL											// default security attribute 
	)) != INVALID_HANDLE_VALUE;

	return result;
}

bool NamedPipeServer::WaitForClientToConnectAsyncHandler()
{
	bool result = false;
	DWORD lastError = 0;

	OVERLAPPED overlapped = { 0 };

	result = ( overlapped.hEvent = ::CreateEvent(
		NULL,
		TRUE,
		FALSE,
		NULL ) ) != INVALID_HANDLE_VALUE;

	if ( result )
	{
		result = ::ConnectNamedPipe(
			m_hPipeHandle,
			&overlapped
		) == NULL;

		lastError = GetLastError();

		assert(
			// if client not yet connected priot to ConnectNamedPipe()
			lastError == ERROR_IO_PENDING && result
			||
			// if client has already connected prior to ConnectNamedPipe()
			lastError == ERROR_PIPE_CONNECTED && result 
		);
	}

	if ( result && lastError == ERROR_IO_PENDING )
	{
		DWORD waitResult = WaitForSingleObject(
			overlapped.hEvent,
			INFINITE
		);

		switch ( waitResult )
		{
		case WAIT_OBJECT_0:
			result = true;
			break;
		case WAIT_ABANDONED:
		case WAIT_TIMEOUT:
		case WAIT_FAILED:
			result = true;
		default:
			assert( !"Should not happen" );
			result = false;
		}
	}
	else
	{
		result = ( result && lastError == ERROR_PIPE_CONNECTED );
	}

	return result;
}

std::vector<uint8_t> NamedPipeServer::ReadAsyncHandler()
{
	bool					result = false;
	std::vector<uint8_t>	readData( m_inboundBufferSize );
	DWORD					numBytesRead = 0;
	DWORD					lastError = 0;

	OVERLAPPED overlapped = { 0 };

	result = ( overlapped.hEvent = ::CreateEvent(
		NULL,
		TRUE,
		FALSE,
		NULL ) ) != INVALID_HANDLE_VALUE;

	while ( 1 )
	{
		if ( result )
		{
			result = ::ReadFile(
				m_hPipeHandle,
				reinterpret_cast<LPVOID>( readData.data() ),
				static_cast<DWORD>( readData.size() ),	
				&numBytesRead,
				&overlapped
			) == TRUE;

			lastError = GetLastError();
		}

		if ( !result && lastError == ERROR_IO_PENDING )
		{
			if ( result )
			{
				DWORD waitResult = WaitForSingleObject(
					overlapped.hEvent,
					INFINITE
				);

				switch ( waitResult )
				{
				case WAIT_OBJECT_0:
					result = true;
					break;
				case WAIT_ABANDONED:
				case WAIT_TIMEOUT:
				case WAIT_FAILED:
					result = true;
				default:
					assert( !"Should not happen" );
					result = false;
				}
			}
		}
		else
			if ( result && numBytesRead != 0 )
			{
				readData.resize( numBytesRead );
				break;
			}
	}

	return readData;
}

std::string NamedPipeServer::ReadAsyncHandlerStr()
{
	std::vector<uint8_t> message = ReadAsyncHandler();

	return std::string( message.begin(), message.end() );
}

std::future<bool> NamedPipeServer::WaitForClientToConnect()
{
	return std::async(
		std::launch::async,
		&NamedPipeServer::WaitForClientToConnectAsyncHandler,
		this
	);
}

std::future<std::vector<uint8_t>> NamedPipeServer::getNextBinaryMessage()
{
	return std::async(
		std::launch::async,
		&NamedPipeServer::ReadAsyncHandler,
		this
	);
}

std::future<std::string> NamedPipeServer::getNextStringMessage()
{
	return std::async(
		std::launch::async,
		&NamedPipeServer::ReadAsyncHandlerStr,
		this
	);
}

}