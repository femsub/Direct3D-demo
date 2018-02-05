struct DirectionalLight
{
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float3 direction;
	float pad;
};

struct PointLight
{
	float4 ambient;
	float4 diffuse;
	float4 specular;

	float3 position;
	float range;

	float3 att;
	float pad;
};

struct SpotLight
{
	float4 ambient;
	float4 diffuse;
	float4 specular;

	float3 position;
	float range;

	float3 direction;
	float spot;

	float3 att;
	float pad;
};

struct Material
{
	float4 ambient;
	float4 diffuse;
	float4 specular;//w表示高光强度
	float4 reflect;
};

//计算平行光
void ComputeDirectionalLight(Material mat,        //材质
	DirectionalLight L,    //平行光
	float3 normal,        //顶点法线
	float3 toEye,        //顶点到眼睛的向量
	out float4 ambient,    //计算结果:环境光
	out float4 diffuse,    //计算结果:漫反射光
	out float4 spec)    //计算结果:高光
{
	// 结果初始化为0
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// 光线方向
	float3 lightVec = -L.direction;

	// 环境光直接计算
	ambient = mat.ambient * L.ambient;

	// 计算漫反射系数
	//光线、法线方向归一化

	float diffuseFactor = dot(lightVec, normal);

	// 顶点背向光源不再计算
	[flatten]
	if (diffuseFactor > 0.0f)
	{

		float3 v = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.specular.w);
		//计算漫反射光
		diffuse = diffuseFactor * mat.diffuse * L.diffuse;
		//计算高光
		spec = specFactor * mat.specular * L.specular;
	}
}

//计算点光源
void ComputePointLight(Material mat,        //材质
	PointLight L,        //点光源
	float3 pos,            //顶点位置
	float3 normal,        //顶点法线
	float3 toEye,        //顶点到眼睛的向量
	out float4 ambient, //计算结果:环境光
	out float4 diffuse, //计算结果:漫反射光
	out float4 spec)    //计算结果:高光
{
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	//光照方向：顶点到光源
	float3 lightVec = L.position - pos;

	//顶点到光源距离
	float d = length(lightVec);

	//超过范围不再计算
	if (d > L.range)
		return;

	//归一化光照方向
	lightVec /= d;

	//计算环境光
	ambient = mat.ambient * L.ambient;

	//漫反射系数
	float diffuseFactor = dot(lightVec, normal);

	[flatten]
	if (diffuseFactor > 0.0f)
	{
		float3 v = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.specular.w);
		//计算漫反射光
		diffuse = diffuseFactor * mat.diffuse * L.diffuse;
		//计算高光
		spec = specFactor * mat.specular * L.specular;
	}

	// 计算衰减
	float att = 1.0f / dot(L.att, float3(1.0f, d, d*d));

	diffuse *= att;
	spec *= att;
}
//计算聚光灯
void ComputeSpotLight(Material mat,            //材质
	SpotLight L,        //聚光灯
	float3 pos,            //顶点位置
	float3 normal,        //顶点法线
	float3 toEye,        //顶点到眼睛向量
	out float4 ambient, //计算结果:环境光
	out float4 diffuse, //计算结果:漫反射光
	out float4 spec)    //计算结果:高光
{
	//初始化结果
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	//光照方向：顶点到光源
	float3 lightVec = L.position - pos;

	//顶点到光源距离
	float d = length(lightVec);

	//距离大于光照方向不再计算
	if (d > L.range)
		return;

	//归一化光照方向
	lightVec /= d;

	//计算环境光
	ambient = mat.ambient * L.ambient;
	//计算漫反射系数
	float diffuseFactor = dot(lightVec, normal);

	[flatten]
	if (diffuseFactor > 0.0f)
	{
		float3 v = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.specular.w);
		//漫反射光
		diffuse = diffuseFactor * mat.diffuse * L.diffuse;
		//高光
		spec = specFactor * mat.specular * L.specular;
	}

	//聚光衰减系数
	float spot = pow(max(dot(-lightVec, L.direction), 0.0f), L.spot);
	//衰减系数
	float att = spot / dot(L.att, float3(1.0f, d, d*d));
	ambient *= spot;
	diffuse *= att;
	spec *= att;
}


