# No Tomorrow architecture

This document records the current repository, not a redesign. "Confirmed" means visible in source, descriptors, config, or tracked assets; "inferred" means a convention consistently suggested by those sources.

## Confirmed current architecture

### Project modules

- `NoTomorrowGame` (runtime): game engine/asset manager/game instance classes; modular GameMode, GameState, PlayerController, Character, HUD, and WorldSettings types; Enhanced Input data and component plumbing; gameplay tags/logging/settings; Game Feature actions for input mappings and software cursors; gameplay-camera integration.
- `NoTomorrowEditor` (editor): the custom `UNotoEditorEngine`, common-map toolbar and style, PIE setup, and editor validation/tooling dependencies. Keep editor APIs here.

NoTomorrowGame.Target.cs builds the runtime game module. NoTomorrowEditor.Target.cs builds both NoTomorrowGame and NoTomorrowEditor. Runtime and Game targets must not include editor-only modules.

### Project plugins

- `GameplayCore` contains `GameplayExperience` and `GameplayEvents` runtime modules plus the uncooked-only `GameplayEventNodes` module. Experiences are primary data assets loaded through a game-instance subsystem and a GameState component; they can activate Game Feature actions. Gameplay events are gameplay-tag channels with immediate or end-of-frame delivery and Blueprint async support. Both runtime areas contain automation tests.
- `ModularGameplayActors` provides modular base classes for actors, pawns, characters, controllers, GameMode/GameState, HUD, and framework components. It integrates with Epic's Modular Gameplay component model and is intended as reusable infrastructure.

The `.uproject` explicitly enables Common UI, Game Features, Modular Gameplay, Modular Gameplay Actors, and editor-only Modeling Tools. Enhanced Input and Gameplay Tags are direct module/plugin dependencies. Disabled media plugins in the descriptor are not architectural building blocks.

### Code and content ownership

- C++ supplies engine-facing foundations, lifecycles, loading, input binding, reusable components, and editor extensions.
- Config selects the custom Engine, EditorEngine, AssetManager, WorldSettings, Common UI viewport, Blueprint GameInstance, and Blueprint GameMode. It also registers GameplayExperience/GameFeatureData primary assets and input gameplay tags.
- Tracked assets provide Blueprint subclasses, a default character, Enhanced Input actions/mapping/config, gameplay-camera assets, the default experience, cursor widgets/textures, and the current map. Do not assume the README's broader game features are implemented.
- Game-specific behavior belongs in `NoTomorrowGame`. Code belongs in a plugin only when it is genuinely reusable, has a clear module boundary, and does not depend on game-specific types or content.

## Lifecycle and placement guidance

- **Actors/Pawns/Controllers:** use for world presence, possession, authority, and replicated identity semantics. Pass the relevant actor/controller/instigator explicitly.
- **Actor components:** use for behavior owned by an actor and sharing its world/replication lifecycle. The current project uses modular framework components for runtime injection.
- **World subsystems:** use for one service per world when the state must not survive travel and does not belong to a specific actor.
- **Game-instance subsystems:** use for process/session services that survive map travel. Do not use them as a convenient home for per-player or world-specific state; current experience and event systems are deliberate examples.
- **Local-player subsystems:** use for per-local-player input, preferences, or presentation. Enhanced Input mapping is already handled through local players.
- **Data assets/primary assets:** use for authored configuration and loadable definitions. Keep mutable runtime state elsewhere.
- **Editor-only code:** keep it in `NoTomorrowEditor` or an editor/uncooked plugin module, guarded by `WITH_EDITOR` only when a runtime type needs a small editor hook.

Do not casually reverse the existing dependency direction (`NoTomorrowGame` -> reusable plugins), make plugins depend on the game, move local-player input into global services, couple reusable gameplay logic to HUD/widgets, or introduce cross-feature dependencies when an explicit interface/event/data asset is sufficient.

## Single-player first, co-op aware

Current scope is single-player. Future co-op is possible but not committed: preserve multiplayer awareness without implementing multiplayer prematurely.

- Optimize current features for the single-player game. Do not add replication, RPCs, online subsystems, or networking abstractions without a feature requirement.
- Do not assume the relevant player is always player index zero. Pass actor, controller, instigator, target, and interaction context explicitly.
- Separate local input and presentation state from authoritative gameplay state. Keep reusable gameplay logic independent from local UI where reasonable.
- Do not store per-player state in global singletons or static mutable variables.
- Choose GameMode, GameState, PlayerState, PlayerController, Pawn/components, and subsystems according to Unreal lifecycle and ownership semantics. Make ownership and authority assumptions visible at important boundaries.
- Keep save-game state distinct from transient runtime and presentation state.

## Inferred conventions and unimplemented possibilities

The existing module split, `Noto` game-type family, plugin-domain naming, data-driven experiences, and modular components suggest extension through focused C++ foundations plus configured assets. Preserve those conventions when they fit; inspect the call sites before extending them.

Co-op, online services, production save systems, and the README's heist/anomaly/story ambitions are future possibilities, not confirmed implemented systems. Do not add architecture for them until a concrete feature needs it.

## Engineering judgment and modern UE facilities

Use the simplest existing Unreal facility that clearly fits the current requirement. While implementing, actively consider whether an existing project system would produce a cleaner boundary, lower coupling, or better lifecycle ownership.

- Use a Game Feature when a feature needs independent activation/deactivation, feature-owned actions or content, and a clear optional boundary. Do not create one for ordinary code organization.
- Use GameplayExperience for data-driven composition and loading of a playable experience; do not use it as a generic container for unrelated features.
- Use Modular Gameplay and the Game Framework Component Manager when behavior must be injected into framework actors without hard-coupling game classes.
- Keep Enhanced Input per local player and use Common UI for UI-layer conventions already present in the project.
- Consider MVVM when a UI feature has meaningful state binding that would otherwise become repeated widget glue. Do not add its plugin or a new UI architecture without a concrete need and approval.
- Prefer an existing project pattern over introducing a new subsystem, plugin, or framework.

When a materially better approach is visible but exceeds the task scope, state a concise `Recommendation` in the final response: the observed issue, proposed approach, why it fits this project, and the smallest follow-up. Do not implement a significant architectural change without approval.
