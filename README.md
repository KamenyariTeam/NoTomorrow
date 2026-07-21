# No Tomorrow

No Tomorrow is an early-stage top-down immersive sim about planning and executing heists in a collapsing world.

The project currently concentrates on a small, durable gameplay foundation. Heists, anomalies, systemic interactions, branching narrative, and possible future co-op remain design goals rather than completed features.

## Current technical foundation

- `NoTomorrowGame`: the single project runtime module, containing the modular game framework classes, a PlayerState-owned Gameplay Ability System, semantic Enhanced Input bindings, cursor aiming, nearby cursor-selected interactions, gameplay tags, and Gameplay Cameras integration.
- `Plugins/ModularGameplayActors`: reusable modular actor, pawn, character, controller, GameMode, and GameState bases aligned with Epic's Modular Gameplay actors.
- Native Unreal configuration installs the default input mapping context and software cursor; concrete pawn, game-mode, input, camera, cursor, and map data lives in project assets.
- Common UI, Game Features, Modular Gameplay, Gameplay Abilities, Enhanced Input, Gameplay Tags, and Gameplay Cameras remain enabled engine integrations. Game Features are available for features that eventually need independent activation, not ordinary project composition.

C++ owns engine-facing foundations and lifecycle behavior. Blueprints and data assets configure concrete game classes and authored data.

## Interaction setup

`ANotoCharacter` owns the local interaction range. It selects an `INotoInteractable` inside that range by its screen distance to the gameplay cursor, marks it through `SetInteractionHighlighted`, and calls `Interact` only on that selected target. Derive doors, loot, pickups, and other world objects from `ANotoInteractableActor` (or implement `INotoInteractable`) and use the actor's query-only `InteractionVolume` for range detection.

To enable input, create `IA_Interact`, add it to `IMC_Player_Default`, and add it to `DA_InputConfig_Player` with `InputTag.Interact`. Place `ANotoTestPickup` in a level to verify the path; it writes custom depth and destroys itself when interacted with. A post-process material that renders Custom Depth as a white outline is required for the visible outline.

## Requirements

- Unreal Engine 5.8, using the Engine build associated with `NoTomorrow.uproject`.
- JetBrains Rider with Unreal Engine support (primary IDE workflow).
- A compatible Windows C++ toolchain: MSVC, Windows SDK, and Unreal's required C++ components.
- Git LFS for binary Unreal and source-art files.

## Open and build with Rider

1. Clone the repository and run `git lfs install` and `git lfs pull`.
2. Open `NoTomorrow.uproject` in Rider. Select or register the associated Unreal Engine 5.8 build when prompted.
3. Let Rider index the project, then select `NoTomorrowEditor`, `Development Editor`, and `Win64`.
4. Build and run the Editor. Use Play In Editor for normal iteration.

Generated solution and project files are local output. Regenerate them through Rider or Unreal when required.

### Command-line build

Set `UE_ROOT` to the Engine root that owns `Engine/Build/BatchFiles/Build.bat`, then run from the repository root:

```powershell
& "$env:UE_ROOT\Engine\Build\BatchFiles\Build.bat" NoTomorrowEditor Win64 Development -Project="$PWD\NoTomorrow.uproject" -WaitMutex
```

Do not copy another developer's absolute Engine path. Rider's Engine association or the local `.uproject` registration is authoritative.

## Important folders

| Path | Purpose |
| --- | --- |
| `Source/NoTomorrowGame/` | Runtime character, input, player, game-mode, and shared development foundations |
| `Plugins/ModularGameplayActors/` | Reusable Modular Gameplay actor bases |
| `Config/` | Engine, game, input, gameplay-tag, and asset-manager configuration |
| `Content/Characters/` | Character Blueprints and Gameplay Cameras assets |
| `Content/Input/` | Enhanced Input actions, mapping context, and tagged input config |
| `Content/Maps/` | Playable and development maps |
| `Content/UI/` | Cursor and UI assets |

`Binaries/`, `DerivedDataCache/`, `Intermediate/`, `Saved/`, `.idea/`, `.vs/`, and generated solution/project files are local output; do not edit or commit them.

## Naming

Use Unreal-style asset prefixes such as `BP_`, `W_`, `DA_`, `IA_`, `IMC_`, `T_`, `M_`, `MI_`, `SM_`, and `SK_`. C++ follows Unreal type prefixes and keeps `Noto` for game-specific public type families. Reusable plugin types use their plugin domain instead.

For coding-agent guidance and architectural boundaries, see `AGENTS.md` and `.agents/documents/`.
