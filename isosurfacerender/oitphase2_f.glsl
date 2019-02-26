
#version 430 core

layout(binding = 1, r32ui) uniform uimage2DRect headPointerImage;

//layout(binding = 2, rgba32ui) uniform uimageBuffer listBuffers;
layout(std430, binding =3) buffer ListBuffer{uvec4 buf[];}listBuffers;
layout(location = 0)out vec4 fragColor;


#define MAX_FRAGMENTS 20
uvec4 fragments[MAX_FRAGMENTS];


int buildLocalFragmentList(){
	int fragCount = 0;

	uint current = imageLoad(headPointerImage,ivec2(gl_FragCoord.xy)).r;


	while(current != 0xFFFFFFFF && fragCount < MAX_FRAGMENTS)
	{
		//uvec4 item = imageLoad(listBuffers,int(current));
		uvec4 item = listBuffers.buf[current];
		current = item.x;
		fragments[fragCount] = item;
		fragCount++;
	}

	return fragCount;

}


int sortFragmentList(int fragCount){
	int i,j;
	int uniqueFragCount =0;
	for(i = 0 ; i <fragCount;i++){
		//bool move = false;
		for(j = i+1;j<fragCount;j++){
			float iDepth = uintBitsToFloat(fragments[i].z);
			float jDepth = uintBitsToFloat(fragments[j].z);
			if(iDepth < jDepth){
				//move = true;
				uvec4 temp = fragments[i];
				fragments[i] = fragments[j];
				fragments[j] = temp;
			}
		}
		//if(!move)
		//	break;
	}

	// i=0;
	// float lastDepth = -1;
	// for(i=0;i<fragCount;i++){
	// 	float d = uintBitsToFloat(fragments[i].z);
	// 	if(abs(lastDepth - d) > 0.1)
	// 	{
	// 		lastDepth = d;
	// 		fragments[uniqueFragCount] = fragments[i];
	// 		uniqueFragCount++;
	// 	}
	// 	else{
	// 		vec4 c1 = unpackUnorm4x8(fragments[uniqueFragCount].y), c2 = unpackUnorm4x8(fragments[i].y);
	// 		fragments[uniqueFragCount].y = packUnorm4x8(vec4(0,0,0,1.0));
	// 	}
	// }
	// return uniqueFragCount;
	return fragCount;
}

vec4 blend_one_minus_src(vec4 currentColor,vec4 newColor)
{
	return mix(currentColor,newColor,newColor.a);
}

vec4 blend_one(vec4 currentColor,vec4 newColor)
{
	return currentColor + newColor;
}

vec4 calculateFinalColor(int fragCount){
	vec4 finalColor = vec4(1.0);

	for(int i = 0 ; i < fragCount;i++)
	{
		vec4 fColor = unpackUnorm4x8(fragments[i].y);
		finalColor = blend_one_minus_src(finalColor,fColor);
	}
	//if(finalColor.x == 0.0 && finalColor.y == 0.0 && finalColor.z == 0.0 && finalColor.z == 0.0)
	// finalColor = vec4(1.0);
	return finalColor;
}

void main(void){
	int fragCount = buildLocalFragmentList();
	int newFragCount = sortFragmentList(fragCount);
	fragColor = calculateFinalColor(newFragCount);
	//fragColor=vec4(1.0*fragCount/MAX_FRAGMENTS);
}