
PVOID(NTAPI* NtConvertBetweenAuxiliaryCounterAndPerformanceCounter)(PVOID, PVOID, PVOID, PVOID);

enum Code {
	Complete,
	BaseRequest,
	SizeRequest,
	PebRequest,
	QIPRequest,
	CopyRequest,
	AVMRequest,
	FVMRequest,
	PVMRequest,
	QVMRequest,
	ModuleRequest,
	IndexRequest,
};

enum Status {
	Inactive,
	Active,
	Waiting,
	Exit
};

typedef struct OperationData {

	struct {
		char* Name;
		DWORD	Id;
		PVOID	BaseAddress;
		SIZE_T  Size;
		PPEB	Peb;
		PROCESS_BASIC_INFORMATION PBI;
	} Process;

	struct {
		SIZE_T Size;
		SIZE_T ReturnLength;

		struct {
			ULONG64 Address;
			PVOID Buffer;
			BOOLEAN	ReadOperation;
		} Copy;

		struct {
			ULONG64 Address;
			ULONG64 Buffer;
			BOOLEAN	ReadOperation;
		} Copy2;

		PVOID Base;
		DWORD AllocType;
		DWORD FreeType;
		DWORD Protect;
		DWORD OldProtect;
		MEMORY_BASIC_INFORMATION MBI;
	} Memory;

	struct {
		PVOID BaseAddress;
		SIZE_T SizeOfImage;
		int Index;
	} Module;
};

typedef struct CommunicationData {

	DWORD	ProcessId;
	PVOID	SharedMemory;
	DWORD* pCode;
	SHORT* pStatus;
	DWORD	Magic;
};

namespace SharedMemory {

	CommunicationData Data{ 0 };
	INT Queue{ 0 };


	void PushQueue() {
		Queue += 1;
	}

	void PopQueue() {
		Queue -= 1;
	}

	BOOL WriteSharedMemory(PVOID Address, PVOID Value, SIZE_T Size) {
		return reinterpret_cast<BOOL>(memcpy(Address, Value, Size));
	}

	template <typename T>
	T ReadSharedMemory(PVOID Address, SIZE_T Size = sizeof(T)) {
		T Ret{ 0 };
		memcpy(static_cast<PVOID>(&Ret), Address, Size);
		return Ret;
	}

	BOOL SetStatus(Status Status) {
		return WriteSharedMemory(Data.pStatus, &Status, sizeof(SHORT));
	}

	BOOL SetCode(DWORD Code) {
		return WriteSharedMemory(Data.pCode, &Code, sizeof(DWORD));
	}

	BOOL SetBuffer(OperationData Buffer) {
		return WriteSharedMemory(Data.SharedMemory, &Buffer, sizeof(OperationData));
	}

	Status GetStatus() {
		return static_cast<Status>(ReadSharedMemory<SHORT>(Data.pStatus));
	}

	DWORD GetCode() {
		return ReadSharedMemory<DWORD>(Data.pCode);
	}

	OperationData GetBuffer() {
		return ReadSharedMemory<OperationData>(Data.SharedMemory);
	}

	BOOL SendRequest(Code Request, OperationData Data) {

		do {
			Sleep(10);
		} while (GetCode() != Complete
			|| GetStatus() != Active
			|| Queue >= 1);

		PushQueue();

		if (SetBuffer(Data)) {
			if (SetCode(Request)) {
				if (SetStatus(Waiting)) {

					do {
						Sleep(10);
					} while (GetCode() != Complete || GetStatus() != Active);

					PopQueue();
					return true;
				}
			}
		}

		PopQueue();
		return false;
	}

	void Connect(CommunicationData InitData) {
		Data = InitData;
		SetStatus(Active);
		SetCode(Complete);
	}

	void Disconnect() {
		SetStatus(Exit);
	}
};

namespace Client {
	bool ErrorFlag{ false };

	void KernelThread(PVOID LParam) {
		INT64 Status{ 0 };

		CommunicationData Data{ *(CommunicationData*)LParam };
		PVOID pData{ &Data };

		HMODULE Module{ LoadLibrary("ntdll.dll") };

		if (!Module) {
			return;
		}

		*(PVOID*)&NtConvertBetweenAuxiliaryCounterAndPerformanceCounter = GetProcAddress(Module, "NtConvertBetweenAuxiliaryCounterAndPerformanceCounter");

		if (!NtConvertBetweenAuxiliaryCounterAndPerformanceCounter) {
			ErrorFlag = true;
			return;
		}
		
		NtConvertBetweenAuxiliaryCounterAndPerformanceCounter((PVOID)1, &pData, &Status, nullptr);

	}

