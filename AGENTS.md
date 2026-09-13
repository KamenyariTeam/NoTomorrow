# No Tomorrow agent instructions

## Scope

No Tomorrow is an early-stage, single-player-first top-down immersive sim built with Unreal Engine 5.8. The repository currently contains one project runtime module, `NoTomorrowGame`, and the local `ModularGameplayActors` runtime plugin. `NoTomorrowEditor` is a build target, not an editor module.

Treat features described only in prose as plans until code, config, assets, or live editor state confirms them.

## Sources of truth

Use this order when information conflicts:

1. C++, `.uproject`, module rules, plugin descriptors, config, and authored Unreal assets.
2. Relevant Unreal Editor state.
3. `.agents/documents/architecture.md` and `.agents/documents/cpp-style.md`.
4. `README.md`.

Update stale documentation when it is part of the requested change. Otherwise report the mismatch.

## Ownership boundaries

- Put reusable gameplay logic, lifecycle handling, engine integration, components, and editor extensions in C++.
- Use Blueprints and assets for concrete classes, composition, defaults, maps, presentation, cameras, input assets, effects, and tuning.
- Keep persistent player state on `ANotoPlayerState`; keep avatar-specific input and presentation on the possessed pawn or controller.
- Keep immutable authored item data in `UNotoItemDefinition` and mutable item state in inventory item instances.
- Treat equipment actors as presentation. Inventory and gameplay components own gameplay state.
- Keep local input and presentation separate from authoritative gameplay state so future co-op does not require an architectural rewrite.
- Use current, native Unreal Engine facilities and plugins when they provide a clear correctness, performance, workflow, or future-maintenance advantage. Prefer measured optimization and established engine patterns over speculative complexity or custom replacements.

Read `.agents/documents/architecture.md` before changing a system boundary. Read `.agents/documents/cpp-style.md` before significant C++ additions or refactors.

## Unreal assets

- Inspect relevant assets before changing them when their current structure matters.
- Use Unreal Editor or other Unreal-aware tooling for `.uasset` and `.umap` changes; never binary-patch them.
- Compile modified Blueprints, save intended assets, and verify important references/defaults when tooling supports it.
- Ask before destructive asset moves, renames, deletes, replacements, or reparenting.
- If tooling cannot perform a required asset edit reliably, report the missing operation instead of moving appropriate authored behavior into C++.

## Working rules

- Inspect nearby code and assets before introducing a pattern. Make the smallest coherent change.
- Preserve unrelated and pre-existing edits. Do not reformat or clean up files outside the task.
- Do not edit generated or local output: `Binaries/`, `DerivedDataCache/`, `Intermediate/`, `Saved/`, IDE metadata, generated solutions/project files, or plugin equivalents.
- Do not hardcode an Engine installation path. Use Rider's associated Engine or `UE_ROOT`.
- Keep runtime code independent of editor-only modules. Reusable plugins must not depend on `NoTomorrowGame`.
- Disable Tick unless behavior genuinely requires per-frame work.
- Prefer existing Unreal systems and project patterns over custom abstractions or new dependencies.

## Validation

Run checks proportional to the change:

- Text/source: inspect the final diff and run `git diff --check`.
- C++: build the affected target and run relevant automation tests. If editor is running, close it before building.
- Blueprints: compile modified Blueprints and inspect introduced warnings/errors.
- Assets/maps: verify references, defaults, and placements; use PIE when it materially increases confidence.

From PowerShell at the repository root:

```powershell
& "$env:UE_ROOT\Engine\Build\BatchFiles\Build.bat" NoTomorrowEditor Win64 Development -Project="$PWD\NoTomorrow.uproject" -WaitMutex -NoHotReloadFromIDE

& "$env:UE_ROOT\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "$PWD\NoTomorrow.uproject" -unattended -nop4 -NullRHI -nosplash -stdout -FullStdOutLogOutput '-ExecCmds=Automation RunTests NoTomorrow' '-TestExit=Automation Test Queue Empty'
```

Report what passed, failed, or was not run.
