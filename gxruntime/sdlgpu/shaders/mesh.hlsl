cbuffer VSParams : register(b0, space1)
{
	float4x4 mvp;
	float4x4 world;
	float4 lightPosDir;
	float4 lightColor;
	float4 ambient;
};

struct VSIn
{
	float3 pos : TEXCOORD0;
	float3 normal : TEXCOORD1;
	float4 color : TEXCOORD2;
	float2 uv : TEXCOORD3;
};

struct VSOut
{
	float4 pos : SV_Position;
	float4 color : COLOR0;
	float2 uv : TEXCOORD0;
};

VSOut VSMain(VSIn i)
{
	VSOut o;
	o.pos = mul(mvp, float4(i.pos, 1.0));

	float4 base = i.color.bgra;
	float4 worldPos = mul(world, float4(i.pos, 1.0));
	float3 nW = normalize(mul(world, float4(i.normal, 0.0)).xyz);
	float3 L = (lightPosDir.w < 0.5)
		? normalize(lightPosDir.xyz)
		: normalize(lightPosDir.xyz - worldPos.xyz);
	float ndl = max(dot(nW, L), 0.0) * lightColor.a;
	float3 lighting = ambient.rgb + lightColor.rgb * ndl;
	o.color = float4(base.rgb * lighting, base.a);
	o.uv = i.uv;
	return o;
}

Texture2D MeshTex : register(t0, space2);
SamplerState MeshSamp : register(s0, space2);

float4 PSMain(VSOut i) : SV_Target0
{
	return MeshTex.Sample(MeshSamp, i.uv) * i.color;
}
