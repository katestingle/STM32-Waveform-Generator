# STM32-Waveform-Generator

## Demos
* [Demo of power on/reset wave config, square wave w/ freq. And real-time duty cycle adjustments](https://photos.app.goo.gl/CojrdDdtn8js7mna8)
* [Demo of alternative waveforms (no freq adjustments)](https://photos.app.goo.gl/SeHq8xfanmppTJT27)
* [Demo of alt. Waveform freq.](https://photos.app.goo.gl/jbnCms4gKhL9R29D8)
* [Demo of alt. Waveforms & rejection of duty cycle adjustment](https://photos.app.goo.gl/geoSsVtTvT2Nrvjh7)

---

## Behavior Description: Short Synopsis
The behavior of the function generator is the same output that one would see out of a normally-operated waveform generator. There is an interface to which you can set the waveform type, frequency, and duty cycle (done via the keypad), and there is a resulting voltage observable at the $V_{OUT}$ node of the digital-to-analog converter (DAC) where that waveform is generated, observable with a time-variant voltage measuring device like an oscilloscope.

### Waveform Configuration Specifications
* **Waveform Types:** Square wave (with variable duty cycle), sinusoid, sawtooth (ramp), and triangle.
* **Adjustable Frequencies:** 100 Hz, 200 Hz, 300 Hz, 400 Hz, 500 Hz ($\pm$ 2.5 Hz in error).
* **Voltage Levels:** All waveforms have a $V_{pp}$ of 3.0 V and are DC-biased at 1.5 V.
* **Default Power-Up State:** 100 Hz square wave with a 50% duty cycle.

---

## System Schematic
<img width="1641" height="947" alt="image" src="https://github.com/user-attachments/assets/e8e1c1bc-bead-4e44-92ad-acaf9ae874c1" />

---

## Software Architecture
We utilized our previous `SPI_DAC` library, `tim2` library, and `keypad` library for this project. We also wrote a new `wave_generator` library that integrates and abstracts away the `tim2` and `SPI_DAC` libraries, allowing for an easy `wave_generator` API from `main`.

### Execution Flow
* **Initialization:** The main program handles keypad and `wave_generator` initializations (`wave_generator` internally configures the `tim2` and `spi_dac` settings).
* **Main Loop:** The program enters a `while` loop that continuously polls the keypad and feeds that input directly into `update_waveform`. This essentially allows the keypad library to listen for requests from the keypad and direct the `waveform_generator` library on what to do with $V_{OUT}$ on the DAC.

### Waveform Generator Library Structure
There is a global state `struct` that describes the current waveform being outputted on the DAC. On initialization, this waveform structure is instantiated with values that default to outputting a 50% duty cycle, 100 Hz square wave.
