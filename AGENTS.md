# No Tomorrow agent guide

## Project summary

No Tomorrow is an early-stage, single-player-first top-down immersive-sim project for Unreal Engine 5.8. `NoTomorrowGame` is the runtime module and `NoTomorrowEditor` contains editor-only extensions. The project uses Enhanced Input, Common UI, Gameplay Tags, Game Features, Modular Gameplay, the local `GameplayCore` plugin (gameplay experiences and tagged events), the local `ModularGameplayActors` plugin, and `unreal-mcp` for direct interaction with the Unreal Editor when editor-authored content needs to be inspected or changed.

C++ is the preferred home for reusable gameplay logic, runtime foundations, engine integrations, modular actor bases, input plumbing, asset/experience loading, and editor extensions. Blueprints and assets should handle concrete game classes, composition, per-asset configuration, tuning, presentation, maps, and other content-authoring tasks where Unreal's editor workflow is the better fit.

Do not choose Blueprint merely because Unreal MCP can edit it, and do not choose C++ merely to avoid an editor-authored change. Put behavior in the layer that best matches its ownership, reuse, complexity, and authoring needs.

Treat planned game concepts in prose as plans unless code, assets, Blueprint graphs, project settings, or editor state confirm them.

## Sources of truth

Use this priority when facts disagree:

1. Code, `.uproject`, module rules, plugin descriptors, config, and authored Unreal assets/Blueprints.
2. Relevant Unreal Editor state when direct inspection is needed.
3. Project agent documentation in this file and `.agents/documents/`.
4. `README.md` and other human-facing documentation.
5. Assumptions.

Verify behavior in the source that owns it. Inspect C++ for code-owned systems and inspect the relevant Blueprint or asset for content-authored behavior. When stale documentation is within the task's scope, update it; otherwise call out the mismatch.

## Choosing C++ vs Blueprint/assets

Use C++ by default when the behavior is part of the project's reusable gameplay or technical foundation. In particular, prefer C++ for:

- reusable gameplay logic and APIs;
- base classes, components, subsystems, and shared systems;
- engine or plugin integration;
- logic that benefits from strong typing, code review, testing, or broad reuse;
- performance-sensitive or structurally complex runtime behavior;
- editor extensions and tooling implemented as code.

Use Blueprints/assets when Unreal's authored-content workflow provides a real advantage. In particular, prefer them for:

- concrete subclasses and content-specific assembly;
- component composition and per-class defaults;
- Data Assets and other data-driven configuration;
- input/camera/content wiring;
- maps, placed actors, experiences, cursors, presentation, VFX/audio hooks, and tuning;
- simple game-specific orchestration that is clearer and faster to author visually than as reusable C++.

When both layers are appropriate, keep the reusable contract and core behavior in C++, expose only the API needed by content, and perform the concrete wiring/configuration in Blueprint or assets.

Avoid two opposite failure modes:

- Do not add unnecessary C++ plumbing just to avoid making an appropriate Blueprint or asset edit.
- Do not move reusable or architectural logic into Blueprint just because direct editor access makes that possible.

Follow nearby project patterns when the correct ownership is not obvious.

## Unreal Editor access

The agent can use Unreal MCP to inspect and modify supported editor-authored content directly. Treat it as tooling for carrying out the chosen architecture, not as a reason to prefer editor-authored solutions.

- Inspect relevant Blueprints/assets before changing them when their current structure matters.
- The agent may create or modify supported Blueprints, defaults, components, graphs, Data Assets, input assets, maps, actors, settings, and other editor-authored content when the requested change is clear.
- Keep Blueprint graph changes small, readable, and consistent with existing conventions. Reuse existing functions/macros and avoid unrelated graph cleanup.
- Save intended asset changes and compile modified Blueprints when that validation is available.
- Do not directly binary-patch `.uasset` or `.umap` files. Use Unreal Editor or another Unreal-aware tool.
- If the available editor tooling cannot perform a required authored change reliably, state the exact missing operation rather than changing the architecture solely to work around the tooling limitation.
- Be deliberate with destructive asset operations such as deleting, renaming, moving, replacing, or reparenting because they can affect references.

## Working rules

- Inspect nearby code and relevant assets before introducing a pattern. Make the smallest coherent, reviewable change that solves the task.
- Preserve unrelated edits. Avoid broad cleanup, mass renaming, whole-file reformatting, bulk asset moves, and speculative abstractions.
- Explain assumptions that cannot be verified. Ask before adding a significant dependency, plugin, module, or architectural layer.
- Do not edit generated Unreal files or local output: `Binaries/`, `DerivedDataCache/`, `Intermediate/`, `Saved/`, IDE metadata, solution/project files generated by Unreal, or plugin equivalents.
- Update documentation when a public workflow or architectural boundary changes.
- Review the final source diff and the set of modified Unreal assets before finishing.
- Before significant C++ additions or refactors, read `.agents/documents/cpp-style.md`.
- Before adding a subsystem, module, plugin, replicated system, or cross-feature dependency, read `.agents/documents/architecture.md`.
- The two supporting documents are not required for trivial Markdown-only edits or straightforward content configuration that does not alter architecture.

## Validation

Choose validation proportional to the change:

- Inspect the final source diff and run `git diff --check` for text/source changes.
- Parse or otherwise validate modified JSON, INI, TOML, Markdown, and scripts where tooling exists.
- Compile the affected Unreal module or the `NoTomorrowEditor` target when C++ changed and the local Engine/toolchain are available.
- Compile modified Blueprints when supported and check for errors or warnings introduced by the change.
- For modified assets or maps, verify important references/defaults/placements and save the assets.
- Run relevant Unreal automation tests when they exist.
- Use PIE or another in-editor check when it materially increases confidence in gameplay behavior.
- Report exactly what was changed, what validation was run, what passed or failed, and what was not validated.

Never hardcode a developer's Engine installation path. Use Rider's configured Unreal Engine, the `.uproject` association, or a documented variable such as `UE_ROOT`.
