# Polysynth

## Author

Hallmar Gauti Halldórsson



## Description
A 4 voice polysynth for the *[Electrosmith Daisy POD](https://www.electro-smith.com/daisy/pod)*  with an amplitude envelope per voice. It's controllable via MIDI USB or hardware MIDI and uses the round robin voice allocation method. My main focus is making a companion polysynth to use with my *[Monome Norns ](https://monome.org/docs/norns/)*:)


## Controls
| Control | Description | Comment |
| --- | --- | --- |
| Knob 1 | Envelope attack | |
| Knob 2 | Envelope decay | |
| Encoder | NA | |
| LED | Voice indicator | |

## To-Do
* change to Phase Modulation synthesis or Frequency Modulation
* map knob 1 to FM/PM amount
* map rotary encoder to ratio between operators
* switches change FM/PM algorithm

## Libraries used
[libDaisy](https://github.com/electro-smith/libDaisy) for MIDI handling and other miscellaneous functions.
[DaisySP](https://github.com/electro-smith/DaisySP) for audio DSP
  