Texture2D txDiffuse;// : register(t0);
SamplerState samLinear : register(s0);
TextureCube SkyMap;// : register(t1);
cbuffer ConstantBuffer : register(b0)
{
	matrix WorldViewProjection;
	matrix World;
	matrix gWorldInvTranspose;
	float4 vMeshColor;
	Material gMaterial;
}
cbuffer PerFrame : register(b1)
{
	DirectionalLight gDirLight;
	PointLight gPointLight;
	SpotLight gSpotLight;
	float3 gEyePosW;			//观察点
}

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float3 Pos : POSITION;
	float3 Norm : NORMAL;
	float2 Tex : TEXCOORD;
};

struct PS_INPUT
{
	float4 PosH : SV_POSITION;
	float3 PosW : POSITION;
	float3 Norm : NORMAL;
	float2 Tex : TEXCOORD;
};
struct SKYMAP_VS_OUTPUT	//output structure for skymap vertex shader
{
	float4 Pos : SV_POSITION;
	float3 texCoord : TEXCOORD;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	//output.Pos = mul(float4(input.Pos, 1.0f), World);
	//output.Pos = mul(output.Pos, View);
	//output.Pos = mul(output.Pos, Projection);
	output.PosH = mul(float4(input.Pos, 1.0f), WorldViewProjection);
	output.PosW = mul(float4(input.Pos, 1.0f), World).xyz;
	output.Norm = mul(float4(input.Norm, 1.0f), World).xyz;
	output.Tex = input.Tex;

	return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
	//return txDiffuse.Sample(samLinear, input.Tex) * vMeshColor;
	//插值运算有可能使法线不再单位化，重新单位化法线
	input.Norm = normalize(input.Norm);

//顶点到观察点向量，归一化
float3 toEyeW = normalize(gEyePosW - input.PosW);

//初始化颜色值全部为0
float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

//每个光源计算后得到的环境光、漫反射光、高光
float4 A, D, S;

//每个光源计算后将ADS更新到最终结果中
ComputeDirectionalLight(gMaterial, gDirLight, input.Norm, toEyeW, A, D, S);
ambient += A;
diffuse += D;
spec += S;

ComputePointLight(gMaterial, gPointLight, input.PosW, input.Norm, toEyeW, A, D, S);
ambient += A;
diffuse += D;
spec += S;

ComputeSpotLight(gMaterial, gSpotLight, input.PosW, input.Norm, toEyeW, A, D, S);
ambient += A;
diffuse += D;
spec += S;

float4 litColor = ambient + diffuse + spec;

//最终颜色透明度使用漫反射光的
litColor.a = gMaterial.diffuse.a;

return litColor * txDiffuse.Sample(samLinear, input.Tex) * vMeshColor;
}

float4 PS_SOILD(PS_INPUT input) : SV_Target
{
	return vMeshColor;
}


SKYMAP_VS_OUTPUT SKYMAP_VS(float3 inPos : POSITION)
{
	SKYMAP_VS_OUTPUT output = (SKYMAP_VS_OUTPUT)0;

	//Set Pos to xyww instead of xyzw, so that z will always be 1 (furthest from camera)
	//output.Pos = mul(float4(inPos, 1.0f), World);
	//output.Pos = mul(output.Pos, View);
	//output.Pos = mul(output.Pos, Projection).xyww;
	output.Pos = mul(float4(inPos, 1.0f), WorldViewProjection).xyww;
	output.texCoord = inPos;

	return output;
}
float4 SKYMAP_PS(SKYMAP_VS_OUTPUT input) : SV_Target
{
	return SkyMap.Sample(samLinear, input.texCoord);
}

float4 main() : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}