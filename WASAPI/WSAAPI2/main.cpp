/*
WASAPI:实现录音和播放功能，主要区别在于
GetDefaultAudioEndpoint：获取指定设备时，capture为录制，render为播放.
播放：通过IAudioRenderClient的getbuffer获取空闲的buffer的起始地址，
然后填充数据，最后releasebuffer释放。
采集：通过IAudioCaptureClient的getbiffer获取有效的sample个数和起始地址.
取出数据，最后releasebuffer释放。

*/
#define _USE_MATH_DEFINES

#include <tchar.h>
#include <iostream>
#include <math.h>
#include <string>

#include <Windows.h>
#include <audiopolicy.h>
#include <Audioclient.h>
#include <mmdeviceapi.h>

using namespace std;

#define REFTIMES_PER_SEC 10000000
#define REFTIMES_PER_MILLISEC 10000

#define EXIT_ON_ERROR(hres) \
	if (FAILED(hres)) { \
		cout << "ERROR: " << hex << "0x" << hres << endl << \
		__FILE__ << ": " << \
		oct << __LINE__ << endl; goto Exit;}

#define SAFE_RELEASE(punk) \
	if ((punk) != NULL) \
		{ (punk)->Release(); (punk) = NULL;}

BYTE* pData;
const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);


/////////////////////////////////////////////////////////////////////////////

const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

typedef struct MyAudioSource {
	float sineFrequency_ = 880.0;
	int pos_ = 0;
	double cycleLength_ = 44100.0 / sineFrequency_;
	BYTE* pWaveData = NULL;
	int dataLenth = 0;

	WAVEFORMATEX* pwfx_ = NULL;

	void setFormat(WAVEFORMATEX* pwfx) {
		pwfx_ = pwfx;
		cycleLength_ = pwfx_->nSamplesPerSec / sineFrequency_;
	}
	void setData(BYTE* pData, int dataLen) {
		pWaveData = pData;
		dataLenth = dataLen;
	}
	void loadData(UINT32 numFramesAvailable, BYTE* pData, DWORD* flags) {
		float* out = (float*)pData;
		UINT32 numSamples = numFramesAvailable * pwfx_->nChannels;
		UINT32 bytesPerSample = pwfx_->wBitsPerSample / 8;
		UINT32 dataSize = numSamples * bytesPerSample;

		// 检查是否有足够的数据
		if ((pos_ + numSamples) * bytesPerSample > dataLenth) {
			// 处理数据不足的情况
			return;
		}

		// 将 16 位 PCM 转换为浮点数并复制到 out
		short* pSrcData = (short*)pWaveData;
		for (UINT32 i = 0; i < numSamples; ++i) {
			out[i] = (float)pSrcData[pos_ + i] / 32768.0f;
		}

		pos_ += numSamples;
	}

} MyAudioSource;

struct WAVEHEADER
{
	DWORD   dwRiff;                     // "RIFF"
	DWORD   dwSize;                     // Size
	DWORD   dwWave;                     // "WAVE"
	DWORD   dwFmt;                      // "fmt "
	DWORD   dwFmtSize;                  // Wave Format Size
};

//  Static RIFF header, we'll append the format to it.
const BYTE WaveHeader[] =
{
	'R',   'I',   'F',   'F',  0x00,  0x00,  0x00,  0x00, 'W',   'A',   'V',   'E',   'f',   'm',   't',   ' ', 0x00, 0x00, 0x00, 0x00
};

//  Static wave DATA tag.
const BYTE WaveData[] = { 'd', 'a', 't', 'a' };
bool ReadWaveFile(HANDLE FileHandle, BYTE* Buffer, const size_t BufferSize, const WAVEFORMATEX* WaveFormat)
{
	DWORD readLength;
	int si = sizeof(WaveHeader);
	ReadFile(FileHandle, Buffer, BufferSize, &readLength, NULL);

	if (Buffer == NULL)
	{
		printf("Unable to allocate %d bytes to hold output wave data\n", readLength);
		return false;
	}
	return true;
}

