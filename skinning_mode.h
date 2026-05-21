#pragma once
//#define SKIN_CPU

#ifdef SKIN_CPU
#undef SKIN_GPU
#else
#define SKIN_GPU
#endif // SKIN_CPU