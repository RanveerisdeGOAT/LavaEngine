# LavaEngine — Current Issues & Work Tracker

> **Snapshot:** 2026-09-17  
> **Repository:** https://github.com/RanveerisdeGOAT/LavaEngine  
> **Version observed:** `0.7.1-indev`
>
> This file tracks issues, gaps, and engineering work identified from the current repository structure, README, changelog, and public project state.
>
> **Important:** Items marked as architectural/feature gaps are not necessarily bugs. They are things that should be clarified, implemented, tested, or documented as LavaEngine matures.

---

## Status Legend

- `[ ]` Open
- `[~]` In progress / partially implemented
- `[x]` Complete
- `[?]` Needs verification

## Priority Legend

- **P0 — Critical:** blocks reliable use of the framework
- **P1 — High:** important for the next usable milestone
- **P2 — Medium:** important for robustness/usability
- **P3 — Low:** polish, convenience, or long-term work
- **IDX** issue id.

---

## [~] ID1 - Define and enforce ownership/lifetime rules

**Priority:** P0  
**Area:** Core / Containers / Modules / Resources

The 0.7.1 changelog says that several lifetime issues were fixed, but the architecture relies heavily on Containers, Modules, Resources, shared references/handles, and cross-container access.

We need a documented and tested ownership model covering:

- Container destruction
- Module destruction
- Resource destruction
- Resource handles after owner destruction
- Imported/exported resources
- References held by scheduled jobs
- References held by Inspector code
- References held by events
- Cross-container resource access
- Shutdown ordering

### Acceptance criteria

- No use-after-free in core lifecycle paths.
- Resources have an unambiguous owner/lifetime policy.
- Jobs cannot outlive objects they capture unless explicitly supported.
- Destruction order is deterministic and documented.
- Regression tests cover lifetime edge cases.

---

## [ ] ID2 - Define scheduler dependency and execution semantics

**Priority:** P0  
**Area:** Scheduler

The README establishes the scheduler as a fundamental part of LavaEngine, but the public design does not yet fully specify:

- Job ordering
- Dependencies between jobs
- Whether jobs may execute concurrently
- Thread affinity
- Cancellation
- Error propagation
- Shutdown behavior
- Re-entrancy
- Long-running jobs
- Resource lifetime while jobs are running

The current scheduler example also uses return values to determine whether a job repeats. This needs a precise contract.

---

## [ ] ID3 - Scheduler is not re-entrant (iterator invalidation / UB)

**Priority:** P0  
**Area:** Scheduler

Concrete hazards found while reviewing `src/Scheduler.cpp`:

- `execute()` (`src/Scheduler.cpp:135-180`) range-iterates `m_jobs` holding a live `Job&`. If `job.task()` calls `createJob()` the `std::vector` can reallocate, and `destroyJob()` erases while the reference is still in use — both invalidate the iterator and the reference → UB.
- `destroyJob()` (`src/Scheduler.cpp:90-119`) calls `it->exit()` (line 106) *before* the erase at line 118. If `exit` re-enters the scheduler (`createJob`/`destroyJob`), `it` is invalidated by the time it is dereferenced/erased.
- `exitAll()` (`src/Scheduler.cpp:122-132`) has the same problem: `exit()` callbacks that mutate `m_jobs` invalidate the range-for.
- `findJob()` (`src/Scheduler.cpp:45-54`) hands out a `Job*` that any subsequent `createJob`/`destroyJob` invalidates; callers cannot detect the staleness.

### Suggested fixes

- Document that jobs must not mutate the scheduler (and enforce with an assertion), or
- Defer mutation (queue new jobs / destroyed ids) until after the iteration completes, or
- Iterate by index and guard against `recreate`-style reallocation.

---

## [ ] ID4 - Container/ModuleRegistry moves leave Modules pointing at the moved-from Container

**Priority:** P1  
**Area:** Containers / Modules

`Container::Container(Container&&)` (`src/Container.cpp:9-29`) moves `m_modules`/`m_resources` but never re-points each Module's `m_container` back at the *new* container. `ModuleRegistry`'s move ops (`src/Module.cpp:14-49`) copy `m_container` verbatim and call `module->setContainer(m_container)`, so every contained module keeps referencing the **moved-from, doomed** container.

After any move, `Module::getContainer()` (`include/LavaEngine/Module.hpp:18-26`) returns a stale container; the next use is use-after-free.

### Suggested fixes

- After moving, walk all modules and `setContainer(this)`.
- Add a regression test moving a container with `addModule<T>`'d modules and verifying `getContainer()`.
- Consider making `Container` non-movable (its name/ownership also make moves awkward) and storing it behind a `unique_ptr` in `Application`.

---

## [ ] ID5 - `typeID<T>()` uses `typeid(T).hash_code()`, which is collision- and DSO-unsafe

**Priority:** P1  
**Area:** Modules / Hot-reloading

`include/LavaEngine/Module.hpp:42-48` keys the `ModuleRegistry` on `typeID<T>()`:

```c++
constexpr TypeID typeID() { return typeid(T).hash_code(); }
```

Two problems:

