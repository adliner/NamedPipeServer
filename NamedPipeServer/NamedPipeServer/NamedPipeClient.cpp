#include "NamedPipeClient.h"


namespace NamedPipe
{

NamedPipeClient::NamedPipeClient( const std::wstring &pipeName )
	: 
	m_pipeName( L"\\\\.\\pipe\\" + pipeName ),
	m_hPipeHandle(INVALID_HANDLE_VALUE)
{
}


NamedPipeClient::~NamedPipeClient()
{
}

void NamedPipeClient::Connect()
{
	bool result = false;

	result = ( m_hPipeHandle = ::CreateFile(
		m_pipeName.c_str(),		// pipe name 
		GENERIC_READ |			// read and write access 
		GENERIC_WRITE,
		0,						// no sharing 
		NULL,					// default security attributes
		OPEN_EXISTING,			// opens existing pipe 
		0,						// default attributes 
		NULL					// no template file 
	) ) != INVALID_HANDLE_VALUE;

	// The pipe connected; change to message-read mode. 

	DWORD dwMode = PIPE_READMODE_MESSAGE;

	if ( result )
	{
		result = SetNamedPipeHandleState(
			m_hPipeHandle,		// pipe handle 
			&dwMode,			// new pipe mode 
			NULL,				// don't set maximum bytes 
			NULL				// don't set maximum time 
		) == TRUE;
	}

	int xxx = 22;
}

std::future<bool> NamedPipeClient::writeBinaryMessage( const std::vector<uint8_t> &message )
{
	return std::async(
		std::launch::async,
		&NamedPipeClient::WriteAsyncHandler,
		this,
		message
	);
}

std::future<bool> NamedPipeClient::writeStringMessage( const std::string & message )
{
	return std::async(
		std::launch::async,
		&NamedPipeClient::WriteAsyncHandler,
		this,
		std::vector<uint8_t>( message.begin(), message.end() )
	);
}

bool NamedPipeClient::WriteAsyncHandler( const std::vector<uint8_t>& message )
{
	bool result = false;
	DWORD numBytesWritten = 0;
	DWORD lastError = 0;

	OVERLAPPED overlapped = { 0 };

	result = ( overlapped.hEvent = ::CreateEvent(
		NULL,
		TRUE,
		FALSE,
		NULL ) ) != INVALID_HANDLE_VALUE;

	if ( result )
	{
		result = WriteFile(
			m_hPipeHandle,
			(LPVOID)( message.data() ),
			message.size(),
			&numBytesWritten,
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
				result = false;
			default:
				assert( !"Should not happen" );
				result = false;
			}
		}
	}
	else
		if ( result && numBytesWritten != 0 )
		{
			result = true;
		}
	return result;
}

}