---binarizerSum
#version 310 es
layout(local_size_x = 1, local_size_y = 1) in;
layout(rgba8, binding = 0) uniform highp readonly image2D u_Texture;

#define LUMINANCE_BITS  5
#define LUMINANCE_SHIFT  (8 - LUMINANCE_BITS)
#define LUMINANCE_BUCKETS (1 << LUMINANCE_BITS)

struct rowsum
{
    highp int sum[LUMINANCE_BUCKETS];
};

layout(binding = 0) buffer ROWSUM
{
  rowsum s[];
} globalRowSum;

void main()
{
  highp int y = int(gl_GlobalInvocationID.y);

  highp int r = int(imageLoad(u_Texture, ivec2(gl_GlobalInvocationID)).r * 255.0);
  
  highp int x = r >> LUMINANCE_SHIFT;
  atomicAdd(globalRowSum.s[y].sum[x], 1);
}
---binarizerFirstPeak
#version 310 es
#define LUMINANCE_BITS  5
#define LUMINANCE_SHIFT  (8 - LUMINANCE_BITS)
#define LUMINANCE_BUCKETS (1 << LUMINANCE_BITS)
layout(local_size_x = 1, local_size_y = 1) in;

struct rowsum
{
    highp int sum[LUMINANCE_BUCKETS];
};

layout(binding = 0) buffer ROWSUM
{
  rowsum s[];
} globalRowSum;

layout(binding = 1) buffer FIRSTPEEK
{
    volatile highp int p[];
} firstpeak;

void main()
{
  highp int x = int(gl_GlobalInvocationID.x);
  highp int y = int(gl_GlobalInvocationID.y);
  while (true) {
    memoryBarrierBuffer();
    int idx = firstpeak.p[y];
    int score = globalRowSum.s[y].sum[idx];
    if (globalRowSum.s[y].sum[x] > score) {
        atomicCompSwap(firstpeak.p[y], idx, x);
    } else {
        break;
    }
  }
}
---binarizerSecondScore
#version 310 es
layout(local_size_x = 1, local_size_y = 1) in;

#define LUMINANCE_BITS  5
#define LUMINANCE_SHIFT  (8 - LUMINANCE_BITS)
#define LUMINANCE_BUCKETS (1 << LUMINANCE_BITS)
struct rowsum
{
    highp int sum[LUMINANCE_BUCKETS];
};

layout(binding = 0) buffer ROWSUM
{
  rowsum s[];
} globalRowSum;

layout(binding = 1) readonly buffer FIRSTPEEK
{
    highp int p[];
} firstpeak;

layout(binding = 3) buffer SECONDPEEKSCORE
{
    highp int score[];
} secondpeakscore;

void main()
{
  highp int x = int(gl_GlobalInvocationID.x);
  highp int y = int(gl_GlobalInvocationID.y);
  highp int distanceToBiggest = int(x) - int(firstpeak.p[y]);
  highp int score = int(globalRowSum.s[y].sum[x]) * distanceToBiggest * distanceToBiggest;
  secondpeakscore.score[y * LUMINANCE_BUCKETS + x] = score;
}

---binarizerSecondPeak
#version 310 es
#define LUMINANCE_BITS  5
#define LUMINANCE_SHIFT  (8 - LUMINANCE_BITS)
#define LUMINANCE_BUCKETS (1 << LUMINANCE_BITS)
layout(local_size_x = 1, local_size_y = 1) in;

layout(binding = 1) readonly buffer FIRSTPEEK
{
    highp int p[];
} firstpeak;

layout(binding = 2) buffer SECONDPEEK
{
    volatile highp int p[];
} secondpeak;

layout(binding = 3) readonly buffer SECONDPEEKSCORE
{
    highp int score[];
} secondpeakscore;

void main()
{
  highp int x = int(gl_GlobalInvocationID.x);
  highp int y = int(gl_GlobalInvocationID.y);
  highp int myscore = secondpeakscore.score[y * LUMINANCE_BUCKETS + x];
  while (true) {
    memoryBarrierBuffer();
    highp int idx = secondpeak.p[y];
    highp int scorecur = secondpeakscore.score[y * LUMINANCE_BUCKETS + idx];
    if (myscore > scorecur) {
        atomicCompSwap(secondpeak.p[y], idx, x);
    } else {
        break;
    }
  }
}
---bestValleyScore
#version 310 es
#define LUMINANCE_BITS  5
#define LUMINANCE_SHIFT  (8 - LUMINANCE_BITS)
#define LUMINANCE_BUCKETS (1 << LUMINANCE_BITS)
layout(local_size_x = 1, local_size_y = 1) in;

