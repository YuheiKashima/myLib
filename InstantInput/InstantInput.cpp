#include "InstantInput.h"

using namespace myLib;
using namespace std;

InstantInput::InstantInput() :m_PrimaryKeyState{}, m_SecondryKeyState{} {
}

InstantInput::~InstantInput() {
}

bool InstantInput::UpdateState() {
	bool res = GetKeyboardState(static_cast<PBYTE>(m_PrimaryKeyState.data()));
	return res;
}

void InstantInput::Ready() {
	std::copy(m_PrimaryKeyState.begin(), m_PrimaryKeyState.end(), m_SecondryKeyState.begin());
	std::fill(m_PrimaryKeyState.begin(), m_PrimaryKeyState.end(), 0);
}

bool InstantInput::GetPress(BYTE _key, bool _debug) {
	if (_debug)
		std::cout << "GetPress: " << static_cast<char>(_key) << " state:" << (_key & m_Compare) << std::endl;
	return GetKeyState(_key) & m_Compare ? true : false;
}

bool InstantInput::GetTrigger(BYTE _key, bool _debug) {
	if (_debug)
		std::cout << "GetTrigger: " << static_cast<char>(_key) << " state:" << (_key & m_Compare) << std::endl;
	return (m_PrimaryKeyState[_key] & m_Compare) && !(m_SecondryKeyState[_key] & m_Compare) ? true : false;
}

bool InstantInput::GetRelease(BYTE _key, bool _debug) {
	if (_debug)
		std::cout << "GetRelease: " << static_cast<char>(_key) << " state:" << (_key & m_Compare) << std::endl;
	return !(m_PrimaryKeyState[_key] & m_Compare) && (m_SecondryKeyState[_key] & m_Compare) ? true : false;
}