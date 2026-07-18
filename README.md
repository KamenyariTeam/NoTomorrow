# No Tomorrow

No Tomorrow is an early-stage top-down immersive sim about planning and executing heists in a collapsing world.

The project is currently focused on building its gameplay and tooling foundations. Heists, anomalies, systemic interactions, branching narrative, and possible future co-op are design goals rather than a list of completed features.

## Current technical foundation

- `NoTomorrowGame`: runtime module with game framework classes, Enhanced Input, gameplay-camera support, asset/experience loading, gameplay tags, Game Feature actions, and the current HUD foundation.
- `NoTomorrowEditor`: editor module with a custom editor engine, common-map toolbar support, PIE setup, and editor tooling dependencies.
- `Plugins/GameplayCore`: reusable gameplay-experience loading, tagged gameplay events, Blueprint async support, an uncooked Blueprint-node module, and related automation tests.
- `Plugins/ModularGameplayActors`: reusable modular actor, pawn, character, controller, GameMode/GameState, HUD, and component bases.
- Engine integrations include Common UI, Game Features, Modular Gameplay, Enhanced Input, Gameplay Tags, and Gameplay Cameras.

C++ implements the engine-facing foundations and reusable systems. Blueprints and content configure concrete game classes, the default character, input and camera assets, the default gameplay experience, cursors, and maps.

## Requirements

- Unreal Engine 5.8, using the project's associated Engine build.
- JetBrains Rider with Unreal Engine support (primary IDE workflow).
- A compatible Windows C++ toolchain: MSVC compiler, Windows SDK, and Unreal's required C++ components. Rider is the IDE and does not replace the compiler/toolchain.

## Open and build with Rider

1. Clone the repository, including Git LFS content if the remote uses LFS.
2. Open `NoTomorrow.uproject` in Rider. Select or register the Unreal Engine 5.8 installation/source checkout when prompted.
3. Let Rider index the Engine and project, then select the `NoTomorrowEditor` target, `Development Editor` configuration, and `Win64` platform.
4. Build and run the Editor from Rider. Use Play In Editor for normal iteration.

Unreal-generated solution and project files are local output and are not authoritative. Regenerate them through Rider or Unreal's project-file generation when required.

### Command-line build

When a terminal build is useful, set `UE_ROOT` to the root of the Engine installation or source checkout that owns `Engine/Build/BatchFiles/Build.bat`. From the repository root in PowerShell:

```powershell
& "$env:UE_ROOT\Engine\Build\BatchFiles\Build.bat" NoTomorrowEditor Win64 Development -Project="$PWD\NoTomorrow.uproject" -WaitMutex
```

Do not copy another developer's absolute Engine path. Rider's Engine association or the local `.uproject` registration is the source for locating it.

## Important folders

| Path | Purpose |
| --- | --- |
| `Source/NoTomorrowGame/` | Runtime C++ module: character, camera, input, game modes, Game Feature actions, systems, player, and UI foundations |
| `Source/NoTomorrowEditor/` | Editor-only module and custom editor engine/tooling |
| `Plugins/GameplayCore/` | Gameplay experiences, tagged events, Blueprint nodes, tests, and plugin content |
| `Plugins/ModularGameplayActors/` | Reusable Modular Gameplay actor and component bases |
| `Config/` | Engine, game, input, gameplay-tag, and asset-manager configuration |
| `Content/Characters/` | Current character Blueprints and camera assets |
| `Content/Input/` | Enhanced Input actions, mapping contexts, and input data |
| `Content/System/` | Default map and gameplay-experience assets |
| `Content/UI/` | Cursor and UI foundation assets |

`Binaries/`, `DerivedDataCache/`, `Intermediate/`, `Saved/`, `.idea/`, `.vs/`, and generated solution/project files are local or generated output; do not edit or commit them.

## Naming

### Assets

Use the established Unreal-style prefixes and preserve nearby naming families:

| Prefix | Asset type | Example |
| --- | --- | --- |
| `BP_` | Blueprint | `BP_NotoGameMode` |
| `W_` | Widget Blueprint | `W_Cursor` |
| `DA_` | Data Asset | `DA_DefaultExperience` |
| `IA_` | Input Action | `IA_Move` |
| `IMC_` | Input Mapping Context | `IMC_Player_Default` |
| `T_` | Texture | `T_Cursor_Crosshair` |
| `M_` / `MI_` | Material / Material Instance | `M_Basic_Wall` |
| `SM_` / `SK_` | Static / Skeletal Mesh | `SM_AssetPlatform` |

Use `W_` consistently for Widget Blueprints in this repository. Introduce a new prefix only when a real asset family needs one.

### C++

- Follow Unreal type prefixes (`A`, `U`, `F`, `E`, `I`, `T`) and prefix booleans with `b`.
- Use PascalCase for types, functions, members, parameters, and local variables, consistent with Unreal style and nearby code.
- Preserve `Noto` for existing game-specific families. Use it for new public/exported main-game types when it improves consistency or collision resistance; it is not required on every symbol.
- Reusable plugin types should use their plugin or module domain rather than `Noto`. Private helpers need no project prefix when scope is clear.
- Do not mass-rename existing symbols to normalize style.

For coding-agent guidance and architectural boundaries, see `AGENTS.md` and `.agents/documents/`.