void ReadWaveData(BYTE* CaptureBuffer, size_t BufferSize, const WAVEFORMATEX* WaveFormat)
{
	HRESULT hr = NOERROR;
	char waveFileName[_MAX_PATH] = { 0 };
	sprintf_s(waveFileName, ".\\WAS_2024-04-07_17_23_08_826.wav");

	HANDLE waveHandle = CreateFileA(waveFileName, GENERIC_READ, 0,              // 不共享
		NULL,           // 默认安全属性
		OPEN_EXISTING,  // 只打开现有文件
		FILE_ATTRIBUTE_NORMAL, // 普通文件
		NULL);          // 没有模板文件
	if (waveHandle != INVALID_HANDLE_VALUE)
	{
		if (ReadWaveFile(waveHandle, CaptureBuffer, BufferSize, WaveFormat))
		{
			printf("Successfully wrote WAVE data to %s\n", waveFileName);
		}
		else
		{
			printf("Unable to write wave file\n");
		}
		CloseHandle(waveHandle);
	}
	else
	{
		printf("Unable to open output WAV file %s: %d\n", waveFileName, GetLastError());
	}

}

HRESULT PlayAudioStream(MyAudioSource* pMySource)
{
	HRESULT hr;
	REFERENCE_TIME hnsRequestDuration = REFTIMES_PER_SEC;
	REFERENCE_TIME hnsActualDuration;
	IMMDeviceEnumerator* pEnumerator = NULL;
	IMMDevice* pDevice = NULL;
	IAudioClient* pAudioClient = NULL;
	IAudioRenderClient* pRenderClient = NULL;
	WAVEFORMATEX* pwfx = NULL;
	UINT32 bufferFrameCount;
	UINT32 numFramesAvailable;
	UINT32 numFramesPadding;
	//BYTE* pData = nullptr;
	DWORD flags = 0;

	
	EXIT_ON_ERROR(CoCreateInstance(
		CLSID_MMDeviceEnumerator,
		NULL,
		CLSCTX_ALL,
		IID_IMMDeviceEnumerator,
		(void**)&pEnumerator));

	

	EXIT_ON_ERROR(pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice));

	EXIT_ON_ERROR(pDevice->Activate(IID_IAudioClient,
		CLSCTX_ALL,
		NULL,
		(void**)&pAudioClient));

	EXIT_ON_ERROR(pAudioClient->GetMixFormat(&pwfx));

	EXIT_ON_ERROR(pAudioClient->Initialize(
		AUDCLNT_SHAREMODE_SHARED,
		0,
		hnsRequestDuration,
		0,
		pwfx,
		NULL));
	//read
	int fileLenth = 0x3ab644;
	BYTE* waveFileData = new (std::nothrow) BYTE[0x3ab644];
	ReadWaveData(waveFileData, fileLenth, pwfx);


	BYTE* realWaveFileData = waveFileData+20*sizeof(BYTE);
	int realFileLenth = fileLenth - 20;
	// format to use
	pMySource->setFormat(pwfx);

	EXIT_ON_ERROR(pAudioClient->GetBufferSize(&bufferFrameCount));

	EXIT_ON_ERROR(pAudioClient->GetService(IID_IAudioRenderClient, (void**)&pRenderClient));

	EXIT_ON_ERROR(pRenderClient->GetBuffer(bufferFrameCount, &pData));
	pMySource->setData(realWaveFileData, realFileLenth);
	pMySource->loadData(bufferFrameCount, pData, &flags);

	EXIT_ON_ERROR(pRenderClient->ReleaseBuffer(bufferFrameCount, flags));

	// Calculate duration
	hnsActualDuration = (double)REFTIMES_PER_SEC * bufferFrameCount / pwfx->nSamplesPerSec;

	EXIT_ON_ERROR(pAudioClient->Start());

	// Loop to fill *ABOUT* half lol OMG
	while (flags != AUDCLNT_BUFFERFLAGS_SILENT)
	{
		// Sleep for half buffer duration -.-
		Sleep((DWORD)(10));

		// See how much space is available.
		EXIT_ON_ERROR(pAudioClient->GetCurrentPadding(&numFramesPadding));

		numFramesAvailable = bufferFrameCount - numFramesPadding;

		// Grab available space in shared buffer.
		EXIT_ON_ERROR(pRenderClient->GetBuffer(numFramesAvailable, &pData));

		// Get next 1/2 second of data.
		pMySource->loadData(numFramesAvailable, pData, &flags);

		EXIT_ON_ERROR(pRenderClient->ReleaseBuffer(numFramesAvailable, flags));
	}

	// Wait for last data in buffer to play before stopping.
	Sleep((DWORD)(hnsActualDuration / REFTIMES_PER_MILLISEC / 2));

	EXIT_ON_ERROR(pAudioClient->Stop());

