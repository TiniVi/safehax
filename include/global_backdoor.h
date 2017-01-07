#pragma once

#include <3ds/types.h>


typedef u32(*backdoor_fn)(u32 arg0, u32 arg1);
u32 svc_30(void *entry_fn, ...); // can pass up to two arguments to entry_fn(...)

Result svcGlobalBackdoor(s32 (*callback)(void));

bool checkSvcGlobalBackdoor(void);