#include <chaos/StandardHeaders.h> 
#include <chaos/FileTools.h> 
#include <chaos/LogTools.h> 
#include <chaos/MyGLFWWindow.h> 
#include <chaos/WinTools.h> 
#include <chaos/Application.h>
#include <chaos/IrrklangTools.h>
#include <chaos/MIDITools.h>

class MyGLFWWindowOpenGLTest1 : public chaos::MyGLFWWindow
{

protected:

	virtual bool OnDraw(int width, int height) override
	{
		glClearColor(0.0f, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		return true;
	}
	
	static void CALLBACK OnMidiInEvent(HMIDIIN hMidiIn, UINT wMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
	{
		MyGLFWWindowOpenGLTest1 * self = (MyGLFWWindowOpenGLTest1 *)dwInstance;
		self->OnMidiInEventImpl(hMidiIn, wMsg, dwParam1, dwParam2);
	}

	void OnMidiInEventImpl(HMIDIIN hMidiIn, UINT wMsg, DWORD dwParam1, DWORD dwParam2)
	{
		switch (wMsg) 		
		{
			case MIM_OPEN: // reserved, reserved
			{
				chaos::LogTools::Log("wMsg = MIM_OPEN\n");
				break;
			}
			case MIM_CLOSE: // reserved, reserved
			{
				chaos::LogTools::Log("wMsg = MIM_CLOSE\n");
				break;
			}
			case MIM_ERROR:
			{
				DWORD dwMidiMessage = dwParam1;
				DWORD dwTimestamp = dwParam2;
				chaos::LogTools::Log("wMsg = MIM_ERROR\n");
				break;
			}
			case MIM_DATA:
			{


				DWORD dwMidiMessage = dwParam1;
				DWORD dwTimestamp   = dwParam2; // milliseconds

				chaos::MIDICommand command((uint32_t)dwMidiMessage);

				if (command.IsSystemMessage())
					break;


				chaos::LogTools::Log("status [%02x] command [%02x] channel [%02x] param1 [%02x] param2 [%02x] param3 [%02x]\n", command.status, command.GetCommand(), command.GetChannel(), command.param1, command.param2, command.param3);
				break;
			}
			case MIM_LONGDATA:
			{
				chaos::LogTools::Log("wMsg = MIM_LONGDATA\n");
				break;
			}
			case MIM_LONGERROR:
			{
				chaos::LogTools::Log("wMsg = MIM_LONGERROR\n");
				break;
			}
			case MIM_MOREDATA:
			{
				chaos::LogTools::Log("wMsg = MIM_MOREDATA\n");
				break;
			}
			default:
			{
				chaos::LogTools::Log("wMsg = unknown\n");
				break;
			}
		}
		return;
	}

	virtual bool Initialize() override
	{
		// enumerate the midi IN devices
		UINT midi_device_count = midiInGetNumDevs();
		for (UINT i = 0; i < midi_device_count; ++i)
		{
			MIDIINCAPS caps;
			midiInGetDevCaps(i, &caps, sizeof(MIDIINCAPS));

			chaos::LogTools::Log("Midi IN Device [%d] : name    = %s", i, caps.szPname);
			chaos::LogTools::Log("                    : support = %d", caps.dwSupport);
			chaos::LogTools::Log("                    : driver  = %d", caps.vDriverVersion);
			chaos::LogTools::Log("                    : mid     = %d", caps.wMid);
			chaos::LogTools::Log("                    : pid     = %d", caps.wPid);
		}

		MMRESULT rv;
		// open the device
		rv = midiInOpen(&hMidiDevice, nMidiPort, (DWORD_PTR)(void*)OnMidiInEvent, (DWORD_PTR)this, CALLBACK_FUNCTION);
		if (rv != MMSYSERR_NOERROR)
			return false;

	#if 0		
		LPSTR       lpData;               /* pointer to locked data block */
		DWORD       dwBufferLength;       /* length of data in data block */
		DWORD       dwBytesRecorded;      /* used for input only */
		DWORD_PTR   dwUser;               /* for client's use */
		DWORD       dwFlags;              /* assorted flags (see defines) */
		struct midihdr_tag far *lpNext;   /* reserved for driver */
		DWORD_PTR   reserved;             /* reserved for driver */

	#if (WINVER >= 0x0400)
		DWORD       dwOffset;             /* Callback offset into buffer */
		DWORD_PTR   dwReserved[8];        /* Reserved for MMSYSTEM */
	#endif
	

		MIDIHDR hdr;
		rv = midiInAddBuffer(hMidiDevice, &hdr, sizeof(hdr));
		if (rv != MMSYSERR_NOERROR)
			return false;
	#endif

		// start device usage
		midiInStart(hMidiDevice);

	#if 0
		// reset the recording and mark all pending buffer as done
		midiInReset(hMidiDevice);
		// stop the recording
		midiInStop(hMidiDevice);
	#endif

		return true;
	}

	virtual void Finalize() override
	{
		midiInStop(hMidiDevice);
		midiInClose(hMidiDevice);
		hMidiDevice = nullptr;
	}

	virtual void TweakSingleWindowApplicationHints(chaos::MyGLFWWindowHints & hints, GLFWmonitor * monitor, bool pseudo_fullscreen) const override
	{
		chaos::MyGLFWWindow::TweakSingleWindowApplicationHints(hints, monitor, pseudo_fullscreen);
		hints.toplevel = 0;
		hints.decorated = 1;
	}

protected:

	HMIDIIN hMidiDevice{ nullptr };
	DWORD nMidiPort{ 0 };
};

int _tmain(int argc, char ** argv, char ** env)
{
	chaos::Application::Initialize<chaos::Application>(argc, argv, env);

	chaos::WinTools::AllocConsoleAndRedirectStdOutput();

	chaos::MyGLFWSingleWindowApplicationParams params;
	params.monitor = nullptr;
	params.width = 500;
	params.height = 500;
	params.monitor_index = 0;
	chaos::MyGLFWWindow::RunSingleWindowApplication<MyGLFWWindowOpenGLTest1>(params);

	return 0;
}


