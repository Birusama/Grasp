// Copyright (c) Jared Taylor


#include "GraspCustomization.h"

#include "DetailLayoutBuilder.h"


TSharedRef<IDetailCustomization> FGraspCustomization::MakeInstance()
{
	return MakeShared<FGraspCustomization>();
}

void FGraspCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.EditCategory(TEXT("Grasp"), FText::GetEmpty(), ECategoryPriority::Important);
}