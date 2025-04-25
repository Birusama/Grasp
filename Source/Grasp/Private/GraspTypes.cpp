// Copyright (c) Jared Taylor. All Rights Reserved


#include "GraspTypes.h"

#include "GraspInteractMarker.h"

DEFINE_LOG_CATEGORY(LogGrasp);

#if WITH_EDITOR
FGraspMarker::FGraspMarker(const UGraspInteractMarker* InMarker)
	: RelativeLocation(InMarker->GetComponentLocation())
	, RelativeRotation(InMarker->GetComponentQuat())
	, AttachParent(InMarker->GetAttachParent())
{}
#endif