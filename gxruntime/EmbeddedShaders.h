#pragma once

namespace EmbeddedShaders
{
    static const char* ShadowDepth = R"(
float4x4 World;
float4x4 WorldView;
float4x4 WorldViewProj;

float FarPlane = 1000.0;

struct VS_INPUT {
    float3 Pos : POSITION;
};

struct VS_OUTPUT {
    float4 Pos   : POSITION;
    float  ViewZ : TEXCOORD0;
};

VS_OUTPUT VS(VS_INPUT IN) {
    VS_OUTPUT OUT;
    OUT.Pos = mul(float4(IN.Pos, 1.0), WorldViewProj);
    OUT.ViewZ = mul(float4(IN.Pos, 1.0), WorldView).z;
    return OUT;
}

float4 PS(VS_OUTPUT IN) : COLOR {
    float d = saturate(IN.ViewZ / max(FarPlane, 0.0001));
    return float4(d, d, d, 1.0);
}

technique ShadowDepth {
    pass P0 {
        VertexShader = compile vs_3_0 VS();
        PixelShader  = compile ps_3_0 PS();
    }
}
)";

    static const char* ShadowLit = R"(
float4x4 World;
float4x4 WorldViewProj;

float4x4 LightView;
float4x4 LightProj;
float3   LightColor;
float3   LightDir;
float3   LightPos;
float    LightIsSpot;
float    LightRange = 1000.0;
float    CosPhi;
float    CosTheta;
float    FarPlane = 1000.0;
float    ShadowTexelSize = 1.0 / 1024.0;
float    ShadowBias = 0.0015;
texture  DiffuseTex;
float    HasDiffuseTex;

sampler2D DiffuseSampler = sampler_state {
    Texture = <DiffuseTex>;
    MinFilter = LINEAR; MagFilter = LINEAR; MipFilter = LINEAR;
    AddressU = WRAP; AddressV = WRAP;
};

texture  ShadowMap;
sampler2D ShadowSampler = sampler_state {
    Texture = <ShadowMap>;
    MinFilter = POINT; MagFilter = POINT; MipFilter = NONE;
    AddressU = CLAMP; AddressV = CLAMP;
};

struct VS_INPUT {
    float3 Pos    : POSITION;
    float3 Normal : NORMAL;
    float2 UV     : TEXCOORD0;
};

struct VS_OUTPUT {
    float4 Pos          : POSITION;
    float3 WorldPos     : TEXCOORD0;
    float3 WorldN       : TEXCOORD1;
    float2 UV           : TEXCOORD2;
    float4 LightClipPos : TEXCOORD3;
    float  LightViewZ   : TEXCOORD4;
};

VS_OUTPUT VS(VS_INPUT IN) {
    VS_OUTPUT OUT;
    float4 worldPos = mul(float4(IN.Pos, 1.0), World);
    OUT.Pos = mul(float4(IN.Pos, 1.0), WorldViewProj);
    OUT.WorldPos = worldPos.xyz;
    OUT.WorldN = normalize(mul(float4(IN.Normal, 0.0), World).xyz);
    OUT.UV = IN.UV;

    float4 lightViewPos = mul(worldPos, LightView);
    OUT.LightClipPos = mul(lightViewPos, LightProj);
    OUT.LightViewZ = lightViewPos.z;

    return OUT;
}

float SampleShadow(float2 uv, float refDepth) {
    float inside = step(0.0, uv.x) * step(uv.x, 1.0) * step(0.0, uv.y) * step(uv.y, 1.0);

    float shadow = 0.0;
    float count = 0.0;

    [unroll]
    for (int i = 0; i < 4; i++) {
        [unroll]
        for (int j = 0; j < 4; j++) {
            float2 offset = float2(i - 1.5, j - 1.5) * ShadowTexelSize;
            float depth = tex2D(ShadowSampler, uv + offset).r;
            shadow += (refDepth <= depth + ShadowBias) ? 1.0 : 0.0;
            count += 1.0;
        }
    }

    shadow /= count;
    return lerp(1.0, shadow, inside);
}

float4 PS(VS_OUTPUT IN) : COLOR {
    float3 N = normalize(IN.WorldN);

    float3 toLight = LightPos - IN.WorldPos;
    float dist = max(length(toLight), 0.0001);
    float3 spotL = toLight / dist;
    float3 directionalL = -LightDir;
    float3 L = lerp(directionalL, spotL, LightIsSpot);

    float spotCos = dot(-spotL, LightDir);
    float spotAtten = saturate((spotCos - CosPhi) / max(CosTheta - CosPhi, 0.0001));
    float distAtten = saturate(1.0 - dist / max(LightRange, 0.0001));
    float atten = lerp(1.0, spotAtten * distAtten, LightIsSpot);

    float NdotL = saturate(dot(N, L));

    float2 shadowUV = (IN.LightClipPos.xy / IN.LightClipPos.w) * float2(0.5, -0.5) + 0.5;
    float refDepth = IN.LightViewZ / max(FarPlane, 0.0001);
    float shadow = SampleShadow(shadowUV, refDepth);

    float3 albedo = lerp(float3(1, 1, 1), tex2D(DiffuseSampler, IN.UV).rgb, HasDiffuseTex);
    float3 result = albedo * LightColor * NdotL * atten * shadow;
    return float4(result, 1.0);
}

technique ShadowLit {
    pass P0 {
        VertexShader = compile vs_3_0 VS();
        PixelShader  = compile ps_3_0 PS();
    }
}
)";
}