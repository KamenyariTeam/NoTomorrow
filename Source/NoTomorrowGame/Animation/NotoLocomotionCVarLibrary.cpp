// Copyright Epic Games, Inc. All Rights Reserved.

#include "Animation/NotoLocomotionCVarLibrary.h"

#include "HAL/IConsoleManager.h"

namespace NotoLocomotionCVars
{
	static TAutoConsoleVariable<float> OffsetRootTranslationRadius(TEXT("Noto.Locomotion.Tuning.OffsetRootTranslationRadius"), 0.0f, TEXT("Live override for the Offset Root Bone translation radius."));
	static TAutoConsoleVariable<int32> DrawShapes(TEXT("Noto.Locomotion.Debug.DrawShapes"), 0, TEXT("Draw locomotion debug shapes. 0: disabled, 1: enabled."), ECVF_Cheat);
	static TAutoConsoleVariable<int32> DrawStates(TEXT("Noto.Locomotion.Debug.DrawStates"), 0, TEXT("Draw locomotion state debug information. 0: disabled, 1: enabled."), ECVF_Cheat);
	static TAutoConsoleVariable<int32> DrawGraphs(TEXT("Noto.Locomotion.Debug.DrawGraphs"), 0, TEXT("Draw locomotion graph debug information. 0: disabled, 1: enabled."), ECVF_Cheat);
	static TAutoConsoleVariable<int32> PoseSearchDatabaseLod(TEXT("Noto.Locomotion.Debug.PoseSearchDatabaseLOD"), 0, TEXT("Override the Pose Search database LOD used for locomotion debugging. 0: default."), ECVF_Cheat);
	static TAutoConsoleVariable<int32> UseThreadSafeUpdate(TEXT("Noto.Locomotion.Threading.UseThreadSafeUpdate"), 0, TEXT("Run locomotion update logic through BlueprintThreadSafeUpdateAnimation. 0: game-thread update, 1: thread-safe update."));
	static TAutoConsoleVariable<int32> UseExperimentalStateMachine(TEXT("Noto.Locomotion.Experimental.UseStateMachine"), 0, TEXT("Use experimental locomotion state-machine paths. 0: Motion Matching paths, 1: experimental state machine."), ECVF_Cheat);
	static TAutoConsoleVariable<int32> LocomotionSetup(TEXT("Noto.Locomotion.Debug.LocomotionSetup"), 0, TEXT("Set the cached locomotion setup enum used by the GASP AnimBP."), ECVF_Cheat);
}

FNotoLocomotionCVarValues UNotoLocomotionCVarLibrary::GetLocomotionCVarValues()
{
	using namespace NotoLocomotionCVars;

	FNotoLocomotionCVarValues Values;
	Values.OffsetRootTranslationRadius = OffsetRootTranslationRadius.GetValueOnGameThread();
	static IConsoleVariable* OffsetRootBoneEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("a.animnode.offsetrootbone.enable"));
	Values.bOffsetRootBoneEnabled = OffsetRootBoneEnabled && OffsetRootBoneEnabled->GetInt() != 0;
	Values.PoseSearchDatabaseLod = PoseSearchDatabaseLod.GetValueOnGameThread();
	Values.bDrawShapes = DrawShapes.GetValueOnGameThread() != 0;
	Values.bDrawStates = DrawStates.GetValueOnGameThread() != 0;
	Values.bDrawGraphs = DrawGraphs.GetValueOnGameThread() != 0;
	Values.bUseThreadSafeUpdate = UseThreadSafeUpdate.GetValueOnGameThread() != 0;
	Values.bUseExperimentalStateMachine = UseExperimentalStateMachine.GetValueOnGameThread() != 0;
	Values.LocomotionSetup = LocomotionSetup.GetValueOnGameThread();
	return Values;
}
