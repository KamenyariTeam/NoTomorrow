#include "K2Node_AsyncAction_WaitGameplayEvent.h"

#include "AsyncAction_WaitGameplayEvent.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintFunctionNodeSpawner.h"
#include "K2Node_AssignmentStatement.h"
#include "K2Node_CallFunction.h"
#include "K2Node_TemporaryVariable.h"
#include "KismetCompiler.h"

#define LOCTEXT_NAMESPACE "K2Node"

namespace AsyncAction_WaitGameplayEvent
{
	static FName EventPinName = "Event";
	static FName EventTypePinName = "EventType";
	static FName DelegateProxyPinName = "ProxyObject";
	static FName OutputChannelPinName = "ActualChannel";
}


void UK2Node_AsyncAction_WaitGameplayEvent::PostReconstructNode()
{
	Super::PostReconstructNode();

	RefreshOutputEventType();
}

void UK2Node_AsyncAction_WaitGameplayEvent::PinDefaultValueChanged(UEdGraphPin* ChangedPin)
{
	if (ChangedPin == GetEventTypePin())
	{
		if (ChangedPin->LinkedTo.Num() == 0)
		{
			RefreshOutputEventType();
		}
	}
}

void UK2Node_AsyncAction_WaitGameplayEvent::GetPinHoverText(const UEdGraphPin& Pin, FString& HoverTextOut) const
{
	Super::GetPinHoverText(Pin, HoverTextOut);
	
	if (Pin.PinName == AsyncAction_WaitGameplayEvent::EventPinName)
	{
		HoverTextOut = HoverTextOut + LOCTEXT("EventToolTip", "\n\nThe received event structure").ToString();
	}
}

void UK2Node_AsyncAction_WaitGameplayEvent::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	struct GetMenuActions_Utils
	{
		static void SetNodeFunc(UEdGraphNode* NewNode, bool /*bIsTemplateNode*/, TWeakObjectPtr<UFunction> FunctionPtr)
		{
			UK2Node_AsyncAction_WaitGameplayEvent* AsyncTaskNode = CastChecked<UK2Node_AsyncAction_WaitGameplayEvent>(NewNode);
			if (FunctionPtr.IsValid())
			{
				UFunction* Func = FunctionPtr.Get();
				FObjectProperty* ReturnProp = CastFieldChecked<FObjectProperty>(Func->GetReturnProperty());
						
				AsyncTaskNode->ProxyFactoryFunctionName = Func->GetFName();
				AsyncTaskNode->ProxyFactoryClass        = Func->GetOuterUClass();
				AsyncTaskNode->ProxyClass               = ReturnProp->PropertyClass;
			}
		}
	};

	UClass* NodeClass = GetClass();
	ActionRegistrar.RegisterClassFactoryActions<UAsyncAction_WaitGameplayEvent>(FBlueprintActionDatabaseRegistrar::FMakeFuncSpawnerDelegate::CreateLambda([NodeClass](const UFunction* FactoryFunc)->UBlueprintNodeSpawner*
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintFunctionNodeSpawner::Create(FactoryFunc);
		check(NodeSpawner != nullptr);
		NodeSpawner->NodeClass = NodeClass;

		TWeakObjectPtr<UFunction> FunctionPtr = MakeWeakObjectPtr(const_cast<UFunction*>(FactoryFunc));
		NodeSpawner->CustomizeNodeDelegate = UBlueprintNodeSpawner::FCustomizeNodeDelegate::CreateStatic(GetMenuActions_Utils::SetNodeFunc, FunctionPtr);

		return NodeSpawner;
	}) );
}

void UK2Node_AsyncAction_WaitGameplayEvent::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();

	// The output of the UK2Node_AsyncAction_WaitGameplayEvent delegates is a proxy object which is used in the follow up call of GetEvent when triggered
	// This is only needed in the internals of this node so hide the pin from the editor.
	UEdGraphPin* DelegateProxyPin = FindPin(AsyncAction_WaitGameplayEvent::DelegateProxyPinName);
	if (ensure(DelegateProxyPin))
	{
		DelegateProxyPin->bHidden = true;
	}

	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Wildcard, AsyncAction_WaitGameplayEvent::EventPinName);
}

bool UK2Node_AsyncAction_WaitGameplayEvent::HandleDelegates(const TArray<FBaseAsyncTaskHelper::FOutputPinAndLocalVariable>& VariableOutputs, UEdGraphPin* ProxyObjectPin, UEdGraphPin*& InOutLastThenPin, UEdGraph* SourceGraph, FKismetCompilerContext& CompilerContext)
{
	bool bIsErrorFree = true;

	if (VariableOutputs.Num() != 3)
	{
		ensureMsgf(false, TEXT("%s - Variable output array not valid. Output delegates must only have the single proxy object output and than must have pin for event."), *FString(__FUNCTION__));
		return false;
	}

	for (TFieldIterator<FMulticastDelegateProperty> PropertyIt(ProxyClass); PropertyIt && bIsErrorFree; ++PropertyIt)
	{
		UEdGraphPin* LastActivatedThenPin = nullptr;
		bIsErrorFree &= FBaseAsyncTaskHelper::HandleDelegateImplementation(*PropertyIt, VariableOutputs, ProxyObjectPin, InOutLastThenPin, LastActivatedThenPin, this, SourceGraph, CompilerContext);

		bIsErrorFree &= HandleEventImplementation(*PropertyIt, VariableOutputs[0], VariableOutputs[2], VariableOutputs[1], LastActivatedThenPin, SourceGraph, CompilerContext);
	}

	return bIsErrorFree;
}

