# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/)
and this project adheres to [Semantic Versioning](http://semver.org/).

## [2.3.3] - 2025-06-26
- Fix quick switch not working [#139](https://github.com/svenstaro/rofi-calc/pull/139) (thanks @zspher)

## [2.3.2] - 2025-06-12
- Switched from autotools to meson [#136](https://github.com/svenstaro/rofi-calc/pull/136) (thanks @zspher)

## [2.3.1] - 2025-06-06
- Specify mode type [#134](https://github.com/svenstaro/rofi-calc/pull/134) (thanks @zspher)

## [2.3.0] - 2025-01-09
- Fix multiline outputs only showing the first line [#128](https://github.com/svenstaro/rofi-calc/pull/128) (thanks @jdholtz)
- Error Message Color [#121](https://github.com/svenstaro/rofi-calc/pull/121) (thanks @sornig)

## [2.2.1] - 2024-03-28
- Fix skip escaping some unicode math symbols [#109](https://github.com/svenstaro/rofi-calc/pull/109) (thanks @arades79)
- Fix functions crashing with libqalculate 5 [#117](https://github.com/svenstaro/rofi-calc/issues/117) (thanks @tiosgz)

## [2.2.0] - 2023-04-27
- Fix high CPU usage when input isn't empty [#66](https://github.com/svenstaro/rofi-calc/pull/66) (thanks @bootstrap-prime)
- Add `-automatic-save-to-history` option [#87](https://github.com/svenstaro/rofi-calc/pull/87) (thanks @matejdro)

## [2.1.0] - 2022-02-06
- Correctly handle ≈ signs emitted by qalc [#78](https://github.com/svenstaro/rofi-calc/pull/78) (thanks @SabrinaJewson)
- Add `-calc-command-history` to add result to history when using `-calc-command` [#79](https://github.com/svenstaro/rofi-calc/pull/79) (thanks @SabrinaJewson)
- Correctly handle multiple equals signs emitted by qalc [#80](https://github.com/svenstaro/rofi-calc/pull/78) (thanks @SabrinaJewson)

## [2.0.0] - 2020-11-14
- Add option to completely disable history [#63](https://github.com/svenstaro/rofi-calc/pull/63) (thanks @UnkwUsr)

## [1.9] - 2020-09-24
- Add options to specify output hints [#59](https://github.com/svenstaro/rofi-calc/pull/59) (thanks @sa5gap)

## [1.8] - 2020-08-13
- Enable qalc's Unicode mode by default
- Add `-no-unicode` option to disable aforementioned unicode support

<!-- next-url -->
[Unreleased]: https://github.com/svenstaro/rofi-calc/compare/v2.3.0...HEAD
[2.3.0]: https://github.com/svenstaro/rofi-calc/compare/v2.2.1...v2.3.0
[2.2.1]: https://github.com/svenstaro/rofi-calc/compare/v2.2.0...v2.2.1
[2.2.0]: https://github.com/svenstaro/rofi-calc/compare/v2.1.0...v2.2.0
[2.1.0]: https://github.com/svenstaro/rofi-calc/compare/v2.0.0...v2.1.0
[2.0.0]: https://github.com/svenstaro/rofi-calc/compare/v1.9...v2.0.0
[1.9]: https://github.com/svenstaro/rofi-calc/compare/v1.8...v1.9
