#ifndef _INSTANTINPUT_
#define _INSTANTINPUT_

#include <Windows.h>
#include <WinUser.h>
#include <iostream>
#include <array>

namespace myLib {
	class InstantInput {
	public:
		InstantInput();
		~InstantInput();
		bool UpdateState();
		void Ready();
		bool GetPress(BYTE _key, bool _debug = false);
		bool GetTrigger(BYTE _key, bool _debug = false);
		bool GetRelease(BYTE _key, bool _debug = false);
	private:
		const BYTE m_Compare = 0x80;
		std::array<BYTE, 256> m_PrimaryKeyState, m_SecondryKeyState;
	};
}

#endif // _INSTANTINPUT_