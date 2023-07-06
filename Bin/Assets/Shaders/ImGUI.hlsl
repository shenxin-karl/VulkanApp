
struct VertexIn {
    [[vk::location(0)]]
    float4 Position     : POSITION0;
    [[vk::location(1)]]
    float2 Texcoord     : TEXCOORD0;
    [[vk::location(2)]]
    float4 Color        : COLOR0;
};

struct VertexOut {
    float4 SVPosition   : SV_POSITION;
    [[vk::location(0)]]
    float2 Texcoord     : TEXCOORD0;
    [[vk::location(1)]]
    float4 Color        : COLOR0;
};

[[vk::binding(0, 0)]]
cbuffer VertexBuffer {
    float4x4 matProj;
};

VertexOut VSMain(VertexIn vin) {
    VertexOut vout;
    vout.SVPosition = mul(matProj, vin.Position);
    vout.Texcoord = vin.Texcoord;
    vout.Color = vin.Color;
    return vout;
}

[[vk::binding(1, 0)]]
Texture2D gAlbedoTex;

SamplerState gSampler {
    Filter = MIN_MAG_MIP_POINT;
    AddressU = WRAP;
    AddressV = WRAP;
    AddressW = WRAP;
    MaxAnisotropy = 1.f;
    MaxLOD = -1000;
    MinLOD = 1000;
};

[[vk::location(0)]]
float4 PSMain(VertexOut pin) : SV_Target {
    float4 color = pin.Color * gAlbedoTex.Sample(gSampler, pin.Texcoord);
    const float gamma = 2.2f;
    color.xyz = pow(color.rgb, gamma);
    return color;
}