cbuffer VSParams : register(b0, space1)
{
	float4x4 mvp;
	float4x4 world;
	float4 lightPosDir;
	float4 lightColor;
	float4 ambient;
	float4 matDiffuse;
	float4 matSpec;
	float4 fogColor;
	float4 fogParams;
	float4 eyePos;
	float4 flags;
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
	float fog : TEXCOORD1;
	float4 fogColor : TEXCOORD2;
	float2 testParams : TEXCOORD3;
};

VSOut VSMain(VSIn i)
{
	VSOut o;
	o.pos = mul(mvp, float4(i.pos, 1.0));

	float4 vcol = i.color.bgra;
	float3 baseRgb = (flags.x > 0.5) ? vcol.rgb : matDiffuse.rgb;
	float baseA = vcol.a * matDiffuse.a;

	float4 worldPos = mul(world, float4(i.pos, 1.0));
	float3 nW = normalize(mul(world, float4(i.normal, 0.0)).xyz);
	float3 L = (lightPosDir.w < 0.5)
		? normalize(lightPosDir.xyz)
		: normalize(lightPosDir.xyz - worldPos.xyz);
	float ndl = max(dot(nW, L), 0.0) * lightColor.a;
	float3 lighting = (flags.y > 0.5) ? float3(1.0, 1.0, 1.0) : (ambient.rgb + lightColor.rgb * ndl);

	float3 spec = float3(0.0, 0.0, 0.0);
	if (flags.y < 0.5 && (matSpec.x + matSpec.y + matSpec.z) > 0.0 && matSpec.w > 0.0 && ndl > 0.0) {
		float3 V = normalize(eyePos.xyz - worldPos.xyz);
		float3 H = normalize(L + V);
		spec = matSpec.rgb * pow(max(dot(nW, H), 0.0), max(matSpec.w, 1.0)) * lightColor.a;
	}

	o.color = float4(baseRgb * lighting + spec, baseA);
	o.uv = i.uv;

	float dist = distance(worldPos.xyz, eyePos.xyz);
	float f = 0.0;
	if (fogParams.w > 0.5 && fogParams.w < 1.5) {
		float span = max(fogParams.y - fogParams.x, 1e-6);
		f = 1.0 - saturate((fogParams.y - dist) / span);
	} else if (fogParams.w > 1.5 && fogParams.w < 2.5) {
		f = 1.0 - exp(-fogParams.z * dist);
	} else if (fogParams.w > 2.5) {
		float d = fogParams.z * dist;
		f = 1.0 - exp(-d * d);
	}
	o.fog = saturate(f);
	o.fogColor = fogColor;
	o.testParams = float2(flags.z, flags.w);
	return o;
}

Texture2D MeshTex : register(t0, space2);
SamplerState MeshSamp : register(s0, space2);

float4 PSMain(VSOut i) : SV_Target0
{
	float4 tex = MeshTex.Sample(MeshSamp, i.uv) * i.color;
	if (i.testParams.x > 0.5 && tex.a < i.testParams.y)
		discard;
	tex.rgb = lerp(tex.rgb, i.fogColor.rgb, i.fog);
	return tex;
}
