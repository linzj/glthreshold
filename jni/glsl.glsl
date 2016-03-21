---binarizerSum
#version 310 es
layout(local_size_x = 1, local_size_y = 1) in;
layout(rgba8ui, binding = 0) uniform highp readonly uimage2D u_Texture;

#define LUMINANCE_BITS  5
#define LUMINANCE_SHIFT  (8 - LUMINANCE_BITS)
#define LUMINANCE_BUCKETS (1 << LUMINANCE_BITS)

struct rowsum
{
    highp uint sum[LUMINANCE_BUCKETS];
};

layout(binding = 0) buffer ROWSUM
{
  rowsum s[];
} globalRowSum;

void main()
{
  uint y = gl_GlobalInvocationID.y;

  highp uint r = imageLoad(u_Texture, ivec2(gl_GlobalInvocationID)).r;
  
  uint x = r >> LUMINANCE_SHIFT;
  atomicAdd(globalRowSum.s[y].sum[x], 1U);
}
