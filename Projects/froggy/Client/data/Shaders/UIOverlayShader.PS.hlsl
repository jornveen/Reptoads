struct VertexShaderOutput
{
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD0;
};


Texture2D UITexture : register(t0);
SamplerState TextureSampler : register(s0);

float4 main( VertexShaderOutput IN ) : SV_Target
{
	return UITexture.Sample(TextureSampler, IN.TexCoord);
}
