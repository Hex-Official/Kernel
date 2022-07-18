#include "Bitmap.h"

bool Bitmap::operator[](uint64_t index){
    uint64_t byteIndex = index / 8;
    uint8_t bitIndex = index % 8;
    uint8_t bitIndexer = 0b10000000 >> bitIndex;
    if ((Buffer[byteIndex] & bitIndexer) > 0){
        return true;
    }
    return false;
}

void Bitmap::Set(uint64_t index, bool value){
    uint64_t byteIndex = index / 8;
    uint8_t bitIndex = index % 8;
    uint8_t bitIndexer = 0b10000000 >> bitIndex;
    Buffer[byteIndex] &= ~bitIndexer;
    if (value){
        Buffer[byteIndex] |= bitIndexer;
    }
}

void __fastcall SyscallStub(
	_In_ unsigned int SystemCallIndex, 
	_Inout_ void** SystemCallFunction)
{
	// 
	// Enabling this message gives you VERY verbose logging... and slows
	// down the system. Use it only for debugging.
	//
	
#if 0
	kprintf("[+] infinityhook: SYSCALL %lu: 0x%p [stack: 0x%p].\n", SystemCallIndex, *SystemCallFunction, SystemCallFunction);
#endif

	UNREFERENCED_PARAMETER(SystemCallIndex);

	//
	// In our demo, we care only about nt!NtCreateFile calls.
	//
	if (*SystemCallFunction == OriginalNtCreateFile)
	{
		//
		// We can overwrite the return address on the stack to our detoured
		// NtCreateFile.
		//
		*SystemCallFunction = DetourNtCreateFile;
	}
}
