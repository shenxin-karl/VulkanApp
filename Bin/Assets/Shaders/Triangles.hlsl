
struct VertexIn {
    [[vk::location(0)]] 
    float3 Position : POSITION;

    [[vk::location(1)]]
    float3 Color    : COLOR;
};

struct VertexOut {
    float4 SVPosition : SV_POSITION;

    [[vk::location(0)]]  
    float3 Color      : COLOR;
};

VertexOut VSMain(VertexIn vin) {
    VertexOut vout;
    vout.SVPosition = float4(vin.Position, 1.0);
    vout.Color = vin.Color;
    return vout;
}

[[vk::location(0)]]
float4 PSMain(VertexOut pin) : SV_Target {
    return float4(pin.Color, 1.0);
}