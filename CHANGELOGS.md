# LavaEngine - Changelog

> **Snapshot**: 2026-09-17
> **Repository**: https://github.com/RanveerisdeGOAT/LavaEngine
> **Version observed**: 0.7.2-indev
>
> This file contains all notable changes to LavaEngine are documented here.

## [0.8.0-indev] - 17/09/26

### Added:
* Generation-checked `ResourceHandle`s and `ResourceView<T>` weak borrows.
* `Container::importResource` returns a `ResourceView<T>` instead of a raw pointer.
* Teardown hooks `Container::onUnload()`/`Module::onUnload()` and phased, documented `Application::unloadGame()`.
* `expose()` boxes values into Container-owned storage, including a new `double` overload.
* `Inspector::detach()`.
* Regression test suite (`tests/`) wired into CTest.
* Documented the teardown/ownership model in `README.md`.

### Fixed:
* Modules kept pointing at the moved-from Container after a move (ID4).
* Stale resource handles could alias recreated resources (ID1).
* `importResource`/`expose` no longer hand out dangling raw pointers (ID6).
* Inspector edited a freed pointer on removed/teardown'd exposed variables, and corrupted `std::string` variables via a char-buffer cast (ID6).
* Inspector toolbar tooltip varargs format mismatch.

## [0.7.2-indev] - 17/09/26

### Added:
* Added issue tracker in `ISSUES.md`.

## [0.7.1-indev] - 16/09/26

### Fixed:
* Small progress to fixing [ID1].

## [0.7.0-indev] - 14/09/26

### Added:
* Notable improvements to the inspector.
    - Logger
    - Explorer
    - File Inspector
* Added Logger
* Added InputHandler

## [0.6.0-indev] - 08/09/26

### Added:
* Added Inspector

## [0.5.0-indev] - 31/08/26

### Added:
* Added Frameworks
* Added Hot-Reloading

## [0.4.0-indev] - 30/08/26

### Added:
* Added Lava Compiler `lavac` to run .so games.

## [0.3.0-indev] - 30/08/26

### Added:
* Added scheduler
* Added GraphicalPiplineModule

## [0.2.0-indev] - 29/08/26

### Added:
* Added window manager
* Implemented containers
* Implemented Modules
* Started implementing resources
* Added Renderer module

## [0.1.0-indev] - 28/08/26
* Initial commit


