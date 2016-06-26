#include <array>
#include <memory>

#include <iostream>

#include <Windows.h>
#include <mmsystem.h>

#define MCI_CAP_ARGS(X) &(X), sizeof(X)

namespace {

std::ostream& operator << (std::ostream& out, MMRESULT result) {
  std::array<wchar_t, MAXERRORLENGTH> message;
  midiInGetErrorText(result, message.data(),
                     static_cast<UINT>(message.size()));
  return out << message.data();
}

template <typename HMIDI>
struct midi_device;

template <>
struct midi_device<HMIDIIN> {
  static constexpr decltype(&midiInGetNumDevs) count = ::midiInGetNumDevs;
  static constexpr decltype(&midiInGetDevCaps) info = ::midiInGetDevCaps;
  static constexpr decltype(&midiInOpen) open = ::midiInOpen;
  static constexpr decltype(&midiInClose) close = ::midiInClose;

  using caps = MIDIINCAPS;

  using type = std::unique_ptr<
    std::remove_pointer_t<HMIDIIN>,
    decltype(&midiInClose)>;
};


template <>
struct midi_device<HMIDIOUT> {
  static constexpr decltype(&midiOutGetNumDevs) count = ::midiOutGetNumDevs;
  static constexpr decltype(&midiOutGetDevCaps) info = ::midiOutGetDevCaps;
  static constexpr decltype(&midiOutOpen) open = ::midiOutOpen;
  static constexpr decltype(&midiOutClose) close = ::midiOutClose;

  using caps = MIDIOUTCAPS;

  using type = std::unique_ptr<
    std::remove_pointer_t<HMIDIOUT>,
    decltype(&midiOutClose)>;
};

template <typename MIDI>
using midi_device_t = typename midi_device<MIDI>::type;

template <typename MIDI>
midi_device_t<MIDI> open_midi_device(UINT i) {
  MIDI hMidi = nullptr;
  if (MMRESULT const retval = midi_device<MIDI>::open(&hMidi, i, 0, 0, CALLBACK_NULL)) {
    std::wcerr << retval << L'\n'; // throw errors
  }
  return midi_device_t<MIDI>(hMidi, midi_device<MIDI>::close);
}

}

int main() {
  {
    using device_type = midi_device<HMIDIIN>;
    for (UINT i = 0, count = device_type::count(); i < count; ++i) {
      device_type::caps caps;
      if (MMRESULT const retval = device_type::info(i, MCI_CAP_ARGS(caps))) {
        std::wcerr << retval << L'\n';
        continue;
      }

      std::wclog << L"in[" << i << L"]: " << caps.szPname << L" ("
        << caps.wMid << L'.' << caps.wPid << L'.' << caps.vDriverVersion << L")\n";


      auto const device = open_midi_device<HMIDIIN>(i);
    }
  }
  {
    using device_type = midi_device<HMIDIOUT>;
    for (UINT i = 0, count = device_type::count(); i < count; ++i) {
      device_type::caps caps;
      if (MMRESULT const retval = device_type::info(i, MCI_CAP_ARGS(caps))) {
        std::wcerr << retval << L'\n';
        continue;
      }

      std::wclog << L"out[" << i << L"]: " << caps.szPname << L" ("
        << caps.wMid << L'.' << caps.wPid << L'.' << caps.vDriverVersion << L")\n";

      auto const device = open_midi_device<HMIDIOUT>(i);
    }
  }

  return 0;
}
