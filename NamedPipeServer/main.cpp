// NamedPipeServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <future>
#include "NamedPipeServer\NamedPipeServer.h"
#include "NamedPipeServer\NamedPipeClient.h"


int main(int argc, char* argv[])
{
	NamedPipe::NamedPipeServer pipeServer(L"test");
	NamedPipe::NamedPipeClient pipeClient( L"test" );

	std::future<bool> clientConnected = pipeServer.WaitForClientToConnect();

	//std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
	std::async(
		std::launch::async,
		[&]()
	{
		std::this_thread::sleep_for( std::chrono::milliseconds( 2000 ) );
		pipeClient.Connect();
	} );

	NamedPipe::NamedPipeClient pipeClient2( L"test" );
	std::future<bool> clientConnected2 = pipeServer.WaitForClientToConnect();
	pipeClient2.Connect();
	bool connectResult2 = clientConnected2.get();
	bool writeResult = pipeClient.writeStringMessage( "xxxx" ).get();
	writeResult = pipeClient.writeBinaryMessage( {0xa, 0xa, 0xa, 0x0 } ).get();
	std::string nextMessage = pipeServer.getNextStringMessage().get();
	auto nextBinMessage = pipeServer.getNextBinaryMessage().get();


	for ( ;; ) {}
	return 0;
}

