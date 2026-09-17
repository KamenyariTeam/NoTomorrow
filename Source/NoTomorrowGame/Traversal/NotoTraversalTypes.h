// © 2025 Kamenyari. All rights reserved.

#pragma once

#include "Animation/NotoLocomotionTypes.h"
#include "CoreMinimal.h"
#include "PoseSearch/PoseSearchHistory.h"
#include "NotoTraversalTypes.generated.h"

class UAnimMontage;
class UPrimitiveComponent;

UENUM(BlueprintType)
enum class ENotoTraversalActionType : uint8
{
	None,
	Hurdle,
	Vault,
	Mantle
};

USTRUCT(BlueprintType)
struct NOTOMORROWGAME_API FNotoTraversalCheckInputs
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	FVector TraceForwardDirection = FVector::ForwardVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal", Meta = (ClampMin = "0.0", Units = "cm"))
	float TraceForwardDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	FVector TraceOriginOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	FVector TraceEndOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal", Meta = (ClampMin = "0.0", Units = "cm"))
	float TraceRadius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal", Meta = (ClampMin = "0.0", Units = "cm"))
	float TraceHalfHeight = 0.0f;
};

USTRUCT(BlueprintType)
struct NOTOMORROWGAME_API FNotoTraversalLedgeData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	bool bHasFrontLedge = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	FTransform FrontLedgeTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	bool bHasBackLedge = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	FTransform BackLedgeTransform = FTransform::Identity;
};

USTRUCT(BlueprintType)
struct NOTOMORROWGAME_API FNotoTraversalCheckResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	ENotoTraversalActionType ActionType = ENotoTraversalActionType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	bool bHasFrontLedge = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	FVector FrontLedgeLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	FVector FrontLedgeNormal = FVector::UpVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	bool bHasBackLedge = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	FVector BackLedgeLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	FVector BackLedgeNormal = FVector::UpVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	bool bHasBackFloor = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	FVector BackFloorLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	float ObstacleHeight = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	float ObstacleDepth = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	float BackLedgeHeight = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	TObjectPtr<UPrimitiveComponent> HitComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	TObjectPtr<UAnimMontage> ChosenMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal", Meta = (ClampMin = "0.0", Units = "s"))
	float StartTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal", Meta = (ClampMin = "0.01"))
	float PlayRate = 1.0f;
};

USTRUCT(BlueprintType)
struct NOTOMORROWGAME_API FNotoTraversalMontageChooserInput
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	ENotoTraversalActionType ActionType = ENotoTraversalActionType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	bool bHasFrontLedge = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	bool bHasBackLedge = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	bool bHasBackFloor = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	float ObstacleHeight = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	float ObstacleDepth = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	float BackLedgeHeight = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	float DistanceToLedge = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	ENotoLocomotionMovementMode MovementMode = ENotoLocomotionMovementMode::OnGround;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	ENotoLocomotionGait Gait = ENotoLocomotionGait::Run;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	float Speed = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal")
	FPoseHistoryReference PoseHistory;
};

USTRUCT(BlueprintType)
struct NOTOMORROWGAME_API FNotoTraversalMontageChooserOutput
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	ENotoTraversalActionType ActionType = ENotoTraversalActionType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	TObjectPtr<UAnimMontage> Montage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal", Meta = (ClampMin = "0.0", Units = "s"))
	float MontageStartTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal", Meta = (ClampMin = "0.01"))
	float PlayRate = 1.0f;
};