struct rowsum
{
    highp int sum[LUMINANCE_BUCKETS];
};

layout(binding = 0) buffer ROWSUM
{
  rowsum s[];
} globalRowSum;

layout(binding = 1) readonly buffer FIRSTPEEK
{
    highp int p[];
} firstpeak;

layout(binding = 2) readonly buffer SECONDPEEK
{
    highp int p[];
} secondpeak;

layout(binding = 3) buffer BESTVALLEYSCORE
{
    highp int score[];
} bestvalleyscore;

void main()
{
  highp int x = int(gl_GlobalInvocationID.x);
  highp int y = int(gl_GlobalInvocationID.y);
  highp int firstPeak = int(firstpeak.p[y]);
  highp int secondPeak = int(secondpeak.p[y]);
  if (x >= secondPeak)
      return;
  if (x <= firstPeak)
      return;
  highp int fromFirst = x - firstPeak;
  highp int score = fromFirst * fromFirst * (secondPeak - x) * (globalRowSum.s[y].sum[firstPeak] - globalRowSum.s[y].sum[x]);

  bestvalleyscore.score[y * LUMINANCE_BUCKETS + x] = score;
}

---bestValley
#version 310 es
#define LUMINANCE_BITS  5
#define LUMINANCE_SHIFT  (8 - LUMINANCE_BITS)
#define LUMINANCE_BUCKETS (1 << LUMINANCE_BITS)
layout(local_size_x = 1, local_size_y = 1) in;

struct rowsum
{
    highp int sum[LUMINANCE_BUCKETS];
};

layout(binding = 0) buffer ROWSUM
{
  rowsum s[];
} globalRowSum;

layout(binding = 1) readonly buffer FIRSTPEEK
{
    highp int p[];
} firstpeak;

layout(binding = 2) readonly buffer SECONDPEEK
{
    highp int p[];
} secondpeak;

layout(binding = 3) readonly buffer BESTVALLEYSCORE
{
    highp int score[];
} bestvalleyscore;

layout(binding = 4) buffer BESTVALLEY
{
    highp volatile int p[];
} bestvalley;

void main()
{
  highp int x = int(gl_GlobalInvocationID.x);
  highp int y = int(gl_GlobalInvocationID.y);
  highp int firstPeak = int(firstpeak.p[y]);
  highp int secondPeak = int(secondpeak.p[y]);
  if (x >= secondPeak)
      return;
  if (x <= firstPeak)
      return;
  highp int myscore = bestvalleyscore.score[y * LUMINANCE_BUCKETS + x];

  while (true) {
    memoryBarrierBuffer();
    highp int idx = bestvalley.p[y];
    highp int scorecur = bestvalleyscore.score[y * LUMINANCE_BUCKETS + idx];
    if (myscore > scorecur) {
        atomicCompSwap(bestvalley.p[y], idx, x);
    } else {
        break;
    }
  }
}
---binarizerAssign
#version 310 es
layout(local_size_x = 1, local_size_y = 1) in;

layout(binding = 4) readonly buffer BESTVALLEY
{
    highp int p[];
} bestvalley;
layout(rgba8, binding = 0) uniform highp readonly image2D u_TextureIn;
layout(rgba8, binding = 1) uniform highp writeonly image2D u_TextureOut;

void main()
{
    highp float left = imageLoad(u_TextureIn, ivec2(gl_GlobalInvocationID.xy) + ivec2(-1, 0)).r;
    highp float center = imageLoad(u_TextureIn, ivec2(gl_GlobalInvocationID.xy)).r;
    highp float right = imageLoad(u_TextureIn, ivec2(gl_GlobalInvocationID.xy) + ivec2(+1, 0)).r;
    highp float blackpoint = float(bestvalley.p[gl_GlobalInvocationID.y]) / 255.0;
    highp float luminance = ((center * 4.0) - left - right) / 2.0;
    if (luminance >= blackpoint) {
        imageStore(u_TextureOut, ivec2(gl_GlobalInvocationID.xy), vec4(luminance));
    } else {
        imageStore(u_TextureOut, ivec2(gl_GlobalInvocationID.xy), vec4(0.0));
    }
}
