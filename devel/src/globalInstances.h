#pragma once

//#define USE_SINGLE_HEADER


#ifdef USE_SINGLE_HEADER

#include "../../include/globalInstances.h"

#else

#include "ConditionalSingleShotOperations.h"
#include "NullptrAccessHandler.h"
#include "staticValue.h"
#include "OptionalValue.h"
#include "InstancePointer.h"
#include "instance.h"
#include "InstanceRegistration.h"


#endif // GLOBALINSTANCES_H





