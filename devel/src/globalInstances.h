#pragma once



#ifdef USE_SINGLE_HEADER

#include "../../include/globalInstances.h"

#else

#include "DeferredOperations.h"
#include "NullptrAccessHandler.h"
#include "staticValue.h"
#include "OptionalValue.h"
#include "InstancePointer.h"
#include "instance.h"
#include "InstanceRegistration.h"
#include "throwImpl.h"

#endif // USE_SINGLE_HEADER
