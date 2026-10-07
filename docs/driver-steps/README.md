# Driver steps

The sound driver grows one channel or one effect at a time. Each step has a plan file listing its acceptance criteria, which are the golden traces the driver must match. A trace comes from the reference player and is checked by the `driver.<song>` test (`ctest -R driver`).

| Step | Adds | Acceptance | Status |
|---|---|---|---|
| [00](00-harness.md) | Test harness, init-only driver | `driver.silence` | Done |
| [01](01-pulse1-scale.md) | Channel 1: notes, instruments, rows, orders, note off (`C`) | `driver.scale` | Done |
| [02](02-pulse2.md) | Channel 2, per-channel state | `driver.pulses` | Done |
| [03](03-wave.md) | Channel 3 and wave loading | `driver.wave` | Done |
| [04](04-noise.md) | Channel 4 and the noise table | `driver.noise`, `driver.minimal`, `driver.instruments` | Done |
| [05](05-flow.md) | Tempo (F), position jump (B), pattern break (D) | `driver.flow`, `driver.jumps` | Done |
| [06](06-register-effects.md) | Effects 5, 6, 8, 9, C on every channel, folded triggers | `driver.registers`, `driver.effects` | Done |
| [07](07-budgets.md) | Cycle and size budgets enforced by the tests | Cycle limit in every `driver.*` test, `driver.size` | Done |
| [08](08-note-cut.md) | Note cut (E), the first timed effect | `driver.cut` | Done |
| [09](09-note-delay.md) | Note delay (7), the second timed effect | `driver.delay` | Done |

## Rules for every step

- **Oracle first.** A behaviour must exist in [driver-contract.md](../driver-contract.md) and in the reference player, with a golden trace in `tests/songs/`, before the driver implements it. The trace is generated with `golem-trace` and reviewed by hand.
- **Acceptance.** The step's `driver.*` tests pass, and so do all tests from earlier steps. The build, `ctest` and `format-check` pass.
- **Registering a test.** A song becomes a driver test by adding `golem_driver_test(<song>)` to [driver/CMakeLists.txt](../../driver/CMakeLists.txt). Only songs the driver fully supports are registered.
- **TDD.** Interface first, then register the failing test, then implement until the trace matches.

## Later steps (oracle first)

These effects are not specified yet. For each one, the contract, the reference player and a golden trace come first, then the driver step:

- 0 (arpeggio)
- 1, 2, 3 (portamento up, down, and tone portamento)
- 4 (vibrato)
- A (volume slide)
