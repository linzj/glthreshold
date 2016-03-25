---binarizerSum
#version 310 es
layout(local_size_x = 1, local_size_y = 1) in;

#define LUMINANCE_BITS  5
#define LUMINANCE_SHIFT  (8 - LUMINANCE_BITS)
#define LUMINANCE_BUCKETS (1 << LUMINANCE_BITS)

struct rowsum
{
    highp int sum[LUMINANCE_BUCKETS];
};

layout(binding = 0) coherent buffer ROWSUM
{
  rowsum s[];
} globalRowSum;

layout(binding = 6) buffer INPUTCOLOR
{
  highp int color[];
} inputcolor;

void main()
{
  highp int y = int(gl_GlobalInvocationID.y);

  highp int r = inputcolor.color[gl_GlobalInvocationID.y * gl_NumWorkGroups.x + gl_GlobalInvocationID.x];
  
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

layout(binding = 0) coherent buffer ROWSUM
{
  rowsum s[];
} globalRowSum;

layout(binding = 1) coherent buffer FIRSTPEEK
{
    highp int p[];
} firstpeak;

void main()
{
  highp int x = int(gl_GlobalInvocationID.x);
  highp int y = int(gl_GlobalInvocationID.y);
  highp int myscore = globalRowSum.s[y].sum[x];
  while (true) {
    int idx = firstpeak.p[y];
    int score = globalRowSum.s[y].sum[idx];
    if (myscore > score) {
        atomicCompSwap(firstpeak.p[y], idx, x);
    } else if (myscore == score && x < idx) {
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

layout(binding = 0) coherent buffer ROWSUM
{
  rowsum s[];
} globalRowSum;

layout(binding = 1) coherent readonly buffer FIRSTPEEK
{
    highp int p[];
} firstpeak;

layout(binding = 3) coherent buffer SECONDPEEKSCORE
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

layout(binding = 1) coherent readonly buffer FIRSTPEEK
{
    highp int p[];
} firstpeak;

layout(binding = 2) coherent buffer SECONDPEEK
{
    highp int p[];
} secondpeak;

layout(binding = 3) coherent readonly buffer SECONDPEEKSCORE
{
    highp int score[];
} secondpeakscore;

void main()
{
  highp int x = int(gl_GlobalInvocationID.x);
  highp int y = int(gl_GlobalInvocationID.y);
  highp int myscore = secondpeakscore.score[y * LUMINANCE_BUCKETS + x];
  while (true) {
    highp int idx = secondpeak.p[y];
    highp int scorecur = secondpeakscore.score[y * LUMINANCE_BUCKETS + idx];
    if (myscore > scorecur) {
        atomicCompSwap(secondpeak.p[y], idx, x);
    } else if (myscore == scorecur && x < idx) {
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

layout(binding = 0) coherent buffer ROWSUM
{
  rowsum s[];
} globalRowSum;

layout(binding = 1) coherent readonly buffer FIRSTPEEK
{
    highp int p[];
} firstpeak;

layout(binding = 2) coherent readonly buffer SECONDPEEK
{
    highp int p[];
} secondpeak;

layout(binding = 4) coherent buffer BESTVALLEYSCORE
{
    highp int score[];
} bestvalleyscore;

void main()
{
  highp int x = int(gl_GlobalInvocationID.x);
  highp int y = int(gl_GlobalInvocationID.y);
  highp int firstPeak = int(firstpeak.p[y]);
  highp int maxIndex = firstPeak;
  highp int secondPeak = int(secondpeak.p[y]);
  if (firstPeak > secondPeak) {
    highp int temp = firstPeak;
    firstPeak = secondPeak;
    secondPeak = temp;
  }
  if (x >= secondPeak)
      return;
  if (x <= firstPeak)
      return;
  highp int fromFirst = x - firstPeak;
  highp int score = fromFirst * fromFirst * (secondPeak - x) * (globalRowSum.s[y].sum[maxIndex] - globalRowSum.s[y].sum[x]);

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

layout(binding = 0) coherent buffer ROWSUM
{
  rowsum s[];
} globalRowSum;

layout(binding = 1) coherent readonly buffer FIRSTPEEK
{
    highp int p[];
} firstpeak;

layout(binding = 2) coherent readonly buffer SECONDPEEK
{
    highp int p[];
} secondpeak;

layout(binding = 4) coherent readonly buffer BESTVALLEYSCORE
{
    highp int score[];
} bestvalleyscore;

layout(binding = 5) coherent buffer BESTVALLEY
{
    highp int p[];
} bestvalley;

void main()
{
  highp int x = int(gl_GlobalInvocationID.x);
  highp int y = int(gl_GlobalInvocationID.y);
  highp int firstPeak = int(firstpeak.p[y]);
  highp int secondPeak = int(secondpeak.p[y]);
  if (firstPeak > secondPeak) {
    int temp = firstPeak;
    firstPeak = secondPeak;
    secondPeak = temp;
  }
  if (x >= secondPeak)
      return;
  if (x <= firstPeak)
      return;
  highp int myscore = bestvalleyscore.score[y * LUMINANCE_BUCKETS + x];
  if (myscore <= 0)
      return;
  while (true) {
    highp int idx = bestvalley.p[y];
    highp int scorecur = bestvalleyscore.score[y * LUMINANCE_BUCKETS + idx];
    if (myscore > scorecur) {
        atomicCompSwap(bestvalley.p[y], idx, x);
    } else if (myscore == scorecur && x < idx) {
        atomicCompSwap(bestvalley.p[y], idx, x);
    } else {
        break;
    }
  }
}
---binarizerAssign
#version 310 es
#define LUMINANCE_BITS  5
#define LUMINANCE_SHIFT  (8 - LUMINANCE_BITS)
#define LUMINANCE_BUCKETS (1 << LUMINANCE_BITS)
layout(local_size_x = 1, local_size_y = 1) in;

layout(binding = 5) coherent readonly buffer BESTVALLEY
{
    highp int p[];
} bestvalley;
layout(binding = 0) uniform highp sampler2D u_TextureIn;

layout(binding = 6) buffer INPUTCOLOR
{
  highp int color[];
} inputcolor;

layout(binding = 7) buffer OUTPUTCOLOR
{
  highp int color[];
} outputcolor;

void main()
{
    highp uint centerIndex = gl_GlobalInvocationID.y * (2U + gl_NumWorkGroups.x) + gl_GlobalInvocationID.x + 1U;
    highp int left = inputcolor.color[centerIndex - 1U];
    highp int center = inputcolor.color[centerIndex];
    highp int right = inputcolor.color[centerIndex + 1U];
    highp int blackpoint = bestvalley.p[gl_GlobalInvocationID.y] << LUMINANCE_SHIFT;
    highp int luminance = ((center * 4) - left - right) / 2;
    if (luminance >= blackpoint) {
        outputcolor.color[centerIndex] = 255;
    } else {
        outputcolor.color[centerIndex] = 0;
    }
}
