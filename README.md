# No Tomorrow

No Tomorrow is an early-stage top-down immersive sim about planning and executing heists in a collapsing world.

The current project is a gameplay foundation rather than a complete game. It includes character control and cursor aiming, interaction, inventory and physical ammunition, equipped-item presentation, hitscan combat, GAS-based health and damage, Common UI integration, and Gameplay Cameras.

## Requirements

- Unreal Engine 5.8 using the Engine associated with `NoTomorrow.uproject`
- JetBrains Rider with Unreal Engine support
- A compatible Windows C++ toolchain
- Git LFS

## Quick start

1. Clone the repository.
2. Install Git LFS and fetch binary assets:

   ```powershell
   git lfs install
   git lfs pull
   ```

3. Open `NoTomorrow.uproject` in Rider.
4. Select `NoTomorrowEditor`, `Development Editor`, and `Win64`.
5. Build and run the editor, then use Play In Editor.

Generated solutions and project files are local output and should not be committed.

## Command-line build and tests

Set `UE_ROOT` to the associated Engine root, then run:

```powershell
& "$env:UE_ROOT\Engine\Build\BatchFiles\Build.bat" NoTomorrowEditor Win64 Development -Project="$PWD\NoTomorrow.uproject" -WaitMutex -NoHotReloadFromIDE

& "$env:UE_ROOT\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "$PWD\NoTomorrow.uproject" -unattended -nop4 -NullRHI -nosplash -stdout -FullStdOutLogOutput '-ExecCmds=Automation RunTests NoTomorrow' '-TestExit=Automation Test Queue Empty'
```

## Contributing

- Keep reusable gameplay and engine integration in C++; use Blueprints and assets for concrete content, composition, presentation, and tuning.
- Preserve unrelated work and keep changes focused.
- Do not commit generated Unreal or IDE output.
- Build affected C++ and run relevant automation tests before submitting changes.

See [AGENTS.md](AGENTS.md) for repository instructions used by coding agents and [.agents/documents/architecture.md](.agents/documents/architecture.md) for current system ownership.