1. `hash_code()` is not guaranteed unique even within a single binary. A collision makes `ModuleRegistry::add` throw "Module already exists" (`Module.hpp:74-77`) for a legitimately new type, or worse, `get<T>`/`remove<T>` (`Module.hpp:94-154`) `static_cast` the wrong type — UB.
2. `type_info` identity is not guaranteed stable across shared-library boundaries. With hot-reloading (`lavac/src/lavac.h:53`, `dlopen` of a game `.so`), a type registered from the game module can hash to a different value than the same type compiled into the engine — silent `get/has/remove` failures under hot reload.

### Suggested fixes

- Replace with a compile-time unique counter (`__COUNTER__`/counter template) or explicit registration macro, or
- A name-based ID, or
- `std::type_index` (itself based on `type_info`, so it does **not** solve the DSO problem — a manual scheme is required).

---

## [ ] ID6 - `importResource`/`expose` hand out raw pointers with no lifetime guarantee

**Priority:** P0  
**Area:** Containers / Resources / Inspector

These are concrete instances of the lifetime hazards ID1 calls out:

- `Container::importResource` (`include/LavaEngine/Container.hpp:122-137`) returns a raw `T*` into the *exporter's* `ResourceRegistry`. No shared ownership, no refcount, no handle: if the exporter `remove()`s the resource or is destroyed, the importer dangles.
- `Container::expose` (`src/Container.cpp:36-54`) stores caller-supplied raw `void*` in `m_variables`. If the pointee (e.g. a Module member) is destroyed first (`removeModule`, container teardown, hot reload), the Inspector later reads/writes through a freed pointer. The Inspector's bool/int/float/double widgets (`lavac/inspector/src/Inspector.h:656-663`) write through these pointers every frame.

### Suggested fixes

- `importResource` should return a shared/lifetime-tracked handle (or a `shared_ptr`/observer that can be queried for validity) instead of a raw pointer.
- `expose` should either own a copy of the value (with write-back), take a `std::shared_ptr`/`weak_ptr`, or at minimum auto-deregister on teardown; document that exposed pointers must outlive the Container.

---

## [ ] ID7 - Logger accessors are not thread-safe

**Priority:** P1  
**Area:** Logger

`Logger::log` (`src/Logger.cpp:62-104`) reads `m_level`, `m_consoleEnabled`, `m_console` under `m_mutex`, but `setLevel`, `level`, `setConsoleEnabled`, `setFileEnabled`, and `setOutput` (`src/Logger.cpp:32-55`) read/write those same fields **without** the lock. Under concurrent logging + configuration this is a data race (UB). `setOutput` can also repoint `m_console` to a stream that is destroyed while `log()` is still flushing.

### Suggested fixes

- Take the mutex in every accessor (or make the hot fields `std::atomic`).
- Document that the redirected output stream must outlive the `Logger`.

---

## [ ] ID8 - `Window::~Window` calls `glfwTerminate()` unconditionally

**Priority:** P2  
**Area:** Window / GLFW lifecycle

`src/Window.cpp:61-67` terminates all of GLFW in the destructor. If more than one `Window` is alive (or a `Window` is moved from and the source is destroyed first), the first destruction tears the entire GLFW context out from under the surviving window(s) — crashes on subsequent `glfw` calls.

Note: `VulkanRenderer` owns a `Window` by value, and LavaVK examples also call `glfwTerminate()` themselves.

### Suggested fixes

- Reference-count `glfwInit`/`glfwTerminate` (e.g. a RAII `GlfwScope` shared by all windows), and only terminate when the last window is destroyed.

---

## [ ] ID9 - `InputHandler`/`Window` moves lose state

**Priority:** P2  
**Area:** Window / InputHandler

- `InputHandler`'s move ctor/assignment (`src/InputHandler.cpp:18-50`) transfer the window pointer and key/mouse state but **not `m_focused`**, which silently resets to `true`.
- `Window`'s move ctor (`src/Window.cpp:93-98`) does not transfer `m_lastFrameTime` (`include/LavaEngine/Window.hpp:72`), so the first `tick()` after a move returns a huge `dt`.

### Suggested fixes

- Transfer `m_focused` and `m_lastFrameTime` in the move ops (or initialize `m_lastFrameTime` from `glfwGetTime()` on move to keep `dt` sane).

---

## [ ] ID10 - `Application::run()`/`step()` busy-spin and can never terminate

**Priority:** P1  
**Area:** Application / Scheduler

`Application::run()` (`include/LavaEngine/Application.hpp:41-51`) loops `while (completed < jobs.size()) execute();`:

- Any job that re-runs forever (returns `0` — per `Scheduler::execute`, `src/Scheduler.cpp:172-178`, only `result > 0` marks a job complete; negatives are treated as "repeat too") spins at 100% CPU with no pacing and no error path.
- A task that calls `createJob` during `execute()` grows `m_jobs`, so the loop can fail to converge.
- An empty scheduler has `0 == 0`; `run()`/`step()` (`Application.hpp:55`) treat it as "done" instantly, which also defeats hot-reload inspection (zero registered jobs ⇒ immediate exit).

### Acceptance criteria

- `run()` should have a deterministic termination contract (e.g. a job explicitly cancels/returns "done").
- Long-running jobs should yield (frame structure) rather than busy-spin.
- Newly created jobs during execution should be handled explicitly (see ID3).
