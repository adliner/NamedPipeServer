#pragma once
#include <string>
#include <future>
#include <exception>
#include <assert.h>
#include <windows.h>

namespace NamedPipe
{

class NamedPipeClient
{
public:
	NamedPipeClient(const std::wstring &pipeName);
	~NamedPipeClient();

	void Connect();
	std::future<bool> writeBinaryMessage( const std::vector<uint8_t> &message );
	std::future<bool> writeStringMessage( const std::string &message );
private:
	bool WriteAsyncHandler( const std::vector<uint8_t> &message );

	std::wstring	m_pipeName;
	HANDLE			m_hPipeHandle;
};

class NamedPipeClientException : public std::exception
{
public:
	NamedPipeClientException( const std::string &message = "" )
		: std::exception( message.c_str() ) {}
};

class NamedPipeClientExceptionFailedToConnect : public NamedPipeClientException
{
public:
	NamedPipeClientExceptionFailedToConnect( const std::string &message = "" )
		: NamedPipeClientException( message ) {}
};

}