Exit:
	CoTaskMemFree(pwfx);
	SAFE_RELEASE(pEnumerator)
		SAFE_RELEASE(pDevice)
		SAFE_RELEASE(pAudioClient)
		SAFE_RELEASE(pRenderClient)

		return 1;
}



///////////////////////////////////////////////////////////////////////////////////////////



typedef struct MyAudioSink {
	WAVEFORMATEX* pwfx_ = nullptr;

	void setFormat(WAVEFORMATEX* pwfx) {
		pwfx_ = pwfx;
	}

	void copyData(BYTE* pData, UINT32 numFramesAvailable, BOOL* bDone) {
		float* out = (float*)pData;

		for (int frames = 0; frames < numFramesAvailable * 2; ++frames) {
			printf("%f ", out[frames]);
		}
		printf("\n");
	}

} MyAudioSink;


HRESULT RecordAudioStream(MyAudioSink *pMySink)
{
	HRESULT hr;
	REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
	REFERENCE_TIME hnsActualDuration;
	UINT32 bufferFrameCount;
	UINT32 numFramesAvailable;
	IMMDeviceEnumerator* pEnumerator = NULL;
	IMMDevice* pDevice = NULL;
	IAudioClient* pAudioClient = NULL;
	IAudioCaptureClient* pCaptureClient = NULL;
	WAVEFORMATEX* pwfx = NULL;
	UINT32 packetLength = 0;
	BOOL bDone = FALSE;
	
	DWORD flags;

	EXIT_ON_ERROR(CoCreateInstance(CLSID_MMDeviceEnumerator,
		NULL,
		CLSCTX_ALL,
		IID_IMMDeviceEnumerator,
		(void**)&pEnumerator));

	EXIT_ON_ERROR(pEnumerator->GetDefaultAudioEndpoint(
		eCapture, eConsole, &pDevice));

	EXIT_ON_ERROR(pDevice->Activate(IID_IAudioClient,
		CLSCTX_ALL,
		NULL,
		(void**)&pAudioClient));

	EXIT_ON_ERROR(pAudioClient->GetMixFormat(&pwfx));

	EXIT_ON_ERROR(pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
		0,
		hnsRequestedDuration,
		0,
		pwfx,
		NULL));

	EXIT_ON_ERROR(pAudioClient->GetBufferSize(&bufferFrameCount));

	EXIT_ON_ERROR(pAudioClient->GetService(IID_IAudioCaptureClient,
		(void**)&pCaptureClient));

	pMySink->setFormat(pwfx);

	hnsActualDuration = (double)REFTIMES_PER_SEC * bufferFrameCount / pwfx->nSamplesPerSec;

	EXIT_ON_ERROR(pAudioClient->Start());

	while (bDone == FALSE)
	{
		Sleep(hnsActualDuration / REFTIMES_PER_MILLISEC / 2);

		EXIT_ON_ERROR(pCaptureClient->GetNextPacketSize(&packetLength));

		while (packetLength != 0)
		{
			EXIT_ON_ERROR(pCaptureClient->GetBuffer(&pData,
				&numFramesAvailable,
				&flags,
				NULL,
				NULL));

			if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
				pData = NULL;

			pMySink->copyData(pData, numFramesAvailable, &bDone);
			
			EXIT_ON_ERROR(pCaptureClient->ReleaseBuffer(numFramesAvailable));

			EXIT_ON_ERROR(pCaptureClient->GetNextPacketSize(&packetLength));
			static int num = 0;
			if (++num == 100) {
				bDone = true;
				break;
			}
		}
	}

	EXIT_ON_ERROR(pAudioClient->Stop());

Exit:
	CoTaskMemFree(pwfx);
	SAFE_RELEASE(pEnumerator)
		SAFE_RELEASE(pDevice)
		SAFE_RELEASE(pAudioClient)
		SAFE_RELEASE(pCaptureClient)

		return 0;
}


int _tmain(int argc, _TCHAR* argv[])
{

	CoInitializeEx(NULL, COINIT_SPEED_OVER_MEMORY);

	/*MyAudioSink pMySink;
	RecordAudioStream(&pMySink);*/

	MyAudioSource pMySource;
	PlayAudioStream(&pMySource);

	cout << "Press enter to exit." << endl;
	getchar();
	return 0;
}