	void Connect() {
		CommunicationData Data{ 0 };

		PVOID Memory{ VirtualAlloc(nullptr,
					   sizeof(OperationData) * 2,
					   MEM_COMMIT | MEM_RESERVE,
					   PAGE_READWRITE) };

		if (!Memory) {
			return;
		}

		Data.ProcessId = GetCurrentProcessId();
		Data.SharedMemory = Memory;
		Data.pCode = (DWORD*)Memory + sizeof(OperationData);
		Data.pStatus = (SHORT*)Data.pCode + 8;
		Data.Magic = 0x999;

		CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)KernelThread, &Data, 0, nullptr);

		Sleep(500);

		if (ErrorFlag) {
			printf(("[!] Error Connecting\n"));
			getchar();
			exit(0);
		}

		SharedMemory::Connect(Data);
		printf(("[+] Driver connection established\n"));
	}

	void Disconnect() {
		SharedMemory::Disconnect();
	}
}

class Driver
{
public:
	DWORD PID;
	DWORD GetProcessID(LPCWSTR process_name)
	{
		HANDLE handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		DWORD process_id = NULL;

		if (handle == INVALID_HANDLE_VALUE)
			return process_id;

		PROCESSENTRY32W entry = { 0 };
		entry.dwSize = sizeof(PROCESSENTRY32W);

		if (Process32FirstW(handle, &entry)) {
			if (!_wcsicmp(process_name, entry.szExeFile))
			{
				process_id = entry.th32ProcessID;
			}
			else while (Process32NextW(handle, &entry))
			{
				if (!_wcsicmp(process_name, entry.szExeFile))
				{
					process_id = entry.th32ProcessID;
				}
			}
		}
		CloseHandle(handle);
		return process_id;
	}

	template <typename T>
	void Write(UINT_PTR dwAddress, T value)
	{
		OperationData opData = {};
		opData.Memory.Size = sizeof(T);
		opData.Memory.Copy.Address = dwAddress;
		opData.Memory.Copy.Buffer = &value;
		opData.Memory.Copy.ReadOperation = false;

		if (!SharedMemory::SendRequest(CopyRequest, opData))
		{
			printf("[!] Error writing memory\n");
		}

	}

	template <typename T>
	T Read(UINT_PTR dwAddress)
	{
		OperationData opData = {};
		opData.Memory.Size = sizeof(T);
		opData.Memory.Copy.Address = dwAddress;
		opData.Memory.Copy.Buffer = &opData.Memory.Copy.Buffer;
		opData.Memory.Copy.ReadOperation = true;

		if (SharedMemory::SendRequest(CopyRequest, opData))
		{

			return*(T*)opData.Memory.Copy.Buffer;
		}
		else
		{
			printf(("[!] Error reading memory\n"));
			return T{};
		}

	}

	ULONG64 GetBaseAddress(LPCSTR module_name)
	{
		if (module_name && PID)
		{
			OperationData Data{ 0 };
			Data.Process.Id = PID;

			if (SharedMemory::SendRequest(BaseRequest, Data)) {
				return reinterpret_cast<ULONG64>(SharedMemory::GetBuffer().Process.BaseAddress);
			}
		}
		return 0;
	}

	template <typename T>
	void ReadCamera(T& value, ULONG64 ReadAddress)
	{

		OperationData opData = {};
		opData.Memory.Size = sizeof(T);
		opData.Memory.Copy.Address = ReadAddress;
		opData.Memory.Copy.Buffer = &value;
		opData.Memory.Copy.ReadOperation = true;

		if (!SharedMemory::SendRequest(CopyRequest, opData))
		{
			printf(("[!] Error reading memory\n"));
		}

	}

	bool readEx(ULONG64 read_address, ULONG64 target_address, ULONG64 size)
	{
		OperationData Data{};
		Data.Process.Id = PID;
		Data.Memory.Copy2.Address = read_address;
		Data.Memory.Copy2.Buffer = (ULONG64)target_address;
		Data.Memory.Size = size;
		Data.Memory.Copy2.ReadOperation = true;

		SharedMemory::SendRequest(CopyRequest, Data);

		return true;
	}


	template<typename t> t
		readChain(uintptr_t address, std::vector<uint64_t> chain)
	{
		uintptr_t current = address;

		for (int i = 0; i < chain.size() - 1; i++)
		{
			current = Read<uintptr_t>(current + chain[i]);
		}

		return Read<t>(current + chain[chain.size() - 1]);
	}

	std::string ReadString(DWORD64 address)
	{
		std::string str;
		char c = '\0';
		do {
			c = Read<char>(address);
			if (c != '\0') {
				str += c;
			}
			address++;
		} while (c != '\0');

		return str;
	}

	

}rDriver;