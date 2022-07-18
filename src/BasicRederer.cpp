#include "BasicRenderer.h"

BasicRenderer* GlobalRenderer;

BasicRenderer::BasicRenderer(Framebuffer* targetFramebuffer, PSF1_FONT* psf1_Font)
{
    TargetFramebuffer = targetFramebuffer;
    PSF1_Font = psf1_Font;
    Colour = 0xffffffff;
    CursorPosition = {0, 0};
}

void BasicRenderer::Clear(uint32_t colour){
    uint64_t fbBase = (uint64_t)TargetFramebuffer->BaseAddress;
    uint64_t bytePerScanline = TargetFramebuffer->PixelsPerScanLine * 4;
    uint64_t fbHeight = TargetFramebuffer->Height;
    uint64_t fbSize = TargetFramebuffer->BufferSize;

    for (int verticalScanLine = 0; verticalScanLine < fbHeight; verticalScanLine ++){
        uint64_t pixPtrBase = fbBase + (bytePerScanline * verticalScanLine);
        for (uint32_t* pixPtr = (uint32_t*)pixPtrBase; pixPtr < (uint32_t*)(pixPtrBase + bytePerScanline); pixPtr ++)
            *pixPtr = colour;
    }
}

void BasicRenderer::Next(){
    CursorPosition.X = 0;
    CursorPosition.Y += 16;
}

void BasicRenderer::Print(const char* str)
{
    
    char* chr = (char*)str;
    while(*chr != 0){
        PutChar(*chr, CursorPosition.X, CursorPosition.Y);
        CursorPosition.X+=8;
        if(CursorPosition.X + 8 > TargetFramebuffer->Width)
        {
            CursorPosition.X = 0;
            CursorPosition.Y += 16;
        }
        chr++;
    }
}

void BasicRenderer::PutChar(char chr, unsigned int xOff, unsigned int yOff)
{
    unsigned int* pixPtr = (unsigned int*)TargetFramebuffer->BaseAddress;
    char* fontPtr = (char*)PSF1_Font->glyphBuffer + (chr * PSF1_Font->psf1_Header->charsize);
    for (unsigned long y = yOff; y < yOff + 16; y++){
        for (unsigned long x = xOff; x < xOff+8; x++){
            if ((*fontPtr & (0b10000000 >> (x - xOff))) > 0){
                    *(unsigned int*)(pixPtr + x + (y * TargetFramebuffer->PixelsPerScanLine)) = Colour;
                }

        }
        fontPtr++;
    }
}

extern "C" NTSTATUS DriverEntry(
	_In_ PDRIVER_OBJECT DriverObject, 
	_In_ PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);

	//
	// Figure out when we built this last for debugging purposes.
	//
	kprintf("[+] infinityhook: Loaded.\n");
	
	//
	// Let the driver be unloaded gracefully. This also turns off 
	// infinity hook.
	//
	DriverObject->DriverUnload = DriverUnload;

	//
	// Demo detouring of nt!NtCreateFile.
	//
	OriginalNtCreateFile = (NtCreateFile_t)MmGetSystemRoutineAddress(&StringNtCreateFile);
	if (!OriginalNtCreateFile)
	{
		kprintf("[-] infinityhook: Failed to locate export: %wZ.\n", StringNtCreateFile);
		return STATUS_ENTRYPOINT_NOT_FOUND;
	}

	//
	// Initialize infinity hook. Each system call will be redirected
	// to our syscall stub.
	//
	NTSTATUS Status = IfhInitialize(SyscallStub);
	if (!NT_SUCCESS(Status))
	{
		kprintf("[-] infinityhook: Failed to initialize with status: 0x%lx.\n", Status);
	}

	return Status;
}

