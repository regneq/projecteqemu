#ifndef EQEMU_ENCRYPTION_H
#define EQEMU_ENCRYPTION_H

#ifdef WIN32
#include <windows.h>
#include <string>

using namespace std;

typedef char*(*DLLFUNC_DecryptUsernamePassword)(const char*, unsigned int, int);
typedef char*(*DLLFUNC_Encrypt)(const char*, unsigned int, unsigned int&);
typedef void(*DLLFUNC_HeapDelete)(char*);

/**
 * Basic windows encryption plugin.
 * Handles the managment of the plugin.
 */
class Encryption
{
public:
	/**
	 * Constructor, sets all member pointers to NULL.
	 */
	Encryption() : h_dll(NULL), encrypt_func(NULL), decrypt_func(NULL), delete_func(NULL) { };

	/**
	 * Destructor, if it's loaded it unloads this library.
	 */
	~Encryption() { if(Loaded()) { Unload(); } }

	/**
	 * Returns true if the dll is loaded, otherwise false.
	 */
	inline bool	Loaded() { return (h_dll != NULL); }

	/**
	 * Loads the plugin.
	 * True if there are no errors, false if there was an error.
	 */
	bool LoadCrypto(string name);

	/**
	 * Wrapper around the plugin's decrypt function.
	 */
	char* DecryptUsernamePassword(const char* encryptedBuffer, unsigned int bufferSize, int mode);

	/**
	 * Wrapper around the plugin's encrypt function.
	 */
	char* Encrypt(const char* buffer, unsigned int bufferSize, unsigned int &outSize);

	/**
	 * Wrapper around the plugin's delete function.
	 */
	void DeleteHeap(char* buffer);

private:
	/**
	 * Loads the named dll into memory.
	 * Returns true if there were no errors, otherwise return false.
	 */
	bool Load(const char *name);

	/**
	 * Frees the dll from memory if it's loaded.
	 */
	void Unload();

	/**
	 * Similar in function to *sym = GetProcAddress(h_dll, name).
	 * Returns true if there were no errors, false otherwise.
	 */
	bool GetSym(const char *name, void **sym);

	/**
	 * Similar in function to return GetProcAddress(h_dll, name).
	 * Returns a pointer to the function if it is found, null on an error.
	 */
	void *GetSym(const char *name);

	HINSTANCE h_dll;
	DLLFUNC_Encrypt encrypt_func;
	DLLFUNC_DecryptUsernamePassword decrypt_func;
	DLLFUNC_HeapDelete delete_func;
};

#endif
#endif

