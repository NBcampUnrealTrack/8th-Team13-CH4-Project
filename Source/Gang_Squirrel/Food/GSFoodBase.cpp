// Fill out your copyright notice in the Description page of Project Settings.


#include "GSFoodBase.h"

#include "NaniteSceneProxy.h"
#include "SNegativeActionButton.h"
#include "Gang_Squirrel/DataAsset/GSFoodPrimaryDataAsset.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Gang_Squirrel/Food/GSFoodWidgetComponent.h"
#include "Gang_Squirrel/Character/GSCharacter.h"
#include "Gang_Squirrel/Food/GSFoodWidget.h"
#include "Misc/OutputDeviceNull.h"

// Sets default values
AGSFoodBase::AGSFoodBase()
{
	bReplicates = true;
	SetReplicateMovement(true);
	
	USceneComponent* DummyRoot = CreateDefaultSubobject<USceneComponent>("DefaultSceneRoot");
	RootComponent = DummyRoot;
	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("StaticMeshComponent");
	StaticMeshComponent->SetupAttachment(RootComponent); 
    
	FoodWidgetComponent = CreateDefaultSubobject<UGSFoodWidgetComponent>("FoodWidgetComponent");
	FoodWidgetComponent->SetupAttachment(RootComponent);
    
	SphereComponent = CreateDefaultSubobject<USphereComponent>("OverlapCollision");
	SphereComponent->SetupAttachment(RootComponent);
    
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AGSFoodBase::OnBeginOverlap);
	SphereComponent->OnComponentEndOverlap.AddDynamic(this, &AGSFoodBase::OnEndOverlap);
    
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	

}

void AGSFoodBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AGSFoodBase, bIsActive);
	DOREPLIFETIME(AGSFoodBase, FoodData);
}

// Called when the game starts or when spawned
void AGSFoodBase::BeginPlay()
{
	Super::BeginPlay();
	
	// Post Process Material
	if (StaticMeshComponent)
	{
		StaticMeshComponent->SetRenderCustomDepth(true);
		// Set StencilValue -> 1 
		StaticMeshComponent->SetCustomDepthStencilValue(1);
	}

	// UE_LOG(LogTemp, Warning, TEXT("[Before] WidgetClass = %s"), *GetNameSafe(FoodWidgetComponent->GetWidgetClass()));
    
	if (FoodWidgetComponent)
	{
		if (FoodWidgetComponent->GetWidgetClass() == nullptr)
		{
			FString WidgetPath = TEXT("/Game/YJW/Food/FoodUI/WBP_FoodWidget.WBP_FoodWidget_C"); 
            
			UClass* LoadedClass = StaticLoadClass(UUserWidget::StaticClass(), nullptr, *WidgetPath);
			if (LoadedClass)
			{
				FoodWidgetComponent->SetWidgetClass(LoadedClass);
			}
		}
		
		FoodWidgetComponent->InitWidget();
		
		if (SphereComponent)
		{
			SphereComponent->SetRelativeScale3D(FVector(5.3f, 5.3f, 5.3f));
			SphereComponent->InitSphereRadius(1.f);
		}
	}
	
	bIsFilling = false;
	
	if (HasAuthority()) Activate();
}

// Called every frame
void AGSFoodBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!IsValid(CurrentCharacter))
	{
		// UE_LOG(LogTemp, Warning, TEXT("REturn"))
		return;
	}
	
	if (bIsFilling && FoodWidgetComponent && CurrentCharacter->bIsEating && FoodData)
	{
		CurrentEatenTime += DeltaTime;
		
		if (bIsStartEating && CurrentCharacter->bIsEating)
		{
			StartWidgetAnime();
			bIsStartEating = false;
		}
		
		float FinalTime = FoodData->EatTime * 4;
		
		float Alpha = FMath::Clamp(CurrentEatenTime / FinalTime, 0.f, 1.f );
		
		UUserWidget* UserWidget = FoodWidgetComponent->GetUserWidgetObject();
		if (UserWidget)
		{
			FOutputDeviceNull ar;
			FString Cmd = FString::Printf(TEXT("UpdateProgress %f"), Alpha);
			UserWidget->CallFunctionByNameWithArguments(*Cmd, ar, nullptr, true);
		}
		
		if (Alpha >= 0.190f)
		{
			bIsFilling = false;
			SetActorTickEnabled(false);
			
			StopWidgetAnim();
			
			bIsFilling = false;
			
			APlayerController* PC = GetWorld()->GetFirstPlayerController();
			if (PC)
			{
				AGSCharacter* LocalChar = Cast<AGSCharacter>(PC->GetPawn());
				if (LocalChar)
				{
					LocalChar->Server_NotifyFoodEaten(this);
				}
			}
			Eaten();
			CurrentCharacter->AddTempScore(FoodData->ScoreAmount);
			
			// UE_LOG(LogTemp, Warning, TEXT("Food Eat Progress Complete!!"));
		}
	}
	else
	{
		StopWidgetAnim();
		bIsStartEating = true;
	}
}

