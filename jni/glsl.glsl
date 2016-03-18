---gaussianFragRowSource
uniform sampler2D u_texture;
uniform ivec2 u_screenGeometry;
uniform mediump vec4 u_kernel[92 / 4];
const mediump float c_blockSize = 92.0;

void main(void)
{
    mediump float i;
    highp vec2 texcoord = (gl_FragCoord.xy - vec2(c_blockSize / 2.0, 0)) /
vec2(u_screenGeometry);
    highp float toffset = 1.0 / float(u_screenGeometry.x);
    highp vec3 color = vec3(0.0);
    for (i = 0.0; i < c_blockSize; i += 4.0) {
       color.r += dot(vec4(texture2D(u_texture, texcoord + vec2(i *
toffset, 0.0)).r, texture2D(u_texture, texcoord + vec2((1.0 + i) * toffset,
0.0)).r, texture2D(u_texture, texcoord + vec2((2.0 + i) * toffset, 0.0)).r,
texture2D(u_texture, texcoord + vec2((3.0 + i) * toffset, 0.0)).r),
u_kernel[int(i / 4.0)]);
    }
    gl_FragColor = vec4(color, 1.0);
}
---gaussianFragColumnSource
uniform sampler2D u_texture;
uniform ivec2 u_screenGeometry;
uniform mediump vec4 u_kernel[92 / 4];
const mediump float c_blockSize = 92.0;

void main(void)
{
    mediump float i;
    highp vec2 texcoord = (gl_FragCoord.xy + vec2(0, c_blockSize / 2.0)) /
vec2(u_screenGeometry);
    highp float toffset = 1.0 / float(u_screenGeometry.y);
    highp vec3 color = vec3(0.0);
    for (i = 0.0; i < c_blockSize; i += 4.0) {
       color.r += dot(vec4(texture2D(u_texture, texcoord - vec2(0.0, i *
toffset)).r, texture2D(u_texture, texcoord - vec2(0.0, (1.0 + i) *
toffset)).r, texture2D(u_texture, texcoord - vec2(0.0, (2.0 + i) *
toffset)).r, texture2D(u_texture, texcoord - vec2(0.0, (3.0 + i) *
toffset)).r), u_kernel[int(i / 4.0)]);
    }
    gl_FragColor = vec4(color, 1.0);
}
---adaptiveThresholdFragSource
uniform mediump float u_maxValue;
uniform ivec2 u_screenGeometry;
uniform sampler2D u_textureOrig;
uniform sampler2D u_textureBlur;

void main(void)
{
    highp vec2 texcoord = gl_FragCoord.xy / vec2(u_screenGeometry);
    mediump float colorOrig = texture2D(u_textureOrig, texcoord).r;
    mediump float colorBlur = texture2D(u_textureBlur, texcoord).r;
    mediump vec3 result;
    result = vec3(colorOrig > colorBlur ? u_maxValue : 0.0);
    gl_FragColor = vec4(result, 1.0);
}
---dilateNonZeroRowSource
#version 300 es
uniform highp int u_kRowSize;
uniform sampler2D u_texture;
layout(location = 0) out highp vec4 outputColor;

void main(void)
{
    highp ivec2 texcoord = (ivec2(gl_FragCoord.xy) - ivec2(u_kRowSize / 2, 0));
    highp vec4 m = vec4(0.0);
    int j;

    for (j = 0; j < u_kRowSize; ++j) {
        m = max(m, texelFetch(u_texture, texcoord + ivec2(j, 0), 0));
    }
    outputColor = m;
}
---dilateNonZeroColumnSource
#version 300 es
uniform highp int u_kColumnSize;
uniform sampler2D u_texture;
layout(location = 0) out highp vec4 outputColor;

void main(void)
{
    highp ivec2 texcoord = (ivec2(gl_FragCoord.xy) + ivec2(0, u_kColumnSize / 2));
    highp vec4 m = vec4(0.0);
    int j;

    for (j = 0; j < u_kColumnSize; ++j) {
        m = max(m, texelFetch(u_texture, texcoord - ivec2(0, j), 0));
    }
    outputColor = m;
}
---erodeNonZeroRowSource
#version 300 es
uniform highp int u_kRowSize;
uniform sampler2D u_texture;
layout(location = 0) out highp vec4 outputColor;

void main(void)
{
    highp ivec2 texcoord = (ivec2(gl_FragCoord.xy) - ivec2(u_kRowSize / 2, 0));
    highp vec4 m = vec4(0.9999999);
    int j;

    for (j = 0; j < u_kRowSize; ++j) {
        m = min(m, texelFetch(u_texture, texcoord + ivec2(j, 0), 0));
    }
    outputColor = m;
}
---erodeNonZeroColumnSource
#version 300 es
uniform highp int u_kColumnSize;
uniform sampler2D u_texture;
layout(location = 0) out highp vec4 outputColor;

void main(void)
{
    highp ivec2 texcoord = (ivec2(gl_FragCoord.xy) + ivec2(0, u_kColumnSize / 2 ));
    highp vec4 m = vec4(0.9999999);
    int j;

    for (j = 0; j < u_kColumnSize; ++j) {
        m = min(m, texelFetch(u_texture, texcoord - ivec2(0, j), 0));
    }
    outputColor = m;
}
---thresholdSource
#version 300 es
uniform mediump float u_maxValue;
uniform mediump float u_threshold;
uniform sampler2D u_texture;
layout(location = 0) out highp vec4 outputColor;

void main(void)
{
    highp ivec2 texcoord = ivec2(gl_FragCoord.xy);
    highp float rcolor = texelFetch(u_texture, texcoord, 0).r;
    outputColor = vec4(rcolor > u_threshold ? u_maxValue : 0.0);
}
---vertexShaderSource
#version 300 es
layout(location = 0) in mediump vec4 v_position;

void main()
{
   gl_Position = v_position;
}
