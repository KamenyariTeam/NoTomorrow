# No Tomorrow architecture

This document records the current foundation and its intended boundaries.

## Confirmed architecture

### Modules and plugins

- `NoTomorrowGame` is the only project module. It owns game-specific framework classes, tagged Enhanced Input plumbing, cursor aiming, gameplay tags, and Gameplay Cameras integration.
- `NoTomorrowEditor.Target.cs` builds `NoTomorrowGame` for the editor; there is no project editor module until a concrete editor-only feature needs one.
- `ModularGameplayActors` supplies reusable modular actor, pawn, character, controller, GameMode, and GameState bases. The game module may depend on it; the plugin must not depend on the game.
- Common UI, Game Features, Modular Gameplay, Enhanced Input, Gameplay Tags, and Gameplay Cameras are enabled facilities. Availability is not a requirement to route every feature through them.

### Runtime composition

- `ANotoGameMode` selects `ANotoCharacter`, `ANotoPlayerController`, and `AModularGameStateBase`. Concrete Blueprint defaults may specialize the pawn and other presentation data.
- `ANotoCharacter::SetupPlayerInputComponent` hands input setup to `UNotoPlayerPawnComponent`.
- `UNotoPlayerPawnComponent` owns local movement bindings and cursor aiming. `UNotoInputConfig` maps semantic gameplay tags to authored input actions; the default mapping context is installed by Enhanced Input developer settings.
- Software cursors are configured through `UUserInterfaceSettings` rather than custom activation actions.
- Stock Unreal Engine, AssetManager, GameInstance, WorldSettings, and HUD behavior is used until game-specific behavior creates a real subclass requirement.

### Content ownership

- C++ owns engine integration, reusable lifecycle behavior, and stable seams.
- Blueprints and assets configure the concrete game mode, character, cameras, tagged input data, cursor, and maps.
- Game-specific behavior belongs in `NoTomorrowGame`. A plugin is appropriate only for genuinely reusable code with a clear independent boundary.
- Game Features are reserved for features that require independent activation/deactivation and feature-owned actions or content. They are not the default project-composition mechanism.

## Placement guidance

- Use actors, pawns, and controllers for world presence, possession, authority, and replicated identity semantics.
- Use actor components for behavior owned by an actor and sharing its lifecycle.
- Use world subsystems for one service per world and game-instance subsystems only for session state that must survive travel.
- Use local-player subsystems for per-local-player input, preferences, and presentation.
- Use data assets for authored immutable configuration; keep mutable runtime state elsewhere.
- Add editor-only code only when a concrete workflow requires it, then isolate it in an editor module or uncooked plugin module.

## Single-player first, co-op aware

- Optimize current features for single-player. Do not add replication, RPCs, online services, or networking abstractions without a requirement.
- Do not assume player index zero. Pass actor, controller, instigator, target, and interaction context explicitly.
- Separate local input and presentation from authoritative gameplay state.
- Do not store per-player state in global singletons or static mutable variables.
- Keep save-game state separate from transient runtime and presentation state.

## Future architecture rule

Use the smallest native Unreal facility that meets the current requirement. Add a subsystem, module, plugin, Game Feature, or other architectural layer only when a concrete caller needs its lifecycle or boundary. The planned GAS work is deliberately separate and must establish one explicit Ability System Component ownership model before abilities are built on top of it.
