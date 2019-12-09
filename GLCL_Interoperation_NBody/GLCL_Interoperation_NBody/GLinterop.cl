__kernel void update(__global float3 *v, __global float3 *p, __global float* m, float dt, float G)
{
    int gid = get_global_id(0);

    float3 a_gid;
    a_gid.x = 0;
    a_gid.y = 0;
	a_gid.z = 0;

    float mass = m[gid];
    float3 p_gid = p[gid];
    //const float G = 0.0001;

    for (int i = 0; i < get_global_size(0); ++i)
    {
        if (i == gid) continue;

        float3 F = p_gid - p[i];
        float r2 = dot(F, F) + 0.001f;

        /*F *= (G * mass * m[i] / r2) / sqrt(r2);
		a_gid += F / mass;*/

        a_gid -= F * ((G * m[i] / r2) / sqrt(r2));
    }

    float3 v_ = v[gid] + dt * a_gid; //"" v = a * t ""
	//v_.z = 0.0f;

//    barrier(CLK_GLOBAL_MEM_FENCE);

    p[gid] = p_gid + dt * v_; // ""s = v * t""
    v[gid] = v_;    
}