bool UK2Node_AsyncAction_WaitGameplayEvent::HandleEventImplementation(FMulticastDelegateProperty* CurrentProperty, const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& ProxyObjectVar, const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& PayloadVar, const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& ActualChannelVar, UEdGraphPin*& InOutLastActivatedThenPin, UEdGraph* SourceGraph, FKismetCompilerContext& CompilerContext)
{
	bool bIsErrorFree = true;
	const UEdGraphPin* EventPin = GetEventPin();
	const UEdGraphSchema_K2* Schema = CompilerContext.GetSchema();

	check(CurrentProperty && SourceGraph && Schema);

	const FEdGraphPinType& PinType = EventPin->PinType;

	if (PinType.PinCategory == UEdGraphSchema_K2::PC_Wildcard)
	{
		if (EventPin->LinkedTo.Num() == 0)
		{
			// If no event type is specified and we're not trying to connect the output to anything ignore the rest of this step
			return true;
		}
		else
		{
			return false;
		}
	}

	UK2Node_TemporaryVariable* TempVarOutput = CompilerContext.SpawnInternalVariable(
		this, PinType.PinCategory, PinType.PinSubCategory, PinType.PinSubCategoryObject.Get(), PinType.ContainerType, PinType.PinValueType);

	UK2Node_CallFunction* const CallGetEventDataNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	CallGetEventDataNode->FunctionReference.SetExternalMember(TEXT("GetEventData"), CurrentProperty->GetOwnerClass());
	CallGetEventDataNode->AllocateDefaultPins();

	// Hook up the self connection
	UEdGraphPin* GetPayloadCallSelfPin = Schema->FindSelfPin(*CallGetEventDataNode, EGPD_Input);
	if (GetPayloadCallSelfPin)
	{
		bIsErrorFree &= Schema->TryCreateConnection(GetPayloadCallSelfPin, ProxyObjectVar.TempVar->GetVariablePin());

		// Hook the activate node up in the exec chain
		UEdGraphPin* GetEventExecPin = CallGetEventDataNode->FindPinChecked(UEdGraphSchema_K2::PN_Execute);
		UEdGraphPin* GetEventThenPin = CallGetEventDataNode->FindPinChecked(UEdGraphSchema_K2::PN_Then);
		
		UEdGraphPin* GetEventPin = CallGetEventDataNode->FindPinChecked(TEXT("OutEventData"));
		bIsErrorFree &= Schema->TryCreateConnection(TempVarOutput->GetVariablePin(), GetEventPin);
		
		UK2Node_AssignmentStatement* AssignNode = CompilerContext.SpawnIntermediateNode<UK2Node_AssignmentStatement>(this, SourceGraph);
		AssignNode->AllocateDefaultPins();
		bIsErrorFree &= Schema->TryCreateConnection(GetEventThenPin, AssignNode->GetExecPin());
		bIsErrorFree &= Schema->TryCreateConnection(PayloadVar.TempVar->GetVariablePin(), AssignNode->GetVariablePin());
		AssignNode->NotifyPinConnectionListChanged(AssignNode->GetVariablePin());
		bIsErrorFree &= Schema->TryCreateConnection(AssignNode->GetValuePin(), TempVarOutput->GetVariablePin());
		AssignNode->NotifyPinConnectionListChanged(AssignNode->GetValuePin());


		bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(*InOutLastActivatedThenPin, *AssignNode->GetThenPin()).CanSafeConnect();
		bIsErrorFree &= Schema->TryCreateConnection(InOutLastActivatedThenPin, GetEventExecPin);

		// Hook up the actual channel connection
		UEdGraphPin* OutActualChannelPin = GetActualChannelPin();
		bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(*OutActualChannelPin, *ActualChannelVar.TempVar->GetVariablePin()).CanSafeConnect();
	}

	return bIsErrorFree;
}

void UK2Node_AsyncAction_WaitGameplayEvent::RefreshOutputEventType()
{
	UEdGraphPin* EventPin = GetEventPin();
	UEdGraphPin* EventTypePin = GetEventTypePin();

	if (EventTypePin->DefaultObject != EventPin->PinType.PinSubCategoryObject)
	{
		if (EventPin->SubPins.Num() > 0)
		{
			GetSchema()->RecombinePin(EventPin);
		}

		EventPin->PinType.PinSubCategoryObject = EventTypePin->DefaultObject;
		EventPin->PinType.PinCategory = (EventTypePin->DefaultObject == nullptr) ? UEdGraphSchema_K2::PC_Wildcard : UEdGraphSchema_K2::PC_Struct;
	}
}

UEdGraphPin* UK2Node_AsyncAction_WaitGameplayEvent::GetEventPin() const
{
	UEdGraphPin* Pin = FindPinChecked(AsyncAction_WaitGameplayEvent::EventPinName);
	check(Pin->Direction == EGPD_Output);
	
	return Pin;
}

UEdGraphPin* UK2Node_AsyncAction_WaitGameplayEvent::GetEventTypePin() const
{
	UEdGraphPin* Pin = FindPinChecked(AsyncAction_WaitGameplayEvent::EventTypePinName);
	check(Pin->Direction == EGPD_Input);
	
	return Pin;
}

UEdGraphPin* UK2Node_AsyncAction_WaitGameplayEvent::GetActualChannelPin() const
{
	UEdGraphPin* Pin = FindPinChecked(AsyncAction_WaitGameplayEvent::OutputChannelPinName);
	check(Pin->Direction == EGPD_Output);

	return Pin;
}

#undef LOCTEXT_NAMESPACE
