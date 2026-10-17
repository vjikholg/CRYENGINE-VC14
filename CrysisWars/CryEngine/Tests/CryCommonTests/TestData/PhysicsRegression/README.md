# CryPhysics Regression Suite

This suite characterizes the public `IPhysicalWorld` behavior before the
physics implementation is refactored or parallelized. It deliberately tests
through `CrySystem.dll` and `CryPhysics.dll`; it does not link against private
`CPhysicalWorld` or `CPhysicalEntity` implementation details.

## Running the suite

From the `CrysisWars` directory, run the test binary that matches the runtime
configuration:

```text
Bin64\Tests\Debug\CryCommonTests.exe --physics-regression Bin64 .
```

An optional fourth argument selects the TSV report path. Each scenario runs in
a new child process. A crash or timeout therefore fails that scenario without
preventing the remaining scenarios from running. Snapshot rows are flushed as
the scenario runs, which preserves the last observable state before a crash.

Do not compare Debug snapshots directly with Profile snapshots. Establish and
review a baseline for each configuration because compiler floating-point and
optimization choices can legitimately alter the least significant digits.

## Active foundation cases

| Case | Coverage |
| --- | --- |
| `world-interface` | CrySystem-to-CryPhysics loading and public interfaces |
| `convex-hull-buffer-growth` | Repeated production QHull growth, valid indices, outward winding, closed edges, point containment, and deterministic output |
| `static-body-invariance` | Static entity transform stability across steps |
| `rigid-free-fall` | Gravity, integration, velocity, and finite state |
| `impulse-response` | Impulse action, mass response, and zero-gravity motion |
| `floor-contact-settle` | Broadphase, narrow phase, contacts, solver, and sleep tendency |
| `stack-stability` | Multi-contact island stability and bounded resting error |
| `impulse-collision-pair` | Dynamic pair collision and linear-momentum symmetry |
| `spatial-query-lifecycle` | Entity IDs, grid queries, ray casts, deletion, and purge |
| `deterministic-replay` | Same-process replay from equivalent initial state |
| `living-movement-contact` | Living-entity grounding, movement, and pushing a rigid body |
| `partitioned-timesteps` | Equivalent one-second motion partitioned at 30, 60, and 120 Hz |
| `collision-event-ownership` | Collision callback entity ownership and ID lookup while the event is live |
| `destroy-during-callback` | Deferred entity destruction initiated from a collision callback |
| `contact-island-isolation` | One active contact island cannot move a disconnected resting island |
| `world-dump-roundtrip` | Text world dump save/load of a living entity, including ID and transform |
| `text-snapshot-bounds` | Null, zero, negative and undersized text-snapshot buffers return failure without changing output |

These assertions use physical invariants and deliberately broad tolerances.
The TSV contains the precise values needed to approve tighter golden ranges
after Debug and Profile baselines have been collected on the target machine.

## Phase 1 expansion matrix

The following cases should be added before large structural changes to the
corresponding subsystem. They are ordered by dependency and diagnostic value.

### Core world and solver

- Fixed-step versus partitioned-step behavior at 30, 60, and 120 Hz.
- Friction on level and inclined surfaces, including zero and high friction.
- Restitution below and above the minimum bounce-speed threshold.
- Sleeping bodies, wake by impulse, wake by contact, and wake propagation.
- Mixed mass ratios and center-of-mass offsets.
- Large contact islands, disconnected islands, and island merge/split.
- Constraint creation/removal during a step and invalid constraint targets.
- Out-of-grid entities, grid-cell boundaries, and broadphase reinsertion.
- Queued and immediate ray/primitive queries, including skip lists.

### Entity families

- `PE_LIVING`: ground contact, slopes, stairs, jump/fall, and push behavior.
- `PE_WHEELEDVEHICLE`: suspension rest state, wheel contacts, drive, braking,
  steering, and collision response.
- `PE_ARTICULATED`: joint limits, locked joints, ragdoll settling, impulses,
  and detach/re-attach behavior.
- `PE_ROPE`: free rope, endpoint attachment, collision, subdivision, and sleep.
- `PE_SOFT`: deformation, collision, attachment, and topology preservation.
- `PE_PARTICLE`: sweep collision, bounce, sliding, and ignored collider.
- `PE_AREA`: gravity override, buoyancy, water flow, overlap transitions, and
  global-area precedence.

### Events, structure, and persistence

- Collision, state-change, post-step, area, and deletion event counts/order.
- Logged versus immediate event delivery and safe callback removal.
- Breakable joints, part creation/removal, mesh updates, and mass conservation.
- World dump save/load round trip with entity IDs, parts, velocities, joints,
  material mappings, and grid configuration preserved.
- Network snapshot write/read round trip for rigid, living, vehicle, and
  articulated entities.

### Lifetime, limits, and concurrency

- Destroy during callbacks, repeated destroy, suspended entities, and purge.
- Geometry registration/reference ownership across add/remove/destroy paths.
- High entity/part counts and checked failure at supported public limits.
- Finite-value rejection for invalid parameters without corrupting world state.
- Repeated world setup and teardown under ASan where available.
- Single-thread versus island-parallel snapshots, event order, and ownership.
- ThreadSanitizer is not available in MSVC; use instrumented lock ownership,
  stress repetition, and a Clang-based build only when that build is viable.

## Baseline policy

1. Keep exact equality only for IDs, counts, classifications, and lifecycle.
2. Use tolerances for transforms, velocities, energies, and contact points.
3. Store scenario inputs and deterministic seeds in source control.
4. Compare invariant ranges before approving exact golden snapshots.
5. Treat a changed snapshot as a review event, not an automatic failure waiver.
6. Run every scenario in legacy single-thread mode before adding a parallel
   comparison mode.
7. Require ASan-clean lifetime tests before accepting ownership refactors.
