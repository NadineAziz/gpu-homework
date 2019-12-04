__kernel void update(__global float2 *v, __global float2 *p, __global float* m, float dt)
{
    int gid = get_global_id(0);

    float2 a_gid;
    a_gid.x = 0;
    a_gid.y = 0;

    float mass = m[gid];
    float2 p_gid = p[gid];
    const float G = 0.0001;

    for (int i = 0; i < get_global_size(0); ++i)
    {
        if (i == gid) continue;

        float2 F = p_gid - p[i];
        float r2 = dot(F, F) + 0.001f;

        /*F *= (G * mass * m[i] / r2) / sqrt(r2);
		a_gid += F / mass;*/

        a_gid -= F * ((G * m[i] / r2) / sqrt(r2));
    }

    float2 v_ = v[gid] + dt * a_gid; //"" v = a * t ""

//    barrier(CLK_GLOBAL_MEM_FENCE);

    p[gid] = p_gid + dt * v_; // ""s = v * t""
    v[gid] = v_;
    
}