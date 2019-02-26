#version 420 core                                                                                                                     
layout (vertices = 3) out;
in vec3 frag_normal[];
in vec3 frag_pos[];
out vec3 frag_normal_from_tcs[];
out vec3 frag_pos_from_tcs[];
uniform vec3 view_pos;

uniform float tlsi;
uniform float tlso;
struct OutputPatch
{
    vec3 WorldPos_B030;
    vec3 WorldPos_B021;
    vec3 WorldPos_B012;
    vec3 WorldPos_B003;
    vec3 WorldPos_B102;
    vec3 WorldPos_B201;
    vec3 WorldPos_B300;
    vec3 WorldPos_B210;
    vec3 WorldPos_B120;
    vec3 WorldPos_B111;
    vec3 Normal[3];
    vec2 TexCoord[3];
};
// attributes of the output CPs 
out patch OutputPatch oPatch;
vec3 ProjectToPlane(vec3 Point, vec3 PlanePoint, vec3 PlaneNormal)
{
    vec3 v = Point - PlanePoint;
    float Len = dot(v, PlaneNormal);
    vec3 d = Len * PlaneNormal;
    return (Point - d);
} 
void CalcPositions()
{
    // The original vertices stay the same
    oPatch.WorldPos_B030 = frag_pos[0];
    oPatch.WorldPos_B003 = frag_pos[1];
    oPatch.WorldPos_B300 = frag_pos[2];
    // Edges are names according to the opposing vertex
    vec3 EdgeB300 = oPatch.WorldPos_B003 - oPatch.WorldPos_B030;
    vec3 EdgeB030 = oPatch.WorldPos_B300 - oPatch.WorldPos_B003;
    vec3 EdgeB003 = oPatch.WorldPos_B030 - oPatch.WorldPos_B300;
    // Generate two midpoints on each edge
    oPatch.WorldPos_B021 = oPatch.WorldPos_B030 + EdgeB300 / 3.0;
    oPatch.WorldPos_B012 = oPatch.WorldPos_B030 + EdgeB300 * 2.0 / 3.0;
    oPatch.WorldPos_B102 = oPatch.WorldPos_B003 + EdgeB030 / 3.0;
    oPatch.WorldPos_B201 = oPatch.WorldPos_B003 + EdgeB030 * 2.0 / 3.0;
    oPatch.WorldPos_B210 = oPatch.WorldPos_B300 + EdgeB003 / 3.0;
    oPatch.WorldPos_B120 = oPatch.WorldPos_B300 + EdgeB003 * 2.0 / 3.0;
    // Project each midpoint on the plane defined by the nearest vertex and its normal
    oPatch.WorldPos_B021 = ProjectToPlane(oPatch.WorldPos_B021, oPatch.WorldPos_B030,
                                          oPatch.Normal[0]);
    oPatch.WorldPos_B012 = ProjectToPlane(oPatch.WorldPos_B012, oPatch.WorldPos_B003,
                                         oPatch.Normal[1]);
    oPatch.WorldPos_B102 = ProjectToPlane(oPatch.WorldPos_B102, oPatch.WorldPos_B003,
                                         oPatch.Normal[1]);
    oPatch.WorldPos_B201 = ProjectToPlane(oPatch.WorldPos_B201, oPatch.WorldPos_B300,
                                         oPatch.Normal[2]);
    oPatch.WorldPos_B210 = ProjectToPlane(oPatch.WorldPos_B210, oPatch.WorldPos_B300,
                                         oPatch.Normal[2]);
    oPatch.WorldPos_B120 = ProjectToPlane(oPatch.WorldPos_B120, oPatch.WorldPos_B030,
                                         oPatch.Normal[0]);
    // Handle the center
    vec3 Center = (oPatch.WorldPos_B003 + oPatch.WorldPos_B030 + oPatch.WorldPos_B300) / 3.0;
    oPatch.WorldPos_B111 = (oPatch.WorldPos_B021 + oPatch.WorldPos_B012 + oPatch.WorldPos_B102 +
                          oPatch.WorldPos_B201 + oPatch.WorldPos_B210 + oPatch.WorldPos_B120) / 6.0;
    oPatch.WorldPos_B111 += (oPatch.WorldPos_B111 - Center) / 2.0;
} 
float GetTessLevel(float Distance0, float Distance1)
{
    float AvgDistance = (Distance0 + Distance1) / 2.0;
    if (AvgDistance <= 5) {
        return 10.0;
    }
    else if (AvgDistance <= 30){
        return 7.0;
    }
    else {
        return 3.0;
    }
}

void main(void)                                                                  
{                            

    for (int i = 0 ; i < 3 ; i++) 
    {
       oPatch.Normal[i] = normalize(frag_normal[i]);
       //oPatch.TexCoord[i] = TexCoord_CS_in[i];
    }

    CalcPositions();

    // float EyeToVertexDistance0 = distance(view_pos, frag_pos[0]);
    // float EyeToVertexDistance1 = distance(view_pos, frag_pos[1]);
    // float EyeToVertexDistance2 = distance(view_pos, frag_pos[2]);
                                                
    // gl_TessLevelOuter[0] = GetTessLevel(EyeToVertexDistance1, EyeToVertexDistance2);                                      
    // gl_TessLevelOuter[1] = GetTessLevel(EyeToVertexDistance1, EyeToVertexDistance2);                                           
    // gl_TessLevelOuter[2] = GetTessLevel(EyeToVertexDistance1, EyeToVertexDistance2);
    // gl_TessLevelInner[0] = gl_TessLevelOuter[2]; 

    gl_TessLevelOuter[0] = tlso;                                      
    gl_TessLevelOuter[1] = tlso;                                           
    gl_TessLevelOuter[2] = tlso;
    gl_TessLevelInner[0] = tlsi;

    //gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    //frag_normal_from_tcs[gl_InvocationID] = frag_normal[gl_InvocationID];
    //frag_pos_from_tcs[gl_InvocationID] = frag_pos[gl_InvocationID];

}
