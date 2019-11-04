#pragma once
#include <string>
#include <vector>
#include <thread>
#include <future>
#include <stdint.h>
#include <assert.h>
#include <exception>
#include <Windows.h>

// based on example https://msdn.microsoft.com/en-us/library/windows/desktop/aa365603(v=vs.85).aspx
namespace NamedPipe
{

const uint32_t DEFAULT_BUFFER_SIZE = 4 * 1024;

class NamedPipeServer
{
public:
	NamedPipeServer( std::wstring pipeName );
	virtual ~NamedPipeServer();
	std::future<bool> WaitForClientToConnect();
	std::future<std::vector<uint8_t>> getNextBinaryMessage();
	std::future<std::string> getNextStringMessage();

private:
	bool Initialize();
	bool CreateNamedPipe();
	bool WaitForClientToConnectAsyncHandler();
	std::vector<uint8_t> ReadAsyncHandler();
	std::string ReadAsyncHandlerStr();

	std::wstring		m_pipeName;

	uint32_t			m_inboundBufferSize;
	uint32_t			m_outboundBufferSize;

	HANDLE				m_hPipeHandle;
};

class NamedPipeServerException : public std::exception
{
public:
	NamedPipeServerException(const std::string &message = "")
		: std::exception( message.c_str() ) {}
};

class NamedPipeServerExceptionFailedToCreate : public NamedPipeServerException
{
public:
	NamedPipeServerExceptionFailedToCreate( const std::string &message = "" )
		: NamedPipeServerException( message ) {}
};


}