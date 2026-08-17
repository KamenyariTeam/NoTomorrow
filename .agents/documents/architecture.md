# No Tomorrow architecture

This document records the current foundation and its intended boundaries.

## Confirmed architecture

### Modules and plugins

- `NoTomorrowGame` is the only project module. It owns game-specific framework classes, tagged Enhanced Input plumbing, cursor aiming, gameplay tags, and Gameplay Cameras integration.
- `NoTomorrowEditor.Target.cs` builds `NoTomorrowGame` for the editor; there is no project editor module until a concrete editor-only feature needs one.
- `ModularGameplayActors` supplies reusable modular actor, pawn, character, controller, GameMode, and GameState bases. The game module may depend on it; the plugin must not depend on the game.
- Common UI, Game Features, Modular Gameplay, Enhanced Input, Gameplay Tags, and Gameplay Cameras are enabled facilities. Availability is not a requirement to route every feature through them.

### Runtime composition

- `ANotoGameMode` selects `ANotoCharacter`, `ANotoPlayerController`, `ANotoPlayerState`, and `AModularGameStateBase`. Concrete Blueprint defaults may specialize the pawn and other presentation data.
- `ANotoCharacter::SetupPlayerInputComponent` hands input setup to `UNotoPlayerPawnComponent`.
- `UNotoPlayerPawnComponent` owns local movement bindings and computes aim from the active input method. `ANotoPlayerController` owns the per-player gameplay reticle, Common Input method switching, and the boundary used by weapons and UI to replace or hide the reticle. `UNotoInputConfig` maps semantic gameplay tags to authored input actions; the default mapping context is installed by Enhanced Input developer settings.
- `ANotoPlayerState` owns the player's single replicated Ability System Component. `ANotoCharacter` implements `IAbilitySystemInterface` as the current avatar and initializes actor info on server possession and client PlayerState replication.
- `UNotoHealthSet` supplies GAS health, max-health, and incoming-damage attributes. Damage is an instant set-by-caller Gameplay Effect whose context retains instigator, causer, source object, and hit result. `UNotoHealthComponent` binds an avatar to those attributes and exposes damage/death presentation events; zero health adds `State.Dead`. Player attributes remain on the PlayerState ASC, while the standalone combat dummy owns a local ASC for NPC/test use.
- `ANotoPlayerState` owns the UI-independent `UNotoInventoryComponent`. Primary data assets describe immutable items; save-friendly item-instance, equipment, and active-slot state hold mutable state. Detachable magazines are unique physical item instances: spare magazines live in the inventory, while an inserted magazine and its GUID live with its weapon. Magazine family tags determine compatibility and magazine definitions determine capacity. Internal-feed weapons keep their loaded count on the weapon and consume matching stackable loose ammunition one round at a time. Reloading swaps the fullest compatible spare magazine as one mutation, preserves a removed partial magazine, and discards a removed empty magazine. Dropped weapons retain their inserted magazine state. After save deserialization, the explicit definition-resolution pass also converts legacy weapon `LoadedAmmo`/`ReserveAmmo` into one inserted magazine plus full and partial spare magazines without consolidating them later. World pickups use the normal interaction interface, and presentation observes inventory/equipment delegates through the controller convenience accessor.
- `UNotoFirearmComponent` performs one hitscan trace per accepted shot, applies GAS damage, and reports gunfire through AI Hearing. It has no tick; fire rate, damage, range, ammunition cost, trace height/channel, and noise range come from the active item definition. `UNotoPlayerPawnComponent` only owns the tagged fire/reload input bindings.
- `UNotoCheatManager` owns developer-only gameplay commands and their transient debug state. `ANotoPlayerController` selects it as the project cheat-manager class and only forwards PlayerState lifecycle changes needed for safe debug delegate rebinding.
- Gameplay reticles are viewport widgets independent from software cursors. Software cursors remain configured through `UUserInterfaceSettings` for menus and other pointer-driven UI.
- Stock Unreal Engine, AssetManager, GameInstance, WorldSettings, and HUD behavior is used until game-specific behavior creates a real subclass requirement.

### Content ownership

- C++ owns engine integration, reusable lifecycle behavior, and stable seams.
- Blueprints and assets configure the concrete game mode, character, cameras, tagged input data, cursor, and maps.
- Item definitions and placed pickups are authored assets. Weapon meshes, attacks, equipped presentation, sounds, icons, notes, and audio-log playback remain presentation or feature content rather than inventory-component responsibilities.
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

Use the smallest native Unreal facility that meets the current requirement. Add a subsystem, module, plugin, Game Feature, or other architectural layer only when a concrete caller needs its lifecycle or boundary. Add custom Ability System Component behavior, attribute sets, and abilities only when a gameplay feature needs them; the PlayerState remains their persistent owner.
