# liftsensor

A hardware part of [Projekt Elevate](https://www.projekt-elevate.de) by
[Sozialhelden e.V.](https://sozialhelden.de).

## Setup

- Use a [Particle Electron](https://docs.particle.io/guide/getting-started/intro/electron/) as
  hardware
- Current development configuration is for Visual Studio Code on Mac. For other platforms, please
  extend this file ✍️
- Install the [po](https://github.com/nrobinson2000/po) utility. This will download the necessary
  dependencies automatically.
- Be sure to install the right GCC version:
    ```bash
    brew install nrobinson2000/newpo/gcc-arm-none-eabi-53
    brew link --overwrite nrobinson2000/newpo/gcc-arm-none-eabi-53
    ```

## How to run it

- Build with the configured build task, start with one of the configured test tasks