void AGSFoodBase::Init(UGSFoodPrimaryDataAsset* InData)
{
	//UE_LOG(LogTemp, Warning, TEXT("Init GSFoodBase"));
	
	if (!IsValid(InData))
	{
		//UE_LOG(LogTemp, Error, TEXT("Init GSFoodBase failed"));
		return;
	}
	FoodData = InData;
	
	if (!IsValid(StaticMeshComponent)) return;
	
	OnRep_FoodData();
}

void AGSFoodBase::OnRep_Activate()
{
	this->SetActorHiddenInGame(!bIsActive);
	this->SetActorEnableCollision(bIsActive);
}

void AGSFoodBase::Activate()
{
	if (!HasAuthority()) return;
	bIsActive = true;
	OnRep_Activate();
}

void AGSFoodBase::Deactivate()
{
	if (!HasAuthority())
	{
		//UE_LOG(LogTemp, Error, TEXT("GSFoodBase deactivated failed"));
		return;
	}
	//UE_LOG(LogTemp, Warning, TEXT("Deactivate GSFoodBase"));
	SetActorTickEnabled(false);
	if (CurrentCharacter)
	{
		CurrentCharacter->InflateCheeks(FoodData->SquirrelScale);
	}
	bIsActive = false;
	OnRep_Activate();
}

void AGSFoodBase::OnRep_FoodData() const
{
	if (!IsValid(FoodData)) return;
	
	StaticMeshComponent->SetStaticMesh(FoodData->FoodMesh);
	
	if (StaticMeshComponent)
	{
		float CurrentMeshSize = FoodData->MeshSize;
		StaticMeshComponent->SetRelativeScale3D(FVector(CurrentMeshSize,CurrentMeshSize,CurrentMeshSize));
		SphereComponent->SetSphereRadius(5.f);

		// Set Overlay Material -> Name: M_Outline
		UStaticMeshComponent* MutableMeshComp = const_cast<UStaticMeshComponent*>(StaticMeshComponent.Get());
		if (MutableMeshComp)
		{
			UMaterialInterface* LoadedOverlayMat = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, TEXT("/Game/ExternalContent/LevelPrototyping/Materials/M_Outline.M_Outline")));
			if (LoadedOverlayMat)
			{
				StaticMeshComponent->SetOverlayMaterial(LoadedOverlayMat);
			}
		}
	}
}

int32 AGSFoodBase::Eaten()
{
	Deactivate();
	
	return FoodData->ScoreAmount;
}

void AGSFoodBase::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	CurrentCharacter  = Cast<AGSCharacter>(OtherActor);
	if (!CurrentCharacter)
	{
		return;
	}
	/*else
	{
		UE_LOG(LogTemp, Error, TEXT("%s"), *CurrentCharacter->GetName());
	}*/
	if (CurrentCharacter->IsLocallyControlled())
	{
		// UE_LOG(LogTemp, Warning, TEXT("Local Player Overlap Begin!"));
		
		bIsFilling = true;
		CurrentEatenTime = 0.f;
		
		SetActorTickEnabled(true);
		
		if (FoodWidgetComponent)
		{
			FoodWidgetComponent->SetVisibility(true);
			UUserWidget* UserWidget = FoodWidgetComponent->GetUserWidgetObject();
			if (UserWidget)
			{
				FOutputDeviceNull ar;
				UserWidget->CallFunctionByNameWithArguments(TEXT("UpdateProgress 0.0"), ar, nullptr, true);
			}
		}
	}
}

void AGSFoodBase::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AGSCharacter* Character = Cast<AGSCharacter>(OtherActor);
	if (!Character) return;
	
	if (Character->IsLocallyControlled())
	{
		// UE_LOG(LogTemp, Warning, TEXT("Local Player Overlap End!"));
		
		bIsFilling = false;
		CurrentEatenTime = 0.f;
		
		SetActorTickEnabled(false);
		
		if (FoodWidgetComponent)
		{
			FoodWidgetComponent->SetVisibility(false);
		}
	}
}

void AGSFoodBase::SetUIVisible(bool bShow)
{
	if (FoodWidgetComponent)
	{
		// UE_LOG(LogTemp, Warning, TEXT("SetUI"));
		FoodWidgetComponent->SetVisibility(bShow);
	}
	
}

void AGSFoodBase::StartWidgetAnime()
{
	if  (FoodWidgetComponent)
	{
		UUserWidget* UserWidget = FoodWidgetComponent->GetUserWidgetObject();
		if (UserWidget)
		{
			UGSFoodWidget* CurrentFoodWidget =  Cast<UGSFoodWidget>(UserWidget);
			if (CurrentFoodWidget)
			{
				CurrentFoodWidget->UpdateWidget();
			}
		}
	}
}

void AGSFoodBase::StopWidgetAnim()
{
	if  (FoodWidgetComponent)
	{
		UUserWidget* UserWidget = FoodWidgetComponent->GetUserWidgetObject();
		if (UserWidget)
		{
			UGSFoodWidget* CurrentFoodWidget =  Cast<UGSFoodWidget>(UserWidget);
			if (CurrentFoodWidget)
			{
				CurrentFoodWidget->StopWidget();
			}
		}
	}
}

