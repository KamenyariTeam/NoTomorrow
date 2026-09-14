# No Tomorrow architecture

No Tomorrow is single-player first. The architecture keeps authoritative state separate from local input and presentation so co-op can be added deliberately later, without paying its full complexity now.

## Modules and content

- `NoTomorrowGame` is the only project module and owns game-specific runtime code.
- `NoTomorrowEditor` is an editor build target; there is no project editor module yet.
- `ModularGameplayActors` provides Game Feature-compatible actor bases and must remain independent of the game module.
- C++ owns reusable behavior and lifecycle. Blueprints and assets own concrete composition, configuration, presentation, maps, and tuning.
- Game Features are available for independently activated features, not ordinary project composition.

## Runtime ownership

| Owner | Responsibility |
| --- | --- |
| `ANotoPlayerState` | Persistent Ability System Component and inventory |
| `ANotoCharacter` | Current ASC avatar and character-owned gameplay components |
| `UNotoPlayerPawnComponent` | Local input, movement state, aim, and input routing |
| `ANotoPlayerController` | Local input-method state and gameplay reticle |
| `UNotoInventoryComponent` | Mutable item instances, equipment slots, active slot, and ammunition state |
| `UNotoItemDefinition` | Immutable authored item and presentation data |
| `UNotoFirearmComponent` | Active-weapon firing, per-item cooldowns, damage, reload, and gunfire noise |
| `UNotoEquippedItemComponent` | Presentation rebuilt from the active inventory item |
| `UNotoHealthSet` / `UNotoHealthComponent` | Replicated health state / avatar-facing damage and death events |

## Lifecycle boundaries

- The PlayerState owns the ASC; the possessed character is its avatar.
- Possession and `OnRep_PlayerState` initialize avatar bindings. Unpossession and end play remove them.
- Inventory changes are event-driven. Definition resolution refreshes both inventory observers and equipped presentation.
- Equipped actors are transient, non-replicated presentation objects reconstructed from inventory state.
- Primary asset IDs and item GUIDs are the stable save-facing identities; resolved UObject pointers are transient.

## Networking status

- Health attributes and the dead tag have replication support on the PlayerState ASC.
- Inventory mutation, interaction, weapon firing, and equipment state are currently standalone-only and are not server-authoritative co-op implementations.
- When co-op becomes a requirement, add authoritative requests and replicated inventory/equipment state at these existing ownership boundaries. Do not duplicate state on the pawn or presentation actors.
- Never assume player index zero or store per-player gameplay state in global mutable objects.

## Growth rules

- Add an editor module only for real editor-only C++.
- Add a subsystem only when its world, game-instance, or local-player lifetime is required.
- Split item definitions into fragments only when multiple item families create meaningful optional behavior, not preemptively.
- Add Lyra-style init-state coordination when asynchronously injected Game Feature components require it.
- Prefer the smallest native Unreal facility that satisfies the current feature.
