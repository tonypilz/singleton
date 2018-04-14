#pragma once



#ifdef USE_SINGLE_HEADER

#include "../../include/globalInstances.h"

#else

#include "exceptionsAvailableDetection.h"
#include "staticValue.h"
#include "throwImpl.h"
#include "NullptrAccessHandler.h"
#include "OptionalValue.h"
#include "InstancePointer.h"
#include "instance.h"
#include "InstanceRegistration.h"

#endif // USE_SINGLE_HEADER
