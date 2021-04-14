#include <iostream>
#include <string>
#include <cstdint>
#include <cassert>
#include <winscard.h>
#include <vector>
#include <time.h>
#pragma comment(lib, "winscard.lib")
using std::cout;                using std::cin;
using std::endl;                using std::string;
using std::vector;              using std::stringstream;

//Global Variables
SCARDCONTEXT    hSC;
LONG            lReturn, lReturn2;
LPTSTR          pmszReaders = NULL;
LPTSTR          pReader;
DWORD           cch = SCARD_AUTOALLOCATE;
SCARDHANDLE     cHandle;
DWORD           dwAP;
DWORD           dwControlCode;
SCARDHANDLE     hcard;
char loopRetry = 'y';
char retry = 'y';

std::uint8_t char_to_nibble(char c)
{
	assert((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'));
	if ((c >= '0') && (c <= '9'))
		return c - '0';
	else
		return c - 'A' + 0xA;
}

void str2ba(const std::string& src, std::vector<std::uint8_t>& dst)
{
	assert(src.size() % 2 == 0);
	dst.reserve(src.size() / 2);
	dst.clear();
	auto it = src.begin();
	while (it != src.end()) {
		std::uint8_t hi = char_to_nibble(*it++);
		std::uint8_t lo = char_to_nibble(*it++);
		dst.push_back(hi * 16 + lo);
	}
}
void delay(int milliseconds) // Cross-platform sleep function
{
	clock_t time_end;
	time_end = clock() + milliseconds * CLOCKS_PER_SEC / 1000;
	while (clock() < time_end)
	{
	}
}
void header() {

	printf(" _   _ _____ ____ __        __    _ _              ");
	printf("\n");
	printf("| \\ | |  ___/ ___|\\ \\      / / __(_) |_ ___ _ __ ");
	printf("\n");
	printf("|  \\| | |_ | |     \\ \\ /\\ / / '__| | __/ _ \\ '__|");
	printf("\n");
	printf("| |\\  |  _|| |___   \\ V  V / | | | | ||  __/ |   ");
	printf("\n");
	printf("|_| \\_|_|   \\____|   \\_/\\_/  |_| |_|\\__\\___|_|   ");
	printf("                                                        \n");
	printf("\n");

}
void connect() {
	while (loopRetry == 'y') {
		printf("Connection: ");
		lReturn = SCardConnect(hSC, pmszReaders, SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &cHandle, &dwAP);

		if (SCARD_S_SUCCESS != lReturn)
		{
			printf("\x1B[31mFailed!\033[0m\t\t\n");
			printf("retry?: (Y/N)");
			cin >> loopRetry;
		}

		// Use the connection.
		// Display the active protocol.
		switch (dwAP)
		{
		case SCARD_PROTOCOL_T0:
			printf("\x1B[32mActive Protocol T0\033[0m\t\t\n");
			loopRetry = 'n';
			break;

		case SCARD_PROTOCOL_T1:
			printf("\x1B[32mActive Protocol T1\033[0m\t\t\n");
			loopRetry = 'n';
			break;

		case SCARD_PROTOCOL_UNDEFINED:
		default:
			printf("\x1B[33mActive protocol unnegotiated or unknown\033[0m\t\t\n");
			printf("retry?: (Y/N)");
			cin >> loopRetry;
			break;
		}
	}
}
void status() {
	WCHAR           szReader[200];
	BYTE            bAttr[32];
	DWORD           cByte = 32;
	DWORD           dwState, dwProtocol;

	lReturn = SCardStatus(cHandle, szReader, &cch, &dwState, &dwProtocol, (LPBYTE)&bAttr, &cByte);
	printf("Card Status: ");

	if (SCARD_S_SUCCESS != lReturn)
	{
		printf("\x1B[31mFailed!\033[0m\t\t\n");
		//exit(1);     
	}

	// Examine retrieved status elements.
	// Look at the reader name and card state.
	//printf("%S\n", szReader);
	switch (dwState)
	{
	case SCARD_ABSENT:
		printf("\x1B[31mCard absent\033[0m\t\t\n");
		break;
	case SCARD_PRESENT:
		printf("\x1B[33mCard present\033[0m\t\t\n");
		break;

	case SCARD_SWALLOWED:
		printf("\x1B[33mCard swallowed\033[0m\t\t\n");
		break;

	case SCARD_POWERED:
		printf("\x1B[33mCard has power\033[0m\t\t\n");
		break;

	case SCARD_NEGOTIABLE:
		printf("\x1B[33mCard reset and waiting PTS negotiation\033[0m\t\t\n");
		break;

	case SCARD_SPECIFIC:
		printf("\x1B[32mCard has specific communication protocols set\033[0m\t\t\n");
		break;

	default:
		printf("\x1B[33mUnknown or unexpected card state\033[0m\t\t\n");
		break;
	}
}
void listReaders() {
	lReturn = SCardListReaders(hSC, NULL, (LPTSTR)&pmszReaders, &cch);
	printf("READER: ");
	switch (lReturn)
	{
	case SCARD_E_NO_READERS_AVAILABLE:
		printf("\x1B[31mNot Found!\033[0m\t\t\n");
		printf("retry?(y/n): ");
		cin >> retry;
		if (retry == 'y') {
			listReaders();
		}
		break;

	case SCARD_S_SUCCESS:

		pReader = pmszReaders;
		while ('\0' != *pReader)
		{
			// Display the value.

			printf(" %S\n", pReader);
			// Advance to the next value.
			pReader = pReader + wcslen((wchar_t*)pReader) + 1;
		}
		// Free the memory.
		lReturn2 = SCardFreeMemory(hSC,
			pmszReaders);
		if (SCARD_S_SUCCESS != lReturn2)
			printf("\x1B[31mFailed! error: Free Memory\033[0m\t\t\n");
		break;

	default:
		printf("\x1B[31mFailed!\033[0m\t\t\n");

		break;
	}
}
void context() {
	lReturn = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hSC);
	printf("CONTEXT: ");
	if (SCARD_S_SUCCESS != lReturn)
		printf("\x1B[31mFailed!\033[0m\t\t\n");
	else
	{

		printf("\x1B[32mEstablished\033[0m\t\t\n");
	}
}
void shutDown() {

	lReturn = SCardDisconnect(cHandle, SCARD_LEAVE_CARD);
	if (SCARD_S_SUCCESS != lReturn)
	{
		//printf('%lu', lReturn);
		printf("Failed SCardDisconnect\n");
		exit(1);  
	}
	lReturn = SCardReleaseContext(hSC);
};
int main()
{

	header();

	///////////////////////////
	//   Establish Context 
	///////////////////////////
	context();

	///////////////////////////
	//    List Readers
	///////////////////////////
	listReaders();
	// Retrieve the list the readers.
	// hSC was set by a previous call to SCardEstablishContext.

	///////////////////////////
	//    Connect!
	/////////////////////////// 
	connect();

	///////////////////////////
	//   Transmit!
	///////////////////////////
	string cmd;
	int i;
	LPSCARD_IO_REQUEST  pioRecvPci;
	BYTE                pbSend[258];//get status
	BYTE                pbRecv[258];
	DWORD               dwRecv;
	//vector<std::uint8_t> itemsTest;
	BYTE cmd1[] = {0xFF, 0x00, 0x00, 0x00, 0x02, 0xD4, 0x04};//Interface Status, Used this to find the bad tag (err27)
	BYTE cmd2[] = { 0xFF, 0xCA, 0x00, 0x00, 0x01};// ISO1443-4 Type a ATS
	BYTE cmd3[] = { 0xFF, 0x00, 0x40, 0xD0, 0x04, 0x05, 0x05, 0x03, 0x01 };//reader party mode
	BYTE cmd4[] = { 0xFF, 0xB0, 0x00, 0x04, 0x01};//Read 16 bytes from binary block 04h

	//////////////////////////////////User input,  Not functional :(       //////////////////////////
	//loopRetry = 'y';
	//while (loopRetry == 'y') {
	//	printf("Enter Command byte for byte. Enter 'end' when finished:\n ");
	//	while (cmd != "end") {

	//		printf("BYTE: ");
	//		std::cin >> cmd;
	//		
	//		if (cmd != "end") {
	//			
	//			str2ba(cmd, itemsTest);
	//			
	//		
	//		}
	//		else {
	//			printf("%lu", sizeof(itemsTest));
	//			loopRetry = 'n';
	//		}
	//	
	//	}

	//}
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	loopRetry = 'y';
	while (loopRetry == 'y') {
		lReturn = SCardTransmit(cHandle, SCARD_PCI_T1, cmd2, sizeof(cmd2), NULL, pbRecv, &dwRecv);//ATS
		lReturn = SCardTransmit(cHandle, SCARD_PCI_T1, cmd4, sizeof(cmd4), NULL, pbRecv, &dwRecv);// Read Command
		if (SCARD_S_SUCCESS != lReturn)
		{

			printf("\x1B[31mFailed SCardTransmit\033[0m\t\t\n");
			printf("retry?: (Y/N)");
			cin >> retry;
			if (retry == 'y') {
				connect();
			}
			else {
				loopRetry = 'n';
			}

		}
		else {
			printf("Success! \n");
			printf("Receive Buffer size: ");
			printf("%lu", dwRecv);
			printf("\n");
			printf("Received: ");
			for (i = 0; i < dwRecv; i++)
				printf("%02X ", pbRecv[i]);
			printf("\n");
		}
		printf("retransmit?: (Y/N)");
		cin >> loopRetry;
	}
///////////////////////////
//   Release resources
///////////////////////////
	printf("\x1B[31mReleasing Resources\033[0m\t\t\n");
	delay(100);
	shutDown();



}


