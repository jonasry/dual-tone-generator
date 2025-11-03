## Dual-Tone Generator

This project builds a dual-tone audio generator as an AU component. The AU generates 2 sine waves simultaneously. The user can control:

- Frequency 1: The frequency of wave 1 (30-400 Hz).
- Frequency 2: The frequency of wave 2 (30-400 Hz).
- Pan 1: The panning (L/R) of wave 1.
- Pan 2: The panning (L/R) of wave 2.
- Gain: Overall gain control (0-100 %).

Pan controls are only available if the AU is on a stereo bus (or higher # channels).

If on a mono bus, the pan controls are unavailable and the signals are summed to the mono channel.

If on a stereo bus, the pans are by default set so wave 1 is panned hard left and wave 2 is panned hard right.

The frequencies of wave 1 and wave 2 can be controlled independently but should initially be set to:

- Frequency 1: 98 Hz
- Frequency 2: 102 Hz

Gain should intially be set to 1.
