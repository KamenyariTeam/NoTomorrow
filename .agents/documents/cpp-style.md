# No Tomorrow C++ style cheat sheet

Use Unreal Engine conventions and the surrounding project code as the baseline. This guide is for new or materially changed code, not mechanical normalization.

## Naming

- Use Unreal type prefixes: `A`, `U`, `F`, `E`, `I`, and `T`; prefix booleans with `b`.
- Use PascalCase for types, functions, members, parameters, and local variables, following Unreal conventions.
- Prefer descriptive names over unclear abbreviations, and preserve established naming families.
- Do not rename existing symbols merely to normalize style.

### Project prefixes

- Preserve `Noto` on existing game-specific type families.
- For new public or exported types in `NoTomorrowGame`, use `Noto` when it improves consistency and collision resistance.
- For reusable plugin types, prefer the plugin or module domain (`GameplayExperience`, `GameplayEvent`, `Modular`, and similar) rather than `Noto`.
- Private implementation helpers need no project prefix when their scope is already clear.
- Do not mass-rename code to enforce this policy.

## Initialization

- Use constructor initializer lists for constructor initialization.
- Use clear, idiomatic in-class defaults such as `= nullptr`, `= false`, or an explicit enum value.
- Use `{}` for aggregates, value initialization, and cases where preventing narrowing is useful.
- Use parentheses when a constructor or API call is clearer that way. Remember that braces can select `std::initializer_list` overloads.
- Follow the surrounding file unless correctness requires a different form. Do not mechanically rewrite initialization syntax.

## Unreal objects and lifetimes

- Use UPROPERTY-backed TObjectPtr for persistent reflected strong references held by UObjects, especially when serialization, editor exposure, or garbage-collection tracking is required.
- Use `TWeakObjectPtr` for non-owning references that must tolerate destruction. Use soft object/class references for assets that should not be loaded or hard-referenced until needed.
- Put UObject references that must participate in reflection, serialization, replication, or garbage collection behind the appropriate `UPROPERTY`; a raw pointer is acceptable only when its non-owning, short-lived semantics are clear.
- Use `IsValid` when pending destruction or lifetime uncertainty matters; a null check is sufficient when only nullability matters.
- Do not capture raw UObjects unsafely in delayed delegates, timers, or asynchronous callbacks. Prefer UObject-bound delegates, weak pointers, or an explicit lifetime owner.
- Use Unreal `Cast`, `CastChecked`, and interface helpers for UObject types. Use normal C++ casts for native types, choosing the narrowest safe cast.
- Expose APIs and properties to Blueprints intentionally. Do not add `BlueprintCallable`, editable properties, or broad categories by default.

## General quality

- Apply const correctness to functions, references, and pointers where it communicates intent.
- Prefer forward declarations in headers when valid; include the defining header in the `.cpp`. Include what a public header actually needs.
- Respect module direction: reusable plugins must not depend on the game module, and runtime code must not depend on editor-only modules.
- Disable Tick by default. Add it only for genuinely per-frame work, and consider events, timers, or state changes first.
- Avoid hidden global mutable state and static mutable gameplay state.
- Prefer Unreal containers, smart pointers, delegates, logging, and async facilities when they integrate with reflection, GC, serialization, or engine tooling; native C++ facilities remain appropriate for isolated native logic.
- Comments should explain intent, lifecycle constraints, ownership, or non-obvious tradeoffs instead of restating code.
- Use `check` for programmer errors that make continuation invalid, `ensure` for recoverable invariant violations worth reporting, and logging for operational or diagnostic information. Use the nearest established log category.
- Remove dead and commented-out code. Do not introduce an abstraction without a current caller or use case.
