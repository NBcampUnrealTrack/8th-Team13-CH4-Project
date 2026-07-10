#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "GS_FallingHazardDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FGSFallingHazardVisualData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName Name = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMesh> Mesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector MeshRelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRotator MeshRelativeRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector MeshRelativeScale = FVector(1.f, 1.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector DamageBoxExtent = FVector(45.f, 45.f, 20.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector DamageBoxRelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float GroundCheckDistanceOverride = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shadow")
	TObjectPtr<UMaterialInterface> ShadowDecalMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shadow")
	FVector2D ShadowDecalLocationOffset = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shadow")
	float ShadowDecalRotationYaw = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shadow")
	FVector2D ShadowDecalSizeRatio = FVector2D(1.f, 1.f);

};

UCLASS(BlueprintType)
class GANG_SQUIRREL_API UGS_FallingHazardDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FGSFallingHazardVisualData> HazardVisualDatas;
};