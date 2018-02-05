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
	float4 specular;//w��ʾ�߹�ǿ��
	float4 reflect;
};

//����ƽ�й�
void ComputeDirectionalLight(Material mat,        //����
	DirectionalLight L,    //ƽ�й�
	float3 normal,        //���㷨��
	float3 toEye,        //���㵽�۾�������
	out float4 ambient,    //������:������
	out float4 diffuse,    //������:�������
	out float4 spec)    //������:�߹�
{
	// �����ʼ��Ϊ0
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// ���߷���
	float3 lightVec = -L.direction;

	// ������ֱ�Ӽ���
	ambient = mat.ambient * L.ambient;

	// ����������ϵ��
	//���ߡ����߷����һ��

	float diffuseFactor = dot(lightVec, normal);

	// ���㱳���Դ���ټ���
	[flatten]
	if (diffuseFactor > 0.0f)
	{

		float3 v = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.specular.w);
		//�����������
		diffuse = diffuseFactor * mat.diffuse * L.diffuse;
		//����߹�
		spec = specFactor * mat.specular * L.specular;
	}
}

//������Դ
void ComputePointLight(Material mat,        //����
	PointLight L,        //���Դ
	float3 pos,            //����λ��
	float3 normal,        //���㷨��
	float3 toEye,        //���㵽�۾�������
	out float4 ambient, //������:������
	out float4 diffuse, //������:�������
	out float4 spec)    //������:�߹�
{
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	//���շ��򣺶��㵽��Դ
	float3 lightVec = L.position - pos;

	//���㵽��Դ����
	float d = length(lightVec);

	//������Χ���ټ���
	if (d > L.range)
		return;

	//��һ�����շ���
	lightVec /= d;

	//���㻷����
	ambient = mat.ambient * L.ambient;

	//������ϵ��
	float diffuseFactor = dot(lightVec, normal);

	[flatten]
	if (diffuseFactor > 0.0f)
	{
		float3 v = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.specular.w);
		//�����������
		diffuse = diffuseFactor * mat.diffuse * L.diffuse;
		//����߹�
		spec = specFactor * mat.specular * L.specular;
	}

	// ����˥��
	float att = 1.0f / dot(L.att, float3(1.0f, d, d*d));

	diffuse *= att;
	spec *= att;
}
//����۹��
void ComputeSpotLight(Material mat,            //����
	SpotLight L,        //�۹��
	float3 pos,            //����λ��
	float3 normal,        //���㷨��
	float3 toEye,        //���㵽�۾�����
	out float4 ambient, //������:������
	out float4 diffuse, //������:�������
	out float4 spec)    //������:�߹�
{
	//��ʼ�����
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	//���շ��򣺶��㵽��Դ
	float3 lightVec = L.position - pos;

	//���㵽��Դ����
	float d = length(lightVec);

	//������ڹ��շ����ټ���
	if (d > L.range)
		return;

	//��һ�����շ���
	lightVec /= d;

	//���㻷����
	ambient = mat.ambient * L.ambient;
	//����������ϵ��
	float diffuseFactor = dot(lightVec, normal);

	[flatten]
	if (diffuseFactor > 0.0f)
	{
		float3 v = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.specular.w);
		//�������
		diffuse = diffuseFactor * mat.diffuse * L.diffuse;
		//�߹�
		spec = specFactor * mat.specular * L.specular;
	}

	//�۹�˥��ϵ��
	float spot = pow(max(dot(-lightVec, L.direction), 0.0f), L.spot);
	//˥��ϵ��
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
	float3 gEyePosW;			//�۲��
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
	//��ֵ�����п���ʹ���߲��ٵ�λ�������µ�λ������
	input.Norm = normalize(input.Norm);

//���㵽�۲����������һ��
float3 toEyeW = normalize(gEyePosW - input.PosW);

//��ʼ����ɫֵȫ��Ϊ0
float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

//ÿ����Դ�����õ��Ļ����⡢������⡢�߹�
float4 A, D, S;

//ÿ����Դ�����ADS���µ����ս����
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

//������ɫ͸����ʹ����������
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