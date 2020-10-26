#pragma once

#if defined(SRV_C_OPT_WRAP_ON)
// Wrapper Func should be implemented with additional platform features.
// This wrapper (Func) at least should call _Func or should not wrap or replace _Func at all
#define SRV_PRIV_C_WRAPPABLE_FUNC(Func) _##Func
#else
#define SRV_PRIV_C_WRAPPABLE_FUNC(Func) Func
#endif
