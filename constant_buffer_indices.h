#ifndef CB_INDICES
#define CB_INDICES

#ifdef __cplusplus

#define CAMERA_CB       1
#define FLAT_COMMON_CB  2
#define FULL_VS_CB      0
#define LIGHTING_CB     0
#define SHADOW_CB       1
#define BONE_CB			2
#define KEYFRAME_CB		3

#else

#define CAMERA_CB       b1 /* 3D    VS, GS, PS  */
#define FLAT_COMMON_CB  b2 /* 2D        GS, PS  */
#define FULL_VS_CB      b0 /* 3D    VS, GS,     */
#define LIGHTING_CB     b0 /* 3D            PS  */
#define SHADOW_CB       b1 /* SD    VS          */
#define BONE_CB			b2 /* 3D    VS          */	
#define KEYFRAME_CB		b3 /* 3D    VS          */

#endif

#define BONE_CB_MAX_BONES 256

#endif // CB_INDICES