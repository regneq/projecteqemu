#include "../common/debug.h"
#include "Encryption.h"
#include "ErrorLog.h"

extern ErrorLog *log;

bool Encryption::LoadCrypto(string name)
{
	if(!Load(name.c_str()))
	{
		log->Log(log_error, "Failed to load %s from the operating system.", name.c_str());
		return false;
	}
	else
	{
		encrypt_func = (DLLFUNC_Encrypt)GetSym("Encrypt");
		if(encrypt_func == NULL)
		{
			log->Log(log_error, "Failed to attach Encrypt.");
			Unload();
			return false;
		}
		decrypt_func = (DLLFUNC_DecryptUsernamePassword)GetSym("DecryptUsernamePassword");
		if(decrypt_func == NULL)
		{
			log->Log(log_error, "Failed to attach DecryptUsernamePassword.");
			Unload();
			return false;
		}
		delete_func = (DLLFUNC_HeapDelete)GetSym("_HeapDeleteCharBuffer");
		if(delete_func == NULL)
		{
			log->Log(log_error, "Failed to attach _HeapDeleteCharBuffer.");
			Unload();
			return false;
		}
	}
	return true;
}

char *Encryption::DecryptUsernamePassword(const char* encrypted_buffer, unsigned int buffer_size, int mode)
{
	if(decrypt_func)
	{
		return decrypt_func(encrypted_buffer, buffer_size, mode);
	}
	return NULL;
}

char* Encryption::Encrypt(const char* buffer, unsigned int buffer_size, unsigned int &out_size)
{
	if(encrypt_func)
	{
		return encrypt_func(buffer, buffer_size, out_size);
	}
	return NULL;
}

void Encryption::DeleteHeap(char* buffer)
{
	if(delete_func)
	{
		delete_func(buffer);
	}
}

bool Encryption::Load(const char *name)
{
	SetLastError(0);
#ifdef UNICODE
	int name_length = strlen(name);
	int wide_length = MultiByteToWideChar(CP_ACP, 0, name, name_length+1, 0, 0);
	WCHAR *wide_string = new WCHAR[wide_length];
	MultiByteToWideChar(CP_ACP, 0, name, name_length+1, wide_string, wide_length);

	h_dll = LoadLibrary(wide_string);
	delete[] wide_string;
#else
	h_dll = LoadLibrary(name);
#endif

	if(h_dll == NULL) 
	{
		return false;
	}
    else 
	{ 
		SetLastError(0); 
	}
	
	return true;
}

void Encryption::Unload()
{
	if(h_dll)
	{
		FreeLibrary(h_dll);
		h_dll = NULL;
	}
}

bool Encryption::GetSym(const char *name, void **sym)
{
	if(Loaded())
	{
		*sym = GetProcAddress(h_dll, name);
		return(*sym != NULL);
	}
	else
	{
		return false;
	}
}

void *Encryption::GetSym(const char *name)
{
	if(Loaded())
	{
		return GetProcAddress(h_dll, name);
	}
	else
	{
		return NULL;
	}
